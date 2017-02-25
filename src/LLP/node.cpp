/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "Templates/server.h"
#include "Include/network.h"
#include "Include/hosts.h"

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
	
		
	/* Push a Message With Information about This Current Node. */
	void CNode::PushVersion()
	{
		int64 nTime = GetUnifiedTimestamp();
		CAddress addrYou = (fUseProxy ? CAddress(CService("0.0.0.0",0)) : addr);
		CAddress addrMe = (fUseProxy || !addrLocalHost.IsRoutable() ? CAddress(CService("0.0.0.0",0)) : addrLocalHost);
		RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
		PushMessage("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe,
					nLocalHostNonce, FormatSubVersion(CLIENT_NAME, DATABASE_VERSION, std::vector<string>()), Core::nBestHeight);
	}
	
	
	/** Handle Event Inheritance. **/
	void CNode::Event(unsigned char EVENT, unsigned int LENGTH = 0)
	{
		/** Handle any DDOS Packet Filters. **/
		if(EVENT == EVENT_HEADER)
		{
			if(fDDOS)
			{
				MessagePacket PACKET   = this->INCOMING;
				
				/* Give higher DDOS score if the Node happens to try to send multiple version messages. */
				if (PACKET.HEADER == "version" && nVersion != 0)
					DDOS->rScore += 5;
					
				/* Check if the Node hit any bans. */
				if(DDOS->Banned())
					return;
					
			}
		}
			
		/** Handle for a Packet Data Read. **/
		if(EVENT == EVENT_PACKET)
		{
				
			/* Check a packet's validity once it is finished being read. */
			if(fDDOS) {
				MessagePacket PACKET   = this->INCOMING;
					
				/* Give higher score for Bad Packets. */
				if(INCOMING.IsComplete() && !INCOMING.IsValid())
					DDOS->rScore += 10;
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
			
		MessagePacket PACKET   = this->INCOMING;
		CDataStream ssMessage(PACKET.DATA, SER_NETWORK, MIN_PROTO_VERSION),
			
			
		/* Message Version is the first message received.
		 * It gives you basic stats about the node to know how to
		 * communicate with it.
		 */
		if (PACKET.COMMAND == "version")
		{
			int64 nTime;
			CAddress addrMe;
			CAddress addrFrom;
			uint64 nNonce    = 1;
			uint64 nServices = 0;
				
			/* Check the Protocol Versions */
			ssMessage >> PROTOCOL_VERSION >> nServices >> nTime >> addrMe;
			if (nVersion < MIN_PROTO_VERSION)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node %s using obsolete version %i; disconnecting\n", GetIPAddress().c_str(), nVersion);
					
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
					printf("***** Node version message: version %d, blocks=%d\n", pfrom->nVersion, pfrom->nStartingHeight);

				/* Add to the Majority Peer Block Count. */
				cPeerBlockCounts.Add(nStartingHeight);
			}

			/* If invalid version message drop the connection. */
			else if (PROTOCOL_VERSION == 0)
				return false;
		}
		
		
		/* Get a Time Offset from the Connection. */
		else if(PACKET.COMMAND == "getoffset")
		{
			
			/* De-Serialize the Timestamp Sent. */
			unsigned int nTimestamp;
			ssMessage >> nTimestamp;
			
			int   nOffset    = (int)(GetUnifiedTimestamp() - nTimestamp);
			PushMessage("offset", nOffset);
				
			if(GetArg("-verbose", 0) >= 3)
				printf("***** Node: Sent Offset %i | %u.%u.%u.%u | Unified %"PRId64"\n", nOffset, ADDRESS[0], ADDRESS[1], ADDRESS[2], ADDRESS[3], GetUnifiedTimestamp());
			
			return true;
		}
		
		/* Recieve a Time Offset from this Node. */
		else if(PACKET.HEADER == "offset")
		{
			
			
		}

		
		/* Handle a new Address Message. 
		 * This allows the exchanging of addresses on the network.
		 */
		else if (PACKET.COMMAND == "addr")
		{
			vector<CAddress> vAddr;
			ssMessage >> vAddr;

			/* Don't want addr from older versions unless seeding */
			if (vAddr.size() > 1000)
				return error("***** Node message addr size() = %d... Dropping Connection", vAddr.size());

			/* Store the new addresses. */
			int64 nNow = GetUnifiedTimestamp();
			int64 nSince = nNow - 10 * 60 * 60;
			BOOST_FOREACH(CAddress& addr, vAddr)
			{
				if (Core::fShutdown)
					return false;
				
				/* ignore IPv6 for now, since it isn't implemented anyway */
				if (!addr.IsIPv4())
					continue;
				
				setAddrKnown.insert(addr);
				
				/* TODO: Relay Address to other nodes. Might be better to have addresses requested by nodes. */
				//Net::addrman.Add(vAddr, addr, 2 * 60 * 60);
			}
		}
		
		
		/* Handle new Inventory Messages.
		 * This is used to know what other nodes have in their inventory to compare to our own. 
		 */
		else if (PACKET.COMMAND == "inv")
		{
			vector<CInv> vInv;
			ssMessage >> vInv;
			
			/* Make sure the inventory size is not too large. */
			if (vInv.size() > 50000)
			{
				DDOS.rScore += 20;
				
				return true;
			}

			/* Check through all the new inventory and decide what to do with it. */
			LLD::CIndexDB indexdb("r");
			for (unsigned int nInv = 0; nInv < vInv.size(); nInv++)
			{
				const CInv &inv = vInv[nInv];

				if (Core::fShutdown)
					return false;
				
				{
					LOCK_GUARD(INVENTORY_MUTEX);
					setInventoryKnown.insert(inv);
				}

				//TODO: Move This Function into Proper Place
				bool fAlreadyHave = AlreadyHave(indexdb, inv);
				
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Node recieved inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");

				if (!fAlreadyHave)
					AskFor(inv);
			}
			
		}

		
		/* Get the Data for a Specific Command. */
		else if (PACKET.COMMAND == "getdata")
		{
			vector<CInv> vInv;
			ssMessage >> vInv;
			if (vInv.size() > 50000)
			{
				DDOS.rScore += 20;
				
				return true;
			}

			BOOST_FOREACH(const CInv& inv, vInv)
			{
				if (Core::fShutdown)
					return false;
						
				if(GetArg("-verbose", 0) >= 3)
					printf("received getdata for: %s\n", inv.ToString().c_str());

				if (inv.type == Net::MSG_BLOCK)
				{
					map<uint1024, Core::CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);
					if (mi != Core::mapBlockIndex.end())
					{
						Core::CBlock block;
						if(!block.ReadFromDisk((*mi).second))
							return error("ProcessMessage() : Could not read Block.");
							
						PushMessage("block", block);
					}
				}
			}
		}


		/* Handle a Request to get a list of Blocks from a Node. */
		else if (PACKET.COMMAND == "getblocks")
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
			unsigned int nBytes = 0;
				
			if(GetArg("-verbose", 0) >= 3)
				printf("***** Node Requested getblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);
				
			for (; pindex; pindex = pindex->pnext)
			{
				if (pindex->GetBlockHash() == hashStop)
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("***** Sending getblocks stopping at %d %s (%u bytes)\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str(), nBytes);
						
					PushInventory(Net::CInv(Net::MSG_BLOCK, pindex->GetBlockHash()));
				}
			}
		}


		else if (PACKET.COMMAND == "tx")
		{
			
			/* Deserialize the Transaction. */
			Core::CTransaction tx;
			ssMessage >> tx;
			
			if(GetArg("-verbose", 0) >= 3)
				printf("received transaction %s\n", block.GetHash().ToString().substr(0,20).c_str());
				
			
			if(GetArg("-verbose", 0) >= 1)
				block.print();

			/* Add the Transaction to Known Inventory. */
			CInv inv(MSG_TX, tx.GetHash());
			AddInventoryKnown(inv);
			
			
			/* Add the Block to the Process Queue. */
			{  LOCK_GUARD(NODE_MUTEX);
				queueBlock.insert(block);
			}
		}


		else if (PACKET.COMMAND == "block")
		{
			Core::CBlock block;
			vRecv >> block;

			if(GetArg("-verbose", 0) >= 3)
				printf("received block %s\n", block.GetHash().ToString().substr(0,20).c_str());
				
			if(GetArg("-verbose", 0) >= 1)
				block.print();

			/* Add the Block to Known Inventory. */
			CInv inv(MSG_BLOCK, block.GetHash());
			AddInventoryKnown(inv);

			/* Add the Block to the Process Queue. */
			{  LOCK_GUARD(NODE_MUTEX);
				queueBlock.insert(block);
			}
			
			//if (Core::ProcessBlock(pfrom, &block))
			//	Net::mapAlreadyAskedFor.erase(inv);
			
		}


		/* TODO: Change this Algorithm. */
		else if (strCommand == "getaddr")
		{
			//pfrom->vAddrToSend.clear();
			//vector<Net::CAddress> vAddr = Net::addrman.GetAddr();
			//BOOST_FOREACH(const Net::CAddress &addr, vAddr)
			//	pfrom->PushAddress(addr);
		}

		/* Send a Ping with a nNonce to get Latency Calculations. */
		else if (PACKET.COMMAND == "ping")
		{
			uint64 nonce = 0;
			ssMessage >> nonce;
			
			nLastPinging = GetUnifiedTimestamp();
			cLatencyTimer.Start();
				
			pfrom->PushMessage("pong", nonce);
		}
		
		else if(PACKET.COMMAND == "pong")
		{
			uint64 nonce = 0;
			ssMessage >> nonce;
			
			nNodeLatency = cLatencyTimer.ElapseMilliseconds();
			cLatencyTimer.Reset();
		}

		
		if (fNetworkNode)
			if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
				AddressCurrentlyConnected(addr);


		return true;
	}
	
}
