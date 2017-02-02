#ifndef NEXUS_MESSAGE_SERVER_H
#define NEXUS_MESSAGE_SERVER_H

#include "server.h"
#include "types.h"

namespace LLP
{
	
	class MessageLLP : public NexusConnection
	{	
	public:
		Net::CAddress addr;
		std::string addrName;
		int nVersion;
		int nHeight;
		
		std::string strSubVer;
		bool fClient;
		bool fInbound;
		bool fNetworkNode;
		bool fSuccessfullyConnected;
		bool fDisconnect;
		bool fHasGrant;
		
		MessageLLP() : NexusConnection(){ }
		MessageLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : NexusConnection( SOCKET_IN, DDOS_IN ) 
		{ ADDRESS = parse_ip(SOCKET_IN->remote_endpoint().address().to_string()); }
		
		/** Handle Event Inheritance. **/
		void Event(unsigned char EVENT, unsigned int LENGTH = 0)
		{
			/** Handle any DDOS Packet Filters. **/
			if(EVENT == EVENT_HEADER)
			{
				if(fDDOS)
				{
					Packet PACKET   = this->INCOMING;
					
					if (PACKET.HEADER == "version" && nVersion == 0)
						DDOS->rScore += 5;
					
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
		bool ProcessPacket()
		{
			
			Packet PACKET   = this->INCOMING;
			CDataStream ssMessage(PACKET.DATA, SER_NETWORK, MIN_PROTO_VERSION),
			
			if (PACKET.HEADER == "version")
			{
				int64 nTime;
				Net::CAddress addrMe;
				Net::CAddress addrFrom;
				uint64 nNonce = 1;
				
				/* Check the Protocol Versions */
				ssMessage >> nVersion >> nServices >> nTime >> addrMe;
				if (pfrom->nVersion < MIN_PROTO_VERSION)
				{
					if(GetArg("-verbose", 0) >= 1)
						printf("***** Node %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
					
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

				// Be shy and don't send version until we hear
				//if (pfrom->fInbound)
				
				int64 nTime = GetUnifiedTimestamp();
				CAddress addrYou = (fUseProxy ? CAddress(CService("0.0.0.0",0)) : addr);
				CAddress addrMe = (fUseProxy || !addrLocalHost.IsRoutable() ? CAddress(CService("0.0.0.0",0)) : addrLocalHost);
				RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
				
				CDataStream ssVersion(SER_NETWORK, MIN_PROTO_VERSION);
				ssVersion << PROTOCOL_VERSION << nLocalServices << nTime << addrYou << addrMe << nLocalHostNonce << FormatSubVersion(CLIENT_NAME, DATABASE_VERSION, std::vector<string>()) << Core::nBestHeight;
			
				pfrom->PushPacket("version", ssVersion);
				pfrom->PushPacket("verack");

				if (!pfrom->fInbound)
				{
					// Advertise our address
					if (!fNoListen && !Net::fUseProxy && Net::addrLocalHost.IsRoutable() &&
						!IsInitialBlockDownload())
					{
						Net::CAddress addr(Net::addrLocalHost);
						addr.nTime = GetUnifiedTimestamp();
						pfrom->PushAddress(addr);
					}

					// Get recent addresses
					if (Net::addrman.size() < 1000)
					{
						pfrom->PushMessage("getaddr");
						pfrom->fGetAddr = true;
					}
					Net::addrman.Good(pfrom->addr);
				} else {
					if (((Net::CNetAddr)pfrom->addr) == (Net::CNetAddr)addrFrom)
					{
						Net::addrman.Add(addrFrom, addrFrom);
						Net::addrman.Good(addrFrom);
					}
				}

				// Ask the first 8 connected node for block updates
				static int nAskedForBlocks = 0;
				if (!pfrom->fClient && (nAskedForBlocks < 8))
				{
					nAskedForBlocks++;
					pfrom->PushGetBlocks(pindexBest, uint1024(0));
				}

				pfrom->fSuccessfullyConnected = true;
				
				if(GetArg("-verbose", 0) >= 1)
					printf("version message: version %d, blocks=%d\n", pfrom->nVersion, pfrom->nStartingHeight);

				cPeerBlockCounts.Add(pfrom->nStartingHeight);
			}


			else if (pfrom->nVersion == 0)
			{
				// Must have a version message before anything else
				pfrom->Misbehaving(1);
				return false;
			}


			else if (strCommand == "verack")
			{
				pfrom->vRecv.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));
			}


			else if (strCommand == "addr")
			{
				vector<Net::CAddress> vAddr;
				vRecv >> vAddr;

				// Don't want addr from older versions unless seeding
				if (Net::addrman.size() > 1000)
					return true;
				if (vAddr.size() > 1000)
				{
					pfrom->Misbehaving(20);
					return error("message addr size() = %d", vAddr.size());
				}

				// Store the new addresses
				int64 nNow = GetUnifiedTimestamp();
				int64 nSince = nNow - 10 * 60 * 60;
				BOOST_FOREACH(Net::CAddress& addr, vAddr)
				{
					if (fShutdown)
						return true;
					// ignore IPv6 for now, since it isn't implemented anyway
					if (!addr.IsIPv4())
						continue;
					//if (addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
					//	addr.nTime = nNow;
					pfrom->AddAddressKnown(addr);
					if (addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable())
					{
						// Relay to a limited number of other nodes
						{
							LOCK(Net::cs_vNodes);
							// Use deterministic randomness to send to the same nodes for 24 hours
							// at a time so the setAddrKnowns of the chosen nodes prevent repeats
							static uint512 hashSalt;
							if (hashSalt == 0)
								hashSalt = GetRand512();
							uint64 hashAddr = addr.GetHash();
							uint512 hashRand = hashSalt ^ (hashAddr << 32) ^ ((GetUnifiedTimestamp() + hashAddr)/ (24*60*60));
							hashRand = SK512(BEGIN(hashRand), END(hashRand));
							multimap<uint512, Net::CNode*> mapMix;
							BOOST_FOREACH(Net::CNode* pnode, Net::vNodes)
							{
								unsigned int nPointer;
								memcpy(&nPointer, &pnode, sizeof(nPointer));
								uint512 hashKey = hashRand ^ nPointer;
								hashKey = SK512(BEGIN(hashKey), END(hashKey));
								mapMix.insert(make_pair(hashKey, pnode));
							}
							int nRelayNodes = 2;
							for (multimap<uint512, Net::CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
								((*mi).second)->PushAddress(addr);
						}
					}
				}
				Net::addrman.Add(vAddr, pfrom->addr, 2 * 60 * 60);
				if (vAddr.size() < 1000)
					pfrom->fGetAddr = false;
			}


			else if (strCommand == "inv")
			{
				vector<Net::CInv> vInv;
				vRecv >> vInv;
				if (vInv.size() > 50000)
				{
					pfrom->Misbehaving(20);
					return error("message inv size() = %d", vInv.size());
				}

				// find last block in inv vector
				unsigned int nLastBlock = (unsigned int)(-1);
				for (unsigned int nInv = 0; nInv < vInv.size(); nInv++) {
					if (vInv[vInv.size() - 1 - nInv].type == Net::MSG_BLOCK) {
						nLastBlock = vInv.size() - 1 - nInv;
						break;
					}
				}
				LLD::CIndexDB indexdb("r");
				for (unsigned int nInv = 0; nInv < vInv.size(); nInv++)
				{
					const Net::CInv &inv = vInv[nInv];

					if (fShutdown)
						return true;
					pfrom->AddInventoryKnown(inv);

					bool fAlreadyHave = AlreadyHave(indexdb, inv);
					if(GetArg("-verbose", 0) >= 3)
						printf("  got inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");

					if (!fAlreadyHave)
						pfrom->AskFor(inv);
					else if (inv.type == Net::MSG_BLOCK && mapOrphanBlocks.count(inv.hash)) {
						pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(mapOrphanBlocks[inv.hash]));
					} else if (nInv == nLastBlock) {
						// In case we are on a very long side-chain, it is possible that we already have
						// the last block in an inv bundle sent in response to getblocks. Try to detect
						// this situation and push another getblocks to continue.
						std::vector<Net::CInv> vGetData(1,inv);
						pfrom->PushGetBlocks(mapBlockIndex[inv.hash], uint1024(0));
						if(GetArg("-verbose", 0) >= 3)
							printf("force request: %s\n", inv.ToString().c_str());
					}

					// Track requests for our stuff
					Inventory(inv.hash);
				}
			}


			else if (strCommand == "getdata")
			{
				vector<Net::CInv> vInv;
				vRecv >> vInv;
				if (vInv.size() > 50000)
				{
					pfrom->Misbehaving(20);
					return error("message getdata size() = %d", vInv.size());
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
							
							pfrom->PushMessage("block", block);

							// Trigger them to send a getblocks request for the next batch of inventory
							if (inv.hash == pfrom->hashContinue)
							{
								// Bypass PushInventory, this must send even if redundant,
								// and we want it right after the last block so they don't
								// wait for other stuff first.
								// Nexus: send latest proof-of-work block to allow the
								// download node to accept as orphan (proof-of-stake 
								// block might be rejected by stake connection check)
								vector<Net::CInv> vInv;
								vInv.push_back(Net::CInv(Net::MSG_BLOCK, GetLastBlockIndex(pindexBest, false)->GetBlockHash()));
								pfrom->PushMessage("inv", vInv);
								pfrom->hashContinue = 0;
							}
						}
					}
					else if (inv.IsKnownType())
					{
						// Send stream from relay memory
						{
							LOCK(Net::cs_mapRelay);
							map<Net::CInv, CDataStream>::iterator mi = Net::mapRelay.find(inv);
							if (mi != Net::mapRelay.end())
								pfrom->PushMessage(inv.GetCommand(), (*mi).second);
						}
					}

					// Track requests for our stuff
					Inventory(inv.hash);
				}
			}


			else if (strCommand == "getblocks")
			{
				CBlockLocator locator;
				uint1024 hashStop;
				vRecv >> locator >> hashStop;

				// Find the last block the caller has in the main chain
				CBlockIndex* pindex = locator.GetBlockIndex();

				// Send the rest of the chain
				if (pindex)
					pindex = pindex->pnext;
				int nLimit = 1000 + locator.GetDistanceBack();
				unsigned int nBytes = 0;
				
				if(GetArg("-verbose", 0) >= 3)
					printf("getblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);
				
				for (; pindex; pindex = pindex->pnext)
				{
					if (pindex->GetBlockHash() == hashStop)
					{
						if(GetArg("-verbose", 0) >= 3)
							printf("  getblocks stopping at %d %s (%u bytes)\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str(), nBytes);
						
						// Nexus: tell downloading node about the latest block if it's
						// without risk being rejected due to stake connection check
						if (hashStop != hashBestChain)
							pfrom->PushInventory(Net::CInv(Net::MSG_BLOCK, hashBestChain));
						break;
					}
					pfrom->PushInventory(Net::CInv(Net::MSG_BLOCK, pindex->GetBlockHash()));
					CBlock block;
					block.ReadFromDisk(pindex, true);
					nBytes += block.GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
					if (--nLimit <= 0 || nBytes >= Net::SendBufferSize()/2)
					{
						// When this block is requested, we'll send an inv that'll make them
						// getblocks the next batch of inventory.
						if(GetArg("-verbose", 0) >= 3)
							printf("  getblocks stopping at limit %d %s (%u bytes)\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str(), nBytes);
						
						pfrom->hashContinue = pindex->GetBlockHash();
						break;
					}
				}
			}


			else if (strCommand == "getheaders")
			{
				CBlockLocator locator;
				uint1024 hashStop;
				vRecv >> locator >> hashStop;

				CBlockIndex* pindex = NULL;
				if (locator.IsNull())
				{
					// If locator is null, return the hashStop block
					map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashStop);
					if (mi == mapBlockIndex.end())
						return true;
					pindex = (*mi).second;
				}
				else
				{
					// Find the last block the caller has in the main chain
					pindex = locator.GetBlockIndex();
					if (pindex)
						pindex = pindex->pnext;
				}

				vector<CBlock> vHeaders;
				int nLimit = 2000;
				
				if(GetArg("-verbose", 0) >= 3)
					printf("getheaders %d to %s\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str());
				
				for (; pindex; pindex = pindex->pnext)
				{
					vHeaders.push_back(pindex->GetBlockHeader());
					if (--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
						break;
				}
				pfrom->PushMessage("headers", vHeaders);
			}


			else if (strCommand == "tx")
			{
				vector<uint512> vWorkQueue;
				vector<uint512> vEraseQueue;
				CDataStream vMsg(vRecv);
				LLD::CIndexDB indexdb("r");
				CTransaction tx;
				vRecv >> tx;

				Net::CInv inv(Net::MSG_TX, tx.GetHash());
				pfrom->AddInventoryKnown(inv);

				bool fMissingInputs = false;
				if (tx.AcceptToMemoryPool(indexdb, true, &fMissingInputs))
				{
					SyncWithWallets(tx, NULL, true);
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

			else
			{
				// Ignore unknown commands for extensibility
			}


			// Update the last seen time for this node's address
			if (pfrom->fNetworkNode)
				if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
					AddressCurrentlyConnected(pfrom->addr);


			return true;
		}
}

#endif
