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
	
	
	void Manager::ConnectionManager()
	{
		while(!fStarted)
			Sleep(1000);
		
		if (mapArgs.count("-addnode"))
			for(auto strAddr : mapMultiArgs["-addnode"])
				AddConnection(strAddr, "9323");
		
		
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}
			

			if(vNew.size() > 0)
			{
				int nRandom = GetRandInt(vNew.size() - 1);
				
				printf("##### Connection Manager::attempting connection %s\n", vNew[nRandom].ToStringIP().c_str());
				if(!AddConnection(vNew[nRandom].ToStringIP(), "9323"))
					vTried.push_back(vNew[nRandom]);
					
				vNew.erase(vNew.begin() + nRandom);
			}
			

			else if(vTried.size() > 0)
			{
				int nRandom = GetRandInt(vTried.size() - 1);
				
				printf("##### Connection Manager::attempting tried connection %s\n", vTried[nRandom].ToStringIP().c_str());
				if(!AddConnection(vTried[nRandom].ToStringIP(), "9323"))
					continue;
				
			}
			
			//TODO: MAke this connection manager more intelligent (Addr Info )
			if(vDropped.size() > 0)
			{
				int nRandom = GetRandInt(vDropped.size() - 1);
				
				printf("##### Connection Manager::retry dropped connection %s\n", vDropped[nRandom].ToStringIP().c_str());
				if(!AddConnection(vDropped[nRandom].ToStringIP(), "9323"))
					continue;
				
				vDropped.erase(vDropped.begin() + nRandom);
				
			}
			
			
			Sleep(1000);
		}
	}
	
	
	/* Randomly Select a Node. 
	 * TODO: Add selection filtering parameters
	 */
	static unsigned int nRequestCounter = 0;
	LLP::CLegacyNode* Manager::SelectNode()
	{
		std::vector<LLP::CLegacyNode*> vNodes = GetConnections();
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
		nLastHeight   = pindexBest->nHeight;
		
		fSynchronizing = true;
		while(!fShutdown)
		{
			Sleep(100);
			
			if(pindexBest->GetBlockTime() > UnifiedTimestamp() - 60 * 60)
				fSynchronizing = false;
			
			
			/* Check for blocks that have been asked for, and re-try if failed. */
			std::vector<uint1024> vBlocks;
			if(blkPool.GetIndexes(blkPool.CHECKED, vBlocks))
			{
				std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);
				
				for(auto hash : vBlocks)
				{
					CBlock blk;
					blkPool.Get(hash, blk);
					
					if(blk.nHeight == nLastHeight + 1000)
					{
						
						/* Request blocks if there is a node. */
						LLP::CLegacyNode* pNode = SelectNode();
						if(pNode)
						{
							nLastHeight   = blk.nHeight;
							
							std::vector<uint1024> vLast = { hash };
							pNode->PushMessage("getblocks", Core::CBlockLocator(vLast), uint1024(0));
							
							if(GetArg("-verbose", 0) >= 1)
								printf("***** Manager::Requested from stop (%s) block range (%s ... 0000000)\n", pNode->GetIPAddress().c_str(), hash.ToString().substr(0, 20).c_str());
							
							nLastBlockRequest = UnifiedTimestamp();
						}
						
						break;
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
			if(GetArg("-verbose", 0) >= 1)
				printf("***** Manager::Process Queue with %u Items\n", vBlocks.size());

			
			/* Run through the list of blocks to see if they need to be connected. */
			Timer cTimer;
			for(auto hash : vBlocks)
			{
				cTimer.Reset();
					
				
				/* Get the Block from the Memory Pool. */
				CBlock block;
				blkPool.Get(hash, block);

				
				/* Check that previous block exists. */
				if(blkPool.State(block.hashPrevBlock) == blkPool.NOTFOUND || !mapBlockIndex.count(block.hashPrevBlock)) //NOTE: mapBlockIndex to be deprecated
				{
					//printf("ORPHANED Height %u %s by Invalid Previous State(%u)\n", block.nHeight, block.hashPrevBlock.ToString().substr(0, 20).c_str(), blkPool.State(block.hashPrevBlock));
					
					/* Request blocks if there is a node. */
					LLP::CLegacyNode* pNode = SelectNode();
					if(pNode)
					{
						LLP::CInv inv(LLP::MSG_BLOCK, block.hashPrevBlock);
						std::vector<LLP::CInv> vInv = { inv };
						pNode->PushMessage("getdata", vInv);
							
						if(GetArg("-verbose", 0) >= 1)
							printf("***** Manager::Requested ORPHAN %s\n", hash.ToString().substr(0, 20).c_str());
						
					}
					
					break;
				}
				
				/* Check the block to the blockchain. */
				if(blkPool.Accept(block))
				{
					/* If this isn't the main chain synchronization broadcast the block to other nodes. */
					if(!fSynchronizing)
					{
						std::vector<LLP::CInv> vInv = { LLP::CInv(LLP::MSG_BLOCK, hash) };
						std::vector<LLP::CLegacyNode*> vNodes = GetConnections();
						for(auto node : vNodes)
							node->PushMessage("inv", vInv);
					}
					
					/* Keep the Meter data up to date. */
					nProcessed++;
					if(GetArg("-verbose", 0) >= 2)
						printf("ACCEPTED %s in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());
					
				}
				else
				{
					if(GetArg("-verbose", 0) >= 2)
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
			printf("METER %f block/s | best=%s | height=%d | trust=%" PRIu64 "\n", RPS, hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust);
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
		
		std::vector<LLP::CLegacyNode*> vNodes = GetConnections();
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
		fStarted = true;
		printf("##### Node Started #####\n");
		
		std::vector<LLP::CAddress> vSeeds = LLP::DNS_Lookup(fTestNet ? LLP::DNS_SeedNodes_Testnet : LLP::DNS_SeedNodes);
		for(int nIndex = 0; nIndex < vSeeds.size(); nIndex++)
			AddAddress(vSeeds[nIndex]);
		
	}
}
