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
		
		
		//FOR TESTING ONLY
		AddConnection("104.192.170.130", "9323");
		AddConnection("104.192.169.10", "9323");
		AddConnection("104.192.170.30", "9323");
		AddConnection("104.192.169.62", "9323");
		AddConnection("96.43.131.82", "9323");
		
		
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}

			//TODO: Make this tied to port macros
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
	LLP::CNode* Manager::SelectNode()
	{
		std::vector<LLP::CNode*> vNodes = GetConnections();
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
		hashLastBlock = pindexBest->GetBlockHash();
		
		fSynchronizing = true;
		while(!fShutdown)
		{
			Sleep(1000);
			
			if(fSynchronizing && nBestHeight > cPeerBlocks.Majority())
				fSynchronizing = false;
			
			if(!fSynchronizing)
				hashLastBlock = pindexBest->GetBlockHash();
			
			
			/* Find what intervals are going to be requested. */
			uint1024 hashBegin = hashLastBlock;
			uint1024 hashEnd  = uint1024(0);

			
			/* Check for blocks that have been asked for, and re-try if failed. */
			std::vector<uint1024> vBlocks;
			if(blkPool.GetIndexes(blkPool.CHECKED, vBlocks))
			{
				std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);
				
				CBlock blk;
				blkPool.Get(vBlocks.front(), blk);
				if(blk.hashPrevBlock != pindexBest->GetBlockHash())
				{
					LLP::CNode* pNode = SelectNode();
					if(pNode)
					{
						pNode->PushMessage("getblocks", Core::CBlockLocator(pindexBest), uint1024(0));
					
						if(GetArg("-verbose", 0) >= 1)
							printf("***** Manager::Requested Missing (%s) block range (%s ... 0000000)\n", pNode->GetIPAddress().c_str(), hashBegin.ToString().substr(0, 20).c_str());
					}
				}
			}
				
				
			/* Request new blocks if requests for new blocks or been 15 seconds. */
			else if(nLastBlockRequest + 15 < UnifiedTimestamp())
			{
				/* Request blocks if there is a node. */
				LLP::CNode* pNode = SelectNode();
				if(pNode)
				{
					std::vector<uint1024> vBegin = { hashBegin };
					pNode->PushMessage("getblocks", Core::CBlockLocator(vBegin), hashEnd);
					
					if(GetArg("-verbose", 0) >= 1)
						printf("***** Manager::Requested (%s) block range (%s ... 0000000)\n", pNode->GetIPAddress().c_str(), hashBegin.ToString().substr(0, 20).c_str());
					
					nLastBlockRequest = UnifiedTimestamp();
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
			Sleep(2000);
				
			
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
				if(blkPool.State(block.hashPrevBlock) != blkPool.CONNECTED &&
				   blkPool.State(block.hashPrevBlock) != blkPool.ACCEPTED )
				{
					printf("ORPHANED Height %u %s by Invalid Previous State(%u)\n", block.nHeight, block.hashPrevBlock.ToString().substr(0, 20).c_str(), blkPool.State(block.hashPrevBlock));
					
					break;
				}
				
				/* Check the block to the blockchain. */
				if(blkPool.Accept(block, NULL))
				{
					
					/* Keep the Meter data up to date. */
					nProcessed++;
					if(GetArg("-verbose", 0) >= 2)
						printf("ACCEPTED %s in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());
				}
				else
				{
					if(GetArg("-verbose", 0) >= 2)
						printf("REJECTED %s in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());
							
					//blkPool.SetState(hash, blkPool.ERROR_ACCEPT);
					
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
