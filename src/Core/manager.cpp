/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#include "include/manager.h"

#include "../LLP/include/hosts.h"
#include "../LLC/include/random.h"
#include "../LLD/include/index.h"

#include "../Util/include/runtime.h"

namespace Core
{
	
	Manager* pManager;
	
	void Manager::TimestampManager()
	{

	}
	
	void Manager::CreateConnection(int nRandom)
	{
		//printf("##### Connection Manager::attempting connection %s\n", vNew[nRandom].ToStringIP().c_str());
		
		if(!LegacyServer->AddConnection(vNew[nRandom].ToStringIP(), "9323"))
			vTried.push_back(vNew[nRandom]);
					
		vNew.erase(vNew.begin() + nRandom);
	}
	
	
	void Manager::ConnectionManager()
	{
		while(!fStarted)
			Sleep(1000);
		
		if (mapArgs.count("-addnode"))
			for(auto strAddr : mapMultiArgs["-addnode"])
				LegacyServer->AddConnection(strAddr, "9323");
		
		
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}
			

			if(vNew.size() > 0)
			{
				int nRandom = GetRandInt(vNew.size() - 1);
				
				boost::thread thread = boost::thread(&Manager::CreateConnection, this, nRandom);
			}
			

			else if(vTried.size() > 0)
			{
				int nRandom = GetRandInt(vTried.size() - 1);
				
				//printf("##### Connection Manager::attempting tried connection %s\n", vTried[nRandom].ToStringIP().c_str());
				if(!LegacyServer->AddConnection(vTried[nRandom].ToStringIP(), "9323"))
					continue;
				
			}
			
			//TODO: MAke this connection manager more intelligent (Addr Info )
			if(vDropped.size() > 0)
			{
				//int nRandom = GetRandInt(vDropped.size() - 1);
				
				//printf("##### Connection Manager::retry dropped connection %s\n", vDropped[nRandom].ToStringIP().c_str());
				//if(!LegacyServer->AddConnection(vDropped[nRandom].ToStringIP(), "9323"))
				//	continue;
				
				//vDropped.erase(vDropped.begin() + nRandom);
				
			}
			
			
			Sleep(2000);
		}
	}
	
	
	/* Randomly Select a Node. 
	 * TODO: Add selection filtering parameters
	 */
	static unsigned int nRequestCounter = 0;
	LLP::CLegacyNode* Manager::SelectNode()
	{
		std::vector<LLP::CLegacyNode*> vNodes = LegacyServer->GetConnections();
		if(vNodes.size() == 0)
			return NULL;
		
		/* Iterate the request counter. */
		nRequestCounter++;
		if(nRequestCounter >= vNodes.size())
			nRequestCounter = 0;
		
		/* Make sure the node has sent a version message. */
		if(vNodes[nRequestCounter] && vNodes[nRequestCounter]->nCurrentVersion == 0)
			return NULL;
		
		return vNodes[nRequestCounter];
	}
	
	
	/** Sort the block list by its height in ascending order **/
	bool SortByHeight(const uint1024& nFirst, const uint1024& nSecond)
	{
		CBlock blkFirst, blkSecond;
		
		pManager->blkPool.Get(nFirst, blkFirst);
		pManager->blkPool.Get(nSecond, blkSecond);
		
		return blkFirst.nHeight < blkSecond.nHeight;
	}
	
	
	/* Manages the Inventory while building the blockchain
	 * 
	 * Finds inconsitent breaks in blockchain download and makes sure the data is processed. 
	 * 
	 */
	void Manager::InventoryProcessor()
	{
		while(!fStarted)
			Sleep(1000);

		unsigned int nLastBlockRequest = UnifiedTimestamp();
		unsigned int nLastHeightRequst = 0;
		
		nLastHeight   = pindexBest->nHeight + 1;
		nProcessed    = 0;
		fSynchronizing = true;
		
		Timer cHeaders;
		cHeaders.Start();
		
		uint1024 hashLastRequest;
		hashLastBlock = Core::hashBestChain;
		nLastHeight = nBestHeight;
		
		bool fBegin = false;
		while(!fShutdown)
		{
			Sleep(100);
			
			if(pindexBest->GetBlockTime() > UnifiedTimestamp() - 60 * 60)
				fSynchronizing = false;
			
			if(fSynchronizing)
			{
				/* Get some more blocks if the block processor is waiting. */
				std::vector<uint1024> vBlocks;
				if(!blkPool.GetIndexes(blkPool.ACCEPTED, vBlocks))
					continue;
				
				/* Request Inventory to each connected nodes. */
				std::vector<LLP::CInv> vRequest;
				
				/* Sort the Blocks by lowest height first. */
				std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);
				
				/* Iterate the headers and request the block level data. */
				for(auto hash : vBlocks)
				{
					CBlock blk;
					if(!blkPool.Get(hash, blk))
					   continue;
					
					if(blk.nHeight >= nLastHeight + 1000)
					{
						LLP::CLegacyNode* pNode = SelectNode();
						if(!pNode)
							continue;
								
						cHeaders.Reset();
						std::vector<uint1024> vNext = { blk.GetHash() };
						pNode->PushMessage("getblocks", Core::CBlockLocator(vNext), uint1024(0));

						printf("##### Manager : Requesting Sync Headers from block %s\n", blk.GetHash().ToString().substr(0, 20).c_str());
						
						nLastHeight = blk.nHeight;
					}
				}
				

				
				
				/* Get some more blocks if the block processor is waiting. */
				if(!blkPool.GetIndexes(blkPool.REQUESTED, vBlocks))
					continue;
				
				/* Sort the Blocks by lowest height first. */
				std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);
				
				/* Re-request blocks of requests older than 10 seconds. */
				for(auto hash : vBlocks)
				{
					if(blkPool.Age(hash) > 15)
						vRequest.push_back(LLP::CInv(LLP::MSG_BLOCK, hash));
					
					if(vRequest.size() == 500)
					{
						/* Select a Node. */
						LLP::CLegacyNode* pNode = SelectNode();
						if(!pNode)
							break;
				
						pNode->PushMessage("getdata", vRequest);
						printf("##### Manager : Retrying block downloads (%s - %s)\n", vRequest[0].hash.ToString().substr(0, 20).c_str(), vRequest[vRequest.size()-1].hash.ToString().substr(0, 20).c_str());

						vRequest.clear();
					}
				}
			}
		}
	}
	
		
	/* Blocks are checked in the order they are recieved. */
	void Manager::BlockProcessor()
	{
		while(!fStarted)
			Sleep(1000);

		Timer cOrphan;
		cOrphan.Start();
		
		while(!fShutdown)
		{
			Sleep(100);
				
			/* Get some more blocks if the block processor is waiting. */
			std::vector<uint1024> vBlocks;
			if(!blkPool.GetIndexes(blkPool.CHECKED, vBlocks))
				continue;
			
			
			/* Sort the Blocks by Height. */
			std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);

			
			/* Block Processor. */
			//if(GetArg("-verbose", 0) >= 1)
			//	printf("***** Manager::Process Queue with %u Items\n", vBlocks.size());
			
			/* Run through the list of blocks to see if they need to be connected. */
			Timer cTimer;
			for(auto hash : vBlocks)
			{
				cTimer.Reset();
				
				/* Get the Block from the Memory Pool. */
				CBlock block;
				blkPool.Get(hash, block);
				
				
				/* Stop trying to connect if above the best block. */
				if(pindexBest->nHeight +1 != block.nHeight)
				{
					printf("***** Manager::Process Queue Stop at Height %u Best %u. Requesting Headers.\n", block.nHeight, pindexBest->nHeight);
					
					break;
				}

				
				// If don't already have its previous block, shunt it off to holding area until we get it
				if (!mapBlockIndex.count(block.hashPrevBlock))
				{
					if(GetArg("-verbose", 0) >= 1)
						printf("ORPHAN BLOCK, prev=%s\n", block.hashPrevBlock.ToString().substr(0,20).c_str());
					
					mapOrphanBlocksByPrev.insert(std::make_pair(block.hashPrevBlock, block));
					
					LLP::CLegacyNode* pNode = SelectNode();
					if(!pNode)
						continue;
							
					std::vector<LLP::CInv> vInv = { LLP::CInv(LLP::MSG_BLOCK, block.hashPrevBlock) };
					pNode->PushMessage("getdata", vInv);
					
					break;
				}
				
				/*
				{
					LLP::CLegacyNode* pNode = SelectNode();
					if(pNode && (cOrphan.Elapsed() > 5 || hashLastOrphan != block.hashPrevBlock) && block.nHeight < nLastHeight)
						
					{
						LLP::CInv inv(LLP::MSG_BLOCK, block.hashPrevBlock);
						std::vector<LLP::CInv> vInv = { inv };
						pNode->PushMessage("getdata", vInv);
							
						if(GetArg("-verbose", 0) >= 1)
							printf("***** Manager::Requested ORPHAN %s at height %u\n", hash.ToString().substr(0, 20).c_str(), block.nHeight - 1);
						
						cOrphan.Reset();
						hashLastOrphan = block.hashPrevBlock;
					}
					
					break;
				}
				*/
				
				/* Check the block to the blockchain. */
				if(blkPool.Accept(block))
				{
					/* If this isn't the main chain synchronization broadcast the block to other nodes. */
					if(!fSynchronizing)
					{
						std::vector<LLP::CInv> vInv = { LLP::CInv(LLP::MSG_BLOCK, hash) };
						std::vector<LLP::CLegacyNode*> vNodes = LegacyServer->GetConnections();
						for(auto node : vNodes)
							node->PushMessage("inv", vInv);
					}
					
					// Recursively process any orphan blocks that depended on this one
					std::vector<uint1024> vWorkQueue;
					vWorkQueue.push_back(hash);
					for (unsigned int i = 0; i < vWorkQueue.size(); i++)
					{
						uint1024 hashPrev = vWorkQueue[i];
						for (std::multimap<uint1024, CBlock>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev); mi != mapOrphanBlocksByPrev.upper_bound(hashPrev); ++mi)
						{
							CBlock pblockOrphan = (*mi).second;
							if (blkPool.Accept(pblockOrphan))
								vWorkQueue.push_back(pblockOrphan.GetHash());
						}
						
						mapOrphanBlocksByPrev.erase(hashPrev);
					}
					
					/* Keep the Meter data up to date. */
					nConnected ++;
					if(GetArg("-verbose", 0) >= 2)
						printf("ACCEPTED %s in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());

				}
				else
				{
					if(GetArg("-verbose", 0) >= 1)
						printf("REJECTED %s in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());
							
					blkPool.SetState(hash, blkPool.ERROR_ACCEPT);
					
					break;
				}
			}
		}
	}
	
	
	/* LLP Meter Thread. Tracks the Requests / Second. */
	void Manager::ProcessorMeter()
	{
		while(!fStarted)
			Sleep(1000);
		
		Timer TIMER;
		TIMER.Start();
			
		while(!fShutdown)
		{	
			Sleep(30000);
					
			double RPS = (double) nProcessed / TIMER.Elapsed();
			double CPS = (double) nConnected / TIMER.Elapsed();
			
			printf("METER %f processed/s | %f connected/s | best=%s | height=%d | trust=%" PRIu64 "\n", RPS, CPS, hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust);
			
			TIMER.Reset();
			nProcessed = 0;
			nConnected = 0;
		}
	}
		
		
	/* Add address to the Queue. */
	void Manager::AddAddress(LLP::CAddress cAddress)
	{
		vNew.push_back(cAddress);
	}
	
	
	/* Get the addresses of connected nodes. */
	std::vector<LLP::CAddress> Manager::GetAddresses()
	{
		std::vector<LLP::CAddress> vAddr;
		
		std::vector<LLP::CLegacyNode*> vNodes = LegacyServer->GetConnections();
		for(auto node : vNodes)
			vAddr.push_back(node->GetAddress());
			
		return vAddr;
	}
		
		
	/* Get a random address from the active connections in the manager. */
	LLP::CAddress Manager::GetRandAddress(bool fNew)
	{
		std::vector<LLP::CAddress> vSelect;
		if(fNew)
			vSelect.insert(vSelect.begin(), vNew.begin(), vNew.end());
		
		vSelect.insert(vSelect.end(), vTried.begin(), vTried.end());
		int nSelect = GetRandInt(vSelect.size() - 1);
		
		return vSelect[nSelect];
	}
	
	
	/* Start up the Node Manager. */
	void Manager::Start()
	{
		printf("##### Node Started #####\n");
		LegacyServer = new LLP::Server<LLP::CLegacyNode>(LLP::GetDefaultPort(), 10, false, 1, 20, 30, 30, GetBoolArg("-listen", true), true);
		
		fStarted = true;
	}
}
