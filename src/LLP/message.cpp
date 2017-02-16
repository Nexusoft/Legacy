/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2013])) == Videlicet[2014] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_MESSAGE_SERVER_H
#define NEXUS_MESSAGE_SERVER_H

#include "server.h"
#include "types.h"

namespace LLP
{
	/* Message LLP Core Functions. 
	 * TODO: Integrate the Core LLP with this Message LLP by keeping Core Protocol alive for next mandatory update and removing on version 3.1.
	 * This will allow the time keeping and address seeding to be on one port and protocol.
	 * 
	 * Mining LLP should remain on its own dedicated protocol for efficiency.
	 * 
	 * Basic Protocol Functioning when Complete:
	 * 1. Get Time seeds from Core LLP (Work out how this will be transitioned into the next update.)
	 * 2. Only start once time is unified. This can be done by an initial time seed received from a version message.
	 * 3. Once Version is seen by new connected node, return an ACK to show that the connection is live.
	 * 4. Seed addresses when asked by other nodes.
	 * 5. Send Blocks when asked by other nodes.
	 * 6. Handle new Message "bestblock" to get the hash and height of greatest block.
	 * 7. Relay new inventory when recieved. This will let the network know of new inventory that hasn't been relayed. If it says "inventory already in vector" then don't relay that. Only relay new inventory one time.
	 * 8. Seed blocks and transactions independent of the main thread to accept them.
	 * 9. Add new "tx" message data into the Memory Pool. 
	 * 10. Run all new block data into ProcessBlock Function. Use a mutex to allow only one process block function to run at a time.
	 * 11. Handle Ping / Pong to know the node latency. Record this as a stat in this class declared in "message.h"
	 * 13. Check the new best block once per minute, if it doesn't have it ask for all blocks up to it.
	 * 14. If a getblocks requests more than 1000 blocks send the inventory, but only allow For a certain threshold of blocks to be sent per second.
	 * 15. Add "checkpoints" message to track the checkpoints other nodes have designated.
	 * 16. Add a "trustkey" command to let other nodes know what your trust key is. Require signature to verify ownership. This then allows nodes to see the network reputation of nodes claiming a key.
	 * 17. Keep eye out for another node using the same trust key. First come first serve on trust keys. If there are duplicate trust keys in two nodes drop both of the node's connections.
	 * 18. Add a "doubletrust" command to let the network know there is a node using a trust key on more than one computer. Ban those nodes from the network. 
	 * 19. Add a "txlock" command to agree on the protocol level what transactions are accepted into memory pool. If there is a lock conflict from two of the same inputs but different outputs remove both from mempool. 
	 * 
	 * 
	 * TODO: Remove all the net / netbase code that isn't needed anymore. Clean up the net folder and move rpcserver.h to RPC/server.h, so forth...
	 * TODO: Work on a HTTP Packet to be used in reading and writing the HTTP headers so that the JSON RPC server can use the LLP as well. This will inheret the LLP threading model and server.
	 */
	bool AlreadyHave(LLD::CIndexDB& indexdb, const Net::CInv& inv)
	{
		switch (inv.type)
		{
		case Net::MSG_TX:
			{
			bool txInMap = false;
				{
				LOCK(mempool.cs);
				txInMap = (mempool.exists(inv.hash.getuint512()));
				}
			return txInMap ||
				   mapOrphanTransactions.count(inv.hash.getuint512()) ||
				   indexdb.ContainsTx(inv.hash.getuint512());
			}

		case Net::MSG_BLOCK:
			return mapBlockIndex.count(inv.hash) ||
				   mapOrphanBlocks.count(inv.hash);
		}
		// Don't know what it is, just say we already got one
		return true;
	}
	
	/** Handle Event Inheritance. **/
	void MessageLLP::Event(unsigned char EVENT, unsigned int LENGTH = 0)
	{
		/** Handle any DDOS Packet Filters. **/
		if(EVENT == EVENT_HEADER)
		{
			if(fDDOS)
			{
				Packet PACKET   = this->INCOMING;
				
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
				Packet PACKET   = this->INCOMING;
					
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
	bool MessageLLP::ProcessPacket()
	{
			
		Packet PACKET   = this->INCOMING;
		CDataStream ssMessage(PACKET.DATA, SER_NETWORK, MIN_PROTO_VERSION),
			
			
		/* Message Version is the first message received.
		 * It gives you basic stats about the node to know how to
		 * communicate with it.
		 */
		if (PACKET.HEADER == "version")
		{
			int64 nTime;
			Net::CAddress addrMe;
			Net::CAddress addrFrom;
			uint64 nNonce    = 1;
			uint64 nServices = 0;
				
			/* Check the Protocol Versions */
			ssMessage >> nVersion >> nServices >> nTime >> addrMe;
			if (nVersion < MIN_PROTO_VERSION)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node %s using obsolete version %i; disconnecting\n", GetIPAddress().c_str(), nVersion);
					
				return false;
			}

			/* Disconnect if we connected to ourselves */
			ssMessage >> addrFrom >> nNonce >> >strSubVer >> nHeight;
			if (nNonce == Net::nLocalHostNonce && nNonce > 1)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node connected to self at %s, disconnecting\n", pfrom->addr.ToString().c_str());

				return false;
			}

			/* Check our External IP Address from Peer Selection. */
			if (addrFrom.IsRoutable() && addrMe.IsRoutable())
				Net::addrSeenByPeer = addrMe;
				
			nTime = GetUnifiedTimestamp();
			CAddress addrYou = (fUseProxy ? CAddress(CService("0.0.0.0",0)) : addr);
			CAddress addrMe = (fUseProxy || !addrLocalHost.IsRoutable() ? CAddress(CService("0.0.0.0",0)) : addrLocalHost);
			RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
			
			/* Push our version back since we just completed getting the version from the other node. */
			PushPacket("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe, nLocalHostNonce, FormatSubVersion(CLIENT_NAME, DATABASE_VERSION, std::vector<string>()), Core::nBestHeight);
			
			PushPacket("verack");
			if (fOUTGOING)
			{
				/* Advertise our address */
				if (!fNoListen && !Net::fUseProxy && Net::addrLocalHost.IsRoutable() &&
					!IsInitialBlockDownload())
				{
					Net::CAddress addr(Net::addrLocalHost);
					addr.nTime = GetUnifiedTimestamp();
						
					PushAddress(addr);
				}

				/* Get recent addresses */
				//if (Net::addrman.size() < 1000)
				//	PushPacket("getaddr");
				
				if(GetArg("-verbose", 0) >= 1)
					printf("***** Node version message: version %d, blocks=%d\n", pfrom->nVersion, pfrom->nStartingHeight);

				/* Add to the Majority Peer Block Count. */
				cPeerBlockCounts.Add(nStartingHeight);
			}

			/* If invalid version message drop the connection. */
			else if (nVersion == 0)
				return false;
		}

		/* Handle a new Address Message. 
		 * This allows the exchanging of addresses on the network.
		 */
		else if (PACKET.HEADER == "addr")
		{
			vector<Net::CAddress> vAddr;
			ssMessage >> vAddr;

			/* Don't want addr from older versions unless seeding */
			if (vAddr.size() > 1000)
				return error("***** Node message addr size() = %d... Dropping Connection", vAddr.size());

			/* Store the new addresses. */
			int64 nNow = GetUnifiedTimestamp();
			int64 nSince = nNow - 10 * 60 * 60;
			BOOST_FOREACH(Net::CAddress& addr, vAddr)
			{
				if (fShutdown)
					return true;
				
				/* ignore IPv6 for now, since it isn't implemented anyway */
				if (!addr.IsIPv4())
					continue;
				
				setAddrKnown.insert(addr);
				
				/* TODO: Relay Address to other nodes. Might be better to have addresses requested by nodes. */
				Net::addrman.Add(vAddr, addr, 2 * 60 * 60);
			}
		}
		
		/* Handle new Inventory Messages.
		 * This is used to know what other nodes have in their inventory to compare to our own. 
		 */
		else if (PACKET.HEADER == "inv")
		{
			vector<Net::CInv> vInv;
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
				const Net::CInv &inv = vInv[nInv];

				if (fShutdown)
					return true;
				
				{
					LOCK(cs_inventory);
					setInventoryKnown.insert(inv);
				}

				bool fAlreadyHave = AlreadyHave(indexdb, inv);
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Node recieved inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");

				if (!fAlreadyHave)
					AskFor(inv);
			}
			
		}

		/* Get the Data for a Specific Command. */
		else if (PACKET.HEADER == "getdata")
		{
			vector<Net::CInv> vInv;
			ssMessage >> vInv;
			if (vInv.size() > 50000)
			{
				DDOS.rScore += 20;
				
				return true;
			}

			BOOST_FOREACH(const Net::CInv& inv, vInv)
			{
				if (fShutdown)
					return true;
						
				if(GetArg("-verbose", 0) >= 3)
					printf("received getdata for: %s\n", inv.ToString().c_str());

				if (inv.type == Net::MSG_BLOCK)
				{
					
					// Send block from disk
					map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);
					if (mi != mapBlockIndex.end())
					{
						CBlock block;
						if(!block.ReadFromDisk((*mi).second))
							return error("ProcessMessage() : Could not read Block.");
							
						CDataStream ssBlock(SER_NETWORK, MIN_PROTO_VERSION);
						ssBlock << block;
						PushPacket("block", ssBlock);
					}
				}
			}
		}


		/* Handle a Request to get a list of Blocks from a Node. */
		else if (PACKET.HEADER == "getblocks")
		{
			CBlockLocator locator;
			uint1024 hashStop;
			ssMessage >> locator >> hashStop;

			/* Find the last block the caller has in the main chain */
			CBlockIndex* pindex = locator.GetBlockIndex();

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
						
					/* TODO: CHANGE THIS. */
					pfrom->PushInventory(Net::CInv(Net::MSG_BLOCK, pindex->GetBlockHash()));
				}
			}
		}


		else if (PACKET.HEADER == "tx")
		{
			vector<uint512> vWorkQueue;
			vector<uint512> vEraseQueue;
				
			CDataStream vMsg(vRecv);
			LLD::CIndexDB indexdb("r");
			
			CTransaction tx;
			ssMessage >> tx;

			/* TODO: Change the Inventory system. */
			Net::CInv inv(Net::MSG_TX, tx.GetHash());
			pfrom->AddInventoryKnown(inv);

			bool fMissingInputs = false;
			if (tx.AcceptToMemoryPool(indexdb, true, &fMissingInputs))
			{
				SyncWithWallets(tx, NULL, true);
				
				/* TODO: Make the Relay Message work properly with new LLP Connections. */
				RelayMessage(inv, vMsg);
			
				Net::mapAlreadyAskedFor.erase(inv);
				vWorkQueue.push_back(inv.hash.getuint512());
				vEraseQueue.push_back(inv.hash.getuint512());

					// Recursively process any orphan transactions that depended on this one
					for (unsigned int i = 0; i < vWorkQueue.size(); i++)
					{
						uint512 hashPrev = vWorkQueue[i];
						for (map<uint512, CDataStream*>::iterator mi = mapOrphanTransactionsByPrev[hashPrev].begin();
							mi != mapOrphanTransactionsByPrev[hashPrev].end();
							++mi)
						{
							const CDataStream& vMsg = *((*mi).second);
							CTransaction tx;
							CDataStream(vMsg) >> tx;
							Net::CInv inv(Net::MSG_TX, tx.GetHash());
							bool fMissingInputs2 = false;

							if (tx.AcceptToMemoryPool(indexdb, true, &fMissingInputs2))
							{
								if(GetArg("-verbose", 1) >= 0)
									printf("   accepted orphan tx %s\n", inv.hash.ToString().substr(0,10).c_str());
								
								SyncWithWallets(tx, NULL, true);
								RelayMessage(inv, vMsg);
								Net::mapAlreadyAskedFor.erase(inv);
								vWorkQueue.push_back(inv.hash.getuint512());
								vEraseQueue.push_back(inv.hash.getuint512());
							}
							else if (!fMissingInputs2)
							{
								// invalid orphan
								vEraseQueue.push_back(inv.hash.getuint512());
								
								if(GetArg("-verbose", 0) >= 0)
									printf("   removed invalid orphan tx %s\n", inv.hash.ToString().substr(0,10).c_str());
							}
						}
					}

					BOOST_FOREACH(uint512 hash, vEraseQueue)
						EraseOrphanTx(hash);
				}
				else if (fMissingInputs)
				{
					AddOrphanTx(vMsg);

					// DoS prevention: do not allow mapOrphanTransactions to grow unbounded
					unsigned int nEvicted = LimitOrphanTxSize(MAX_ORPHAN_TRANSACTIONS);
					if (nEvicted > 0)
						printf("mapOrphan overflow, removed %u tx\n", nEvicted);
				}
				if (tx.nDoS) pfrom->Misbehaving(tx.nDoS);
			}


			else if (strCommand == "block")
			{
				CBlock block;
				vRecv >> block;

				if(GetArg("-verbose", 0) >= 3)
					printf("received block %s\n", block.GetHash().ToString().substr(0,20).c_str());
				
				if(GetArg("-verbose", 0) >= 1)
					block.print();

				Net::CInv inv(Net::MSG_BLOCK, block.GetHash());
				pfrom->AddInventoryKnown(inv);

				if (ProcessBlock(pfrom, &block))
					Net::mapAlreadyAskedFor.erase(inv);
				if (block.nDoS) pfrom->Misbehaving(block.nDoS);
			}


			else if (strCommand == "getaddr")
			{
				pfrom->vAddrToSend.clear();
				vector<Net::CAddress> vAddr = Net::addrman.GetAddr();
				BOOST_FOREACH(const Net::CAddress &addr, vAddr)
					pfrom->PushAddress(addr);
			}

			else if (strCommand == "ping")
			{
				uint64 nonce = 0;
				vRecv >> nonce;
				
				/** Echo the message back with the nonce. This allows for two useful features:

				1) A remote node can quickly check if the connection is operational
				2) Remote nodes can measure the latency of the network thread. If this node
				is overloaded it won't respond to pings quickly and the remote node can
				avoid sending us more work, like chain download requests.

				The nonce stops the remote getting confused between different pings: without
				it, if the remote node sends a ping once per second and this node takes 5
				seconds to respond to each, the 5th ping the remote sends would appear to
				return very quickly. **/
				
				pfrom->PushMessage("pong", nonce);
			}


			// Update the last seen time for this node's address
			if (pfrom->fNetworkNode)
				if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
					AddressCurrentlyConnected(pfrom->addr);


			return true;
		}
}

#endif
