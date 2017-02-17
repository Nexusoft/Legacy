/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "message.h"

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
	
	
	/* Check the Inventory to See if a given hash is found. 
		TODO: Rebuild this Inventory System. Much Better Ways to Do it. 
		*/
	bool AlreadyHave(LLD::CIndexDB& indexdb, const Net::CInv& inv)
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
			
			/* Push our version back since we just completed getting the version from the other node. */
			PushMessage("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe, nLocalHostNonce, FormatSubVersion(CLIENT_NAME, DATABASE_VERSION, std::vector<string>()), Core::nBestHeight);
			
			PushMessage("verack");
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
			vector<CAddress> vAddr;
			ssMessage >> vAddr;

			/* Don't want addr from older versions unless seeding */
			if (vAddr.size() > 1000)
				return error("***** Node message addr size() = %d... Dropping Connection", vAddr.size());

			/* Store the new addresses. */
			int64 nNow = GetUnifiedTimestamp();
			int64 nSince = nNow - 10 * 60 * 60;
			BOOST_FOREACH(Net::CAddress& addr, vAddr)
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
		else if (PACKET.HEADER == "inv")
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
		else if (PACKET.HEADER == "getdata")
		{
			vector<Net::CInv> vInv;
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
					
					// Send block from disk
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
		else if (PACKET.HEADER == "getblocks")
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


		else if (PACKET.HEADER == "tx")
		{
			vector<uint512> vWorkQueue;
			vector<uint512> vEraseQueue;
				
			CDataStream vMsg(vRecv);
			LLD::CIndexDB indexdb("r");
			
			Core::CTransaction tx;
			ssMessage >> tx;

			/* TODO: Change the Inventory system. */
			CInv inv(MSG_TX, tx.GetHash());
			AddInventoryKnown(inv);

			bool fMissingInputs = false;
			if (tx.AcceptToMemoryPool(indexdb, true, &fMissingInputs))
			{
				Core::SyncWithWallets(tx, NULL, true);
				
				/* TODO: Make the Relay Message work properly with new LLP Connections. */
				RelayMessage(inv, vMsg);
			
				/* TODO CHANGE THESE THREE LINES. */
				mapAlreadyAskedFor.erase(inv);
				vWorkQueue.push_back(inv.hash.getuint512());
				vEraseQueue.push_back(inv.hash.getuint512());

				
				// Recursively process any orphan transactions that depended on this one
				for (unsigned int i = 0; i < vWorkQueue.size(); i++)
				{
					uint512 hashPrev = vWorkQueue[i];
					for (map<uint512, CDataStream*>::iterator mi = mapOrphanTransactionsByPrev[hashPrev].begin(); mi != mapOrphanTransactionsByPrev[hashPrev].end(); ++mi)
					{
						const CDataStream& vMsg = *((*mi).second);
						Core::CTransaction tx;
						CDataStream(vMsg) >> tx;
						
						CInv inv(MSG_TX, tx.GetHash());
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
		}


		else if (strCommand == "block")
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

			/* TODO: Change Process Block Code. */
			if (Core::ProcessBlock(pfrom, &block))
				Net::mapAlreadyAskedFor.erase(inv);
			
		}


		/* TODO: Change this Algorithm. */
		else if (strCommand == "getaddr")
		{
			//pfrom->vAddrToSend.clear();
			//vector<Net::CAddress> vAddr = Net::addrman.GetAddr();
			//BOOST_FOREACH(const Net::CAddress &addr, vAddr)
			//	pfrom->PushAddress(addr);
		}

		/* TODO: Change the way this works. */
		else if (strCommand == "ping")
		{
			uint64 nonce = 0;
			vRecv >> nonce;
				
			pfrom->PushMessage("pong", nonce);
		}

		
		if (fNetworkNode)
			if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
				AddressCurrentlyConnected(addr);


			return true;
		}
}
