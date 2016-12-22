/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../main.h"

#include "../wallet/db.h"
#include "../LLU/ui_interface.h"

#include "../LLD/index.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;


namespace Core
{

	string strMintMessage = _("Info: Minting suspended due to locked wallet."); 
	string strMintWarning;

	string GetWarnings(string strFor)
	{
		int nPriority = 0;
		string strStatusBar;
		string strRPC;
		if (GetBoolArg("-testsafemode"))
			strRPC = "test";

		// Nexus: wallet lock warning for minting
		if (strMintWarning != "")
		{
			nPriority = 0;
			strStatusBar = strMintWarning;
		}

		// Misc warnings like out of disk space and clock is wrong
		if (strMiscWarning != "")
		{
			nPriority = 1000;
			strStatusBar = strMiscWarning;
		}

		if (strFor == "statusbar")
			return strStatusBar;
		else if (strFor == "rpc")
			return strRPC;
			
		assert(!"GetWarnings() : invalid parameter");
		return "error";
	}




	//////////////////////////////////////////////////////////////////////////////
	//
	// Messages
	//


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



	/* Static Reference for now. */
	bool ProcessMessage(Net::CNode* pfrom, string strCommand, CDataStream& vRecv)
	{
		static map<Net::CService, vector<unsigned char> > mapReuseKey;
		RandAddSeedPerfmon();
		if(GetArg("-verbose", 0) >= 3) {
			printf("%s ", DateTimeStrFormat(GetUnifiedTimestamp()).c_str());
			printf("received: %s (%d bytes)\n", strCommand.c_str(), vRecv.size());
		}
		if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
		{
			printf("dropmessagestest DROPPING RECV MESSAGE\n");
			return true;
		}





		if (strCommand == "version")
		{
			// Each connection can only send one version message
			if (pfrom->nVersion != 0)
			{
				pfrom->Misbehaving(1);
				return false;
			}

			int64 nTime;
			Net::CAddress addrMe;
			Net::CAddress addrFrom;
			uint64 nNonce = 1;
			vRecv >> pfrom->nVersion >> pfrom->nServices >> nTime >> addrMe;
			if (pfrom->nVersion < MIN_PROTO_VERSION)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
				
				pfrom->fDisconnect = true;
				return false;
			}

			if (pfrom->nVersion == 10300)
				pfrom->nVersion = 300;
			if (!vRecv.empty())
				vRecv >> addrFrom >> nNonce;
			if (!vRecv.empty())
				vRecv >> pfrom->strSubVer;
			if (!vRecv.empty())
				vRecv >> pfrom->nStartingHeight;

			// Disconnect if we connected to ourself
			if (nNonce == Net::nLocalHostNonce && nNonce > 1)
			{
				if(GetArg("-verbose", 0) >= 1)
					printf("connected to self at %s, disconnecting\n", pfrom->addr.ToString().c_str());
				
				pfrom->fDisconnect = true;
				return true;
			}

			// Nexus: record my external IP reported by peer
			if (addrFrom.IsRoutable() && addrMe.IsRoutable())
				Net::addrSeenByPeer = addrMe;

			// Be shy and don't send version until we hear
			if (pfrom->fInbound)
				pfrom->PushVersion();

			pfrom->fClient = !(pfrom->nServices & Net::NODE_NETWORK);

			// Change version
			pfrom->PushMessage("verack");
			pfrom->vSend.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

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

	bool ProcessMessages(Net::CNode* pfrom)
	{
		CDataStream& vRecv = pfrom->vRecv;
		if (vRecv.empty())
			return true;
		//if (fDebug)
		//    printf("ProcessMessages(%u bytes)\n", vRecv.size());

		//
		// Message format
		//  (4) message start
		//  (12) command
		//  (4) size
		//  (4) checksum
		//  (x) data
		//

		unsigned char pchMessageStart[4];
		Net::GetMessageStart(pchMessageStart);
		static int64 nTimeLastPrintMessageStart = 0;
		if (GetArg("-verbose", 0) >= 3 && nTimeLastPrintMessageStart + 30 < GetUnifiedTimestamp())
		{
			string strMessageStart((const char *)pchMessageStart, sizeof(pchMessageStart));
			vector<unsigned char> vchMessageStart(strMessageStart.begin(), strMessageStart.end());
			printf("ProcessMessages : AdjustedTime=%"PRI64d" MessageStart=%s\n", GetUnifiedTimestamp(), HexStr(vchMessageStart).c_str());
			nTimeLastPrintMessageStart = GetUnifiedTimestamp();
		}

		loop
		{
			// Scan for message start
			CDataStream::iterator pstart = search(vRecv.begin(), vRecv.end(), BEGIN(pchMessageStart), END(pchMessageStart));
			int nHeaderSize = vRecv.GetSerializeSize(Net::CMessageHeader());
			if (vRecv.end() - pstart < nHeaderSize)
			{
				if ((int)vRecv.size() > nHeaderSize)
				{
					printf("\n\nPROCESSMESSAGE MESSAGESTART NOT FOUND\n\n");
					vRecv.erase(vRecv.begin(), vRecv.end() - nHeaderSize);
				}
				break;
			}
			if (pstart - vRecv.begin() > 0)
				printf("\n\nPROCESSMESSAGE SKIPPED %d BYTES\n\n", pstart - vRecv.begin());
			vRecv.erase(vRecv.begin(), pstart);

			// Read header
			vector<char> vHeaderSave(vRecv.begin(), vRecv.begin() + nHeaderSize);
			Net::CMessageHeader hdr;
			vRecv >> hdr;
			if (!hdr.IsValid())
			{
				printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
				continue;
			}
			string strCommand = hdr.GetCommand();

			// Message size
			unsigned int nMessageSize = hdr.nMessageSize;
			if (nMessageSize > MAX_SIZE)
			{
				printf("ProcessMessages(%s, %u bytes) : nMessageSize > MAX_SIZE\n", strCommand.c_str(), nMessageSize);
				continue;
			}
			if (nMessageSize > vRecv.size())
			{
				// Rewind and wait for rest of message
				vRecv.insert(vRecv.begin(), vHeaderSave.begin(), vHeaderSave.end());
				break;
			}

			// Checksum
			uint512 hash = SK512(vRecv.begin(), vRecv.begin() + nMessageSize);
			unsigned int nChecksum = 0;
			memcpy(&nChecksum, &hash, sizeof(nChecksum));
			if (nChecksum != hdr.nChecksum)
			{
				printf("ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
				   strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
				continue;
			}

			// Copy message to its own buffer
			CDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize, vRecv.nType, vRecv.nVersion);
			vRecv.ignore(nMessageSize);

			// Process message
			bool fRet = false;
			try
			{
				{
					LOCK(cs_main);
					fRet = ProcessMessage(pfrom, strCommand, vMsg);
				}
				if (fShutdown)
					return true;
			}
			catch (std::ios_base::failure& e)
			{
				if (strstr(e.what(), "end of data"))
				{
					// Allow exceptions from underlength message on vRecv
					printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand.c_str(), nMessageSize, e.what());
				}
				else if (strstr(e.what(), "size too large"))
				{
					// Allow exceptions from overlong size
					printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand.c_str(), nMessageSize, e.what());
				}
				else
				{
					PrintExceptionContinue(&e, "ProcessMessages()");
				}
			}
			catch (std::exception& e) {
				PrintExceptionContinue(&e, "ProcessMessages()");
			} catch (...) {
				PrintExceptionContinue(NULL, "ProcessMessages()");
			}

			if (!fRet)
				printf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand.c_str(), nMessageSize);
		}

		vRecv.Compact();
		return true;
	}


	bool SendMessages(Net::CNode* pto, bool fSendTrickle)
	{
		TRY_LOCK(cs_main, lockMain);
		if (lockMain) {
			// Don't send anything until we get their version message
			if (pto->nVersion == 0)
				return true;

			// Keep-alive ping. We send a nonce of zero because we don't use it anywhere
			// right now.
			if (pto->nLastSend && GetUnifiedTimestamp() - pto->nLastSend > 30 * 60 && pto->vSend.empty()) {
				uint64 nonce = 0;
				pto->PushMessage("ping", nonce);
			}

			// Resend wallet transactions that haven't gotten in a block yet
			ResendWalletTransactions();

			// Address refresh broadcast
			static int64 nLastRebroadcast;
			if (!IsInitialBlockDownload() && (GetUnifiedTimestamp() - nLastRebroadcast > 24 * 60 * 60))
			{
				{
					LOCK(Net::cs_vNodes);
					BOOST_FOREACH(Net::CNode* pnode, Net::vNodes)
					{
						// Periodically clear setAddrKnown to allow refresh broadcasts
						if (nLastRebroadcast)
							pnode->setAddrKnown.clear();

						// Rebroadcast our address
						if (!fNoListen && !Net::fUseProxy && Net::addrLocalHost.IsRoutable())
						{
							Net::CAddress addr(Net::addrLocalHost);
							addr.nTime = GetUnifiedTimestamp();
							pnode->PushAddress(addr);
						}
					}
				}
				nLastRebroadcast = GetUnifiedTimestamp();
			}

			//
			// Message: addr
			//
			if (fSendTrickle)
			{
				vector<Net::CAddress> vAddr;
				vAddr.reserve(pto->vAddrToSend.size());
				BOOST_FOREACH(const Net::CAddress& addr, pto->vAddrToSend)
				{
					// returns true if wasn't already contained in the set
					if (pto->setAddrKnown.insert(addr).second)
					{
						vAddr.push_back(addr);
						// receiver rejects addr messages larger than 1000
						if (vAddr.size() >= 1000)
						{
							pto->PushMessage("addr", vAddr);
							vAddr.clear();
						}
					}
				}
				pto->vAddrToSend.clear();
				if (!vAddr.empty())
					pto->PushMessage("addr", vAddr);
			}


			//
			// Message: inventory
			//
			vector<Net::CInv> vInv;
			vector<Net::CInv> vInvWait;
			{
				LOCK(pto->cs_inventory);
				vInv.reserve(pto->vInventoryToSend.size());
				vInvWait.reserve(pto->vInventoryToSend.size());
				BOOST_FOREACH(const Net::CInv& inv, pto->vInventoryToSend)
				{
					if (pto->setInventoryKnown.count(inv))
						continue;

					// trickle out tx inv to protect privacy
					if (inv.type == Net::MSG_TX && !fSendTrickle)
					{
						// 1/4 of tx invs blast to all immediately
						static uint512 hashSalt;
						if (hashSalt == 0)
							hashSalt = GetRand512();
						uint512 hashRand = inv.hash.getuint512() ^ hashSalt;
						hashRand = SK512(BEGIN(hashRand), END(hashRand));
						bool fTrickleWait = ((hashRand & 3) != 0);

						// always trickle our own transactions
						if (!fTrickleWait)
						{
							Wallet::CWalletTx wtx;
							if (GetTransaction(inv.hash.getuint512(), wtx))
								if (wtx.fFromMe)
									fTrickleWait = true;
						}

						if (fTrickleWait)
						{
							vInvWait.push_back(inv);
							continue;
						}
					}

					// returns true if wasn't already contained in the set
					if (pto->setInventoryKnown.insert(inv).second)
					{
						vInv.push_back(inv);
						if (vInv.size() >= 1000)
						{
							pto->PushMessage("inv", vInv);
							vInv.clear();
						}
					}
				}
				pto->vInventoryToSend = vInvWait;
			}
			if (!vInv.empty())
				pto->PushMessage("inv", vInv);


			//
			// Message: getdata
			//
			vector<Net::CInv> vGetData;
			int64 nNow = GetUnifiedTimestamp() * 1000000;
			LLD::CIndexDB CIndexDB("r");
			while (!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
			{
				const Net::CInv& inv = (*pto->mapAskFor.begin()).second;
				if (!AlreadyHave(CIndexDB, inv))
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("sending getdata: %s\n", inv.ToString().c_str());
					
					vGetData.push_back(inv);
					if (vGetData.size() >= 1000)
					{
						pto->PushMessage("getdata", vGetData);
						vGetData.clear();
					}
				}
				Net::mapAlreadyAskedFor[inv] = nNow;
				pto->mapAskFor.erase(pto->mapAskFor.begin());
			}
			if (!vGetData.empty())
				pto->PushMessage("getdata", vGetData);

		}
		return true;
	}

}




