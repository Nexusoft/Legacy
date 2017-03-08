/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/message.h"
#include "include/hosts.h"
#include "include/node.h"

#include "../LLC/include/random.h"
#include "../LLD/include/index.h"
#include "../LLU/include/args.h"

namespace LLP
{
	
	
	/* Check Inventory to see if already owned. */
	bool AlreadyHave(LLD::CIndexDB& indexdb, const CInv& inv)
	{
		switch (inv.type)
		{
		case MSG_TX:
			{
			bool txInMap = false;
				{
				LOCK(Core::mempool.cs);
				txInMap = (Core::mempool.exists(inv.hash.getuint512()));
				}
			return txInMap ||
				   Core::mapOrphanTransactions.count(inv.hash.getuint512()) ||
				   indexdb.ContainsTx(inv.hash.getuint512());
			}

		case MSG_BLOCK:
			return Core::mapBlockIndex.count(inv.hash) ||
					 Core::mapOrphanBlocks.count(inv.hash);
		}
		
		/* Pretend we know what it is even if we don't. */
		return true;
	}
	
        /* Keep Track of the Inventory we Already have. */
        void CNode::AddInventoryKnown(const CInv& inv) { }

		
        /* Send Inventory We have. */
        void CNode::PushInventory(const CInv& inv) {}
	
		
	/* Push a Message With Information about This Current Node. */
	void CNode::PushVersion()
	{
		/* Random Session ID */
		RAND_bytes((unsigned char*)&nSessionID, sizeof(nSessionID));
		
		/* Current Unified Timestamp. */
		int64 nTime = Core::UnifiedTimestamp();
		
		/* Dummy Variable NOTE: Remove in Tritium ++ */
		uint64 nLocalServices = 0;
		
		/* Relay Your Address. */
		CAddress addrMe   = (fUseProxy || !addrMyNode.IsRoutable() ? CAddress(CService("0.0.0.0",0)) : addrMyNode);
		
		/* Push the Message to receiving node. */
		PushMessage("version", LLP::PROTOCOL_VERSION, nLocalServices, nTime, GetAddress(), addrMe,
					nSessionID, FormatFullVersion(), Core::nBestHeight);
	}
	
	
	/** Handle Event Inheritance. **/
	void CNode::Event(unsigned char EVENT, unsigned int LENGTH)
	{
		/** Handle any DDOS Packet Filters. **/
		if(EVENT == EVENT_HEADER)
		{
			if(fDDOS)
			{
				
				/* Give higher DDOS score if the Node happens to try to send multiple version messages. */
				if (INCOMING.GetMessage() == "version" && nCurrentVersion != 0)
					DDOS->rSCORE += 25;
				
			}
			
			return;
		}
			
		/** Handle for a Packet Data Read. **/
		if(EVENT == EVENT_PACKET)
		{
				
			/* Check a packet's validity once it is finished being read. */
			if(fDDOS) {

				/* Give higher score for Bad Packets. */
				if(INCOMING.Complete() && !INCOMING.IsValid())
					DDOS->rSCORE += 15;
				
				if((INCOMING.GetMessage() == "getoffset" || INCOMING.GetMessage() == "offset") && INCOMING.LENGTH != 16)
					DDOS->Ban(strprintf("INVALID PACKET SIZE | OFFSET/GETOFFSET | LENGTH %u", INCOMING.LENGTH));

			}
				
			return;
		}
			
		/* On Generic Event, Broadcast new block if flagged. 
		 * 
		 * NOTE: Replacement for Send Messages
		 */
		if(EVENT == EVENT_GENERIC)
		{
		
		}
			
			
		/** On Connect Event, Assign the Proper Daemon Handle. **/
		if(EVENT == EVENT_CONNECT)
			return;
			
		
		/** On Disconnect Event, Reduce the Connection Count for Daemon **/
		if(EVENT == EVENT_DISCONNECT)
			return;
		
	}
		
		
	/** This function is necessary for a template LLP server. It handles your 
		custom messaging system, and how to interpret it from raw packets. **/
	bool CNode::ProcessPacket()
	{
		CDataStream ssMessage(INCOMING.DATA, SER_NETWORK, MIN_PROTO_VERSION);
			
		/* Get a Time Offset from the Connection. */
		if(INCOMING.GetMessage() == "getoffset")
		{
			/* Don't service unified seeds unless time is unified. */
			if(!Core::fTimeUnified)
				return true;
			
			/* De-Serialize the Request ID. */
			unsigned int nRequestID;
			ssMessage >> nRequestID;
			
			/* De-Serialize the Timestamp Sent. */
			uint64 nTimestamp;
			ssMessage >> nTimestamp;
			
			int   nOffset    = (int)(Core::UnifiedTimestamp(true) - nTimestamp);
			PushMessage("offset", nRequestID, Core::UnifiedTimestamp(true), nOffset);
				
			if(GetArg("-verbose", 0) >= 3)
				printf("***** Node: Sent Offset %i | %s | Unified %" PRIu64 "\n", nOffset, addrThisNode.ToString().c_str(), Core::UnifiedTimestamp());
			
			return true;
		}
		
		/* Recieve a Time Offset from this Node. */
		else if(INCOMING.GetMessage() == "offset")
		{
			
			/* De-Serialize the Request ID. */
			unsigned int nRequestID;
			ssMessage >> nRequestID;
			
			/* De-Serialize the Timestamp Sent. */
			uint64 nTimestamp;
			ssMessage >> nTimestamp;
			
			/* Handle the Request ID's. */
			unsigned int nLatencyTime = (Core::UnifiedTimestamp(true) - nTimestamp);
			{ LOCK(NODE_MUTEX);
				
				/* Ignore Messages Recieved that weren't Requested. */
				if(!mapSentRequests.count(nRequestID)) {
					DDOS->rSCORE += 5;
					
					if(GetArg("-verbose", 0) >= 3)
						printf("***** Node (%s): Invalid Request : Message Not Requested [%x][%u ms]\n", addrThisNode.ToString().c_str(), nRequestID, nLatencyTime);
						
					return true;
				}
				
				
				/* Reject Samples that are recieved 30 seconds after last check on this node. */
				if(Core::UnifiedTimestamp(true) - mapSentRequests[nRequestID] > 30000) {
					mapSentRequests.erase(nRequestID);
					
					if(GetArg("-verbose", 0) >= 3)
						printf("***** Node (%s): Invalid Request : Message Stale [%x][%u ms]\n", addrThisNode.ToString().c_str(), nRequestID, nLatencyTime);
						
					DDOS->rSCORE += 15;
					
					return true;
				}
				

				/* De-Serialize the Offset. */
				int nOffset;
				ssMessage >> nOffset;
				
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Node (%s): Received Unified Offset %i [%x][%u ms]\n", addrThisNode.ToString().c_str(), nOffset, nRequestID, nLatencyTime);
				
				/* Adjust the Offset for Latency. */
				nOffset -= nLatencyTime;
				
				/* Add the Samples. */
				setTimeSamples.insert(nOffset);
				
				/* Remove the Request from the Map. */
				mapSentRequests.erase(nRequestID);
			}
			
			return true;
		}
		
		/* Push a transaction into the Node's Recieved Transaction Queue. */
		else if (INCOMING.GetMessage() == "tx")
		{
			
			/* Deserialize the Transaction. */
			Core::CTransaction tx;
			ssMessage >> tx;
			
			if(GetArg("-verbose", 0) >= 3)
				printf("received transaction %s\n", tx.GetHash().ToString().substr(0,20).c_str());
				
			
			if(GetArg("-verbose", 0) >= 4)
				tx.print();

			
			/* Add the Block to the Process Queue. */
			{  LOCK(NODE_MUTEX);
				CInv inv(MSG_TX, tx.GetHash());
				AddInventoryKnown(inv);
			
				queueTransaction.push(tx);
			}
			
			return true;
		}


		/* Push a block into the Node's Recieved Blocks Queue. */
		else if (INCOMING.GetMessage() == "block")
		{
			Core::CBlock block;
			ssMessage >> block;
			
			/* De-Serialize the Request ID. */
			if(nCurrentVersion > 20000)
			{
				unsigned int nRequestID;
				ssMessage >> nRequestID;
			}
			
			if(GetArg("-verbose", 0) >= 3)
				printf("received block %s\n", block.GetHash().ToString().substr(0,20).c_str());
				
			if(GetArg("-verbose", 0) >= 4)
				block.print();

			/* Add the Block to the Process Queue. */
			{  LOCK(NODE_MUTEX);
				CInv inv(MSG_BLOCK, block.GetHash());
				AddInventoryKnown(inv);
			
				queueBlock.push(block);
			}
			
			//if (Core::ProcessBlock(pfrom, &block))
			//	Net::mapAlreadyAskedFor.erase(inv);
			
			return true;
		}
		
		
		/* Send a Ping with a nNonce to get Latency Calculations. */
		else if (INCOMING.GetMessage() == "ping")
		{
			uint64 nonce = 0;
			ssMessage >> nonce;
			
			nLastPing = Core::UnifiedTimestamp();
			cLatencyTimer.Start();
				
			PushMessage("pong", nonce);
			
			return true;
		}
		
		
		else if(INCOMING.GetMessage() == "pong")
		{
			uint64 nonce = 0;
			ssMessage >> nonce;
			
			nNodeLatency = cLatencyTimer.ElapsedMilliseconds();
			cLatencyTimer.Reset();
			
			return true;
		}
		
			
		/* ______________________________________________________________
		 * 
		 * 
		 * NOTE: These following methods will be deprecated post Tritium. 
		 *
		 * ______________________________________________________________
		 */
			
			
		/* Message Version is the first message received.
		 * It gives you basic stats about the node to know how to
		 * communicate with it.
		 */
		else if (INCOMING.GetMessage() == "version")
		{
			int64 nTime;
			CAddress addrMe;
			CAddress addrFrom;
			uint64 nServices = 0;
			
				
			/* Check the Protocol Versions */
			ssMessage >> nCurrentVersion >> nServices >> nTime >> addrMe >> addrFrom >> nSessionID >> strNodeVersion >> nStartingHeight;
			if (nCurrentVersion < MIN_PROTO_VERSION)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node %s using obsolete version %i; disconnecting\n", GetIPAddress().c_str(), PROTOCOL_VERSION);
					
				return false;
			}
			
			/* Push our version back since we just completed getting the version from the other node. */
			PushVersion();
			
			PushMessage("verack");
			if (fOUTGOING)
			{
				/* Get recent addresses */
				//if (Net::addrman.size() < 1000)
				//	PushPacket("getaddr");
				
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node version message: version %d, blocks=%d\n", nCurrentVersion, nStartingHeight);

				/* Add to the Majority Peer Block Count. */
				Core::cPeerBlockCounts.Add(nStartingHeight);
			}

			/* If invalid version message drop the connection. */
			else if (nCurrentVersion == 0)
				return false;
		}

		
		/* Handle a new Address Message. 
		 * This allows the exchanging of addresses on the network.
		 */
		else if (INCOMING.GetMessage() == "addr")
		{
			std::vector<CAddress> vAddr;
			ssMessage >> vAddr;

			/* Don't want addr from older versions unless seeding */
			if (vAddr.size() > 1000){
				DDOS->rSCORE += 20;
				
				return error("***** Node message addr size() = %d... Dropping Connection", vAddr.size());
			}

			/* Store the new addresses. */
			BOOST_FOREACH(CAddress& addr, vAddr)
			{
				/* ignore IPv6 for now, since it isn't implemented anyway */
				if (!addr.IsIPv4())
					continue;
				
				//setAddrKnown.insert(addr);
				
				/* TODO: Relay Address to other nodes. Might be better to have addresses requested by nodes. */
				//Net::addrman.Add(vAddr, addr, 2 * 60 * 60);
			}
		}
		
		
		/* Handle new Inventory Messages.
		 * This is used to know what other nodes have in their inventory to compare to our own. 
		 */
		else if (INCOMING.GetMessage() == "inv")
		{
			std::vector<CInv> vInv;
			ssMessage >> vInv;
			
			/* Make sure the inventory size is not too large. */
			if (vInv.size() > 50000)
			{
				DDOS->rSCORE += 20;
				
				return true;
			}

			/* Check through all the new inventory and decide what to do with it. */
			LLD::CIndexDB indexdb("r");
			for (int i = 0; i < vInv.size(); i++)
			{
				{
					LOCK(NODE_MUTEX);
					setInventoryKnown.insert(vInv[i]);
				}

				//TODO: Move This Function into Proper Place
				bool fAlreadyHave = AlreadyHave(indexdb, vInv[i]);
				
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Node recieved inventory: %s  %s\n", vInv[i].ToString().c_str(), fAlreadyHave ? "have" : "new");

				if (!fAlreadyHave)
					PushMessage("getdata", vInv[i]);
			}
			
		}

		
		/* Get the Data for a Specific Command. */
		else if (INCOMING.GetMessage() == "getdata")
		{
			std::vector<CInv> vInv;
			ssMessage >> vInv;
			if (vInv.size() > 50000)
			{
				DDOS->rSCORE += 20;
				
				return true;
			}

			for(int i = 0; i < vInv.size(); i++)
			{
				if(GetArg("-verbose", 0) >= 3)
					printf("received getdata for: %s\n", vInv[i].ToString().c_str());

				if (vInv[i].type == MSG_BLOCK)
				{
					std::map<uint1024, Core::CBlockIndex*>::iterator mi = Core::mapBlockIndex.find(vInv[i].hash);
					if (mi != Core::mapBlockIndex.end())
					{
						Core::CBlock block;
						if(!block.ReadFromDisk((*mi).second))
							return error("ProcessMessage() : Could not read Block.");
							
						PushMessage("block", block);
					}
				}
				
				//else if(vInv[i].type == MSG_TX)
			}
		}


		/* Handle a Request to get a list of Blocks from a Node. */
		else if (INCOMING.GetMessage() == "getblocks")
		{
			Core::CBlockLocator locator;
			uint1024 hashStop;
			ssMessage >> locator >> hashStop;

			/* Find the last block the caller has in the main chain */
			Core::CBlockIndex* pindex = locator.GetBlockIndex();

			/* Send the rest of the chain as INV messages. */
			if (pindex)
				pindex = pindex->pnext;
			
			int nLimit = 1000 + locator.GetDistanceBack();
			if(GetArg("-verbose", 0) >= 3)
				printf("***** Node Requested getblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);
				
			for (; pindex; pindex = pindex->pnext)
			{
				if (pindex->GetBlockHash() == hashStop)
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("***** Sending getblocks stopping at %d %s \n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
						
					if(hashStop != Core::hashBestChain)
						PushInventory(CInv(MSG_BLOCK, Core::hashBestChain));
				}
				
				PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));
			}
		}


		/* TODO: Change this Algorithm. */
		else if (INCOMING.GetMessage() == "getaddr")
		{
			//pfrom->vAddrToSend.clear();
			//vector<Net::CAddress> vAddr = Net::addrman.GetAddr();
			//BOOST_FOREACH(const Net::CAddress &addr, vAddr)
			//	pfrom->PushAddress(addr);
		}


		return true;
	}
	
}
