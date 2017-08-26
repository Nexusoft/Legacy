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
	
	
	/* NODE MANAGER NOTE:
	 * 
	 * The Node Manager handles all the connections associated with a specifric node and rates them based on their interactions and trust they have built in th network
	 * 
	 * It is also intelligent to distinguish the best nodes to connect with and also process blocks in a single location so that there are not more processes that need happen
	 * and that everything can be done in the order it was inteneded.
	 */
	
	
	void Manager::TimestampManager()
	{

	}
	
	
	void Manager::ConnectionManager()
	{
		while(!fStarted)
			Sleep(1000);
		
		
		//FOR TESTING ONLY
		AddConnection("104.192.170.130", "9323");
		AddConnection("96.43.131.82", "9323");
		AddConnection("104.192.170.30", "9323");
		AddConnection("54.169.195.135", "9323");
		
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}

			//TODO: Make this tied to port macros
			int nRandom = GetRandInt(vNew.size() - 1);
			
			
			/* Attempt to make connections from the manager. */
			printf("##### Connection Manager::attempting connection %s\n", vNew[nRandom].ToStringIP().c_str());
			if(!AddConnection(vNew[nRandom].ToStringIP(), "9323"))
				vTried.push_back(vNew[nRandom]);
				
			vNew.erase(vNew.begin() + nRandom);
			Sleep(5000);
		}
	}

	
	bool SortByHeight(const uint1024& nFirst, const uint1024& nSecond)
	{ 
		CBlock blkFirst, blkSecond;
		
		pManager->blkPool.Get(nFirst, blkFirst);
		pManager->blkPool.Get(nSecond, blkSecond);
		
		return blkFirst.nHeight < blkSecond.nHeight;
	}
	
	
	bool SortByLatency(const LLP::CNode* nFirst, const LLP::CNode* nSecond)
	{ 
		return nFirst->nNodeLatency < nSecond->nNodeLatency;
	}
		
		
	/* Blocks are checked in the order they are recieved. */
	void Manager::BlockProcessor()
	{
		while(!fStarted)
			Sleep(1000);
		
		unsigned int nLastBlockRequest = 0;
		while(!fShutdown)
		{
			Sleep(100);
			
			/* Handle Orphan Checking. */
			std::vector<uint1024> vOrphans;
			if(blkPool.GetIndexes(blkPool.ORPHANED, vOrphans))
			{
				//if(GetArg("-verbose", 0) >= 2)
				//	printf("##### Block Processor::queued Job of %u Orphans.\n", vOrphans.size());
				
				std::sort(vOrphans.begin(), vOrphans.end(), SortByHeight);
				for(auto hash : vOrphans)
				{
					CBlock block;
					blkPool.Get(hash, block);
					
					if(!blkPool.Has(block.hashPrevBlock) && !Core::mapBlockIndex.count(block.hashPrevBlock))
						continue;
						
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::PROCESSING %s\n", hash.ToString().substr(0, 20).c_str());
					
					if(!blkPool.Process(block, NULL))
						continue;
				}
			}
			
			/* Get some more blocks if the block processor is waiting. */
			std::vector<uint1024> vBlocks;
			if(!blkPool.GetIndexes(blkPool.ACCEPTED, vBlocks))
			{
				std::vector<LLP::CNode*> vNodes = GetConnections();
				if(vNodes.size() == 0)
					continue;
				
				/* Only ask lowest latency nodes. */
				std::sort(vNodes.begin(), vNodes.end(), SortByLatency);
				if((nBestHeight < cPeerBlocks.Majority() && nLastBlockRequest + 15 < Core::UnifiedTimestamp()) ||
					nLastBlockRequest + 30 < Core::UnifiedTimestamp())
				{
					if(GetArg("-verbose", 0) >= 2)
						printf("##### Block Processor::Requested Blocks from node %s (%u ms)\n",  vNodes[0]->GetIPAddress().c_str(), vNodes[0]->nNodeLatency);
				
					nLastBlockRequest = Core::UnifiedTimestamp();
					vNodes[0]->PushMessage("getblocks", Core::CBlockLocator(Core::pindexBest), uint1024(0));
				}
				
				continue;
			}
			
			
			if(GetArg("-verbose", 0) >= 2)
				printf("##### Block Processor::queued Job of %u Blocks.\n", vBlocks.size());
			
			std::sort(vBlocks.begin(), vBlocks.end(), SortByHeight);
			
			LLD::CIndexDB indexdb("r+");
			for(auto hash : vBlocks)
			{
				if(GetArg("-verbose", 0) >= 3)
					printf("##### Block Processor::PROCESSING %s\n", hash.ToString().substr(0, 20).c_str());
				
				CBlock block;
				blkPool.Get(hash, block);
				
				
				/* Start the LLD transaction. */
				indexdb.TxnBegin();
					
				/* Set the best chain if it is highest trust.
				 * TODO: Have this processor check different chain states.
				 */
				CBlockIndex* pindexNew = mapBlockIndex[hash];
				
				
				/* Write the Index to DISK. */
				if(!indexdb.WriteBlockIndex(CDiskBlockIndex(pindexNew)))
					continue;
				else
					blkPool.SetState(hash, blkPool.INDEXED);
			
				if (pindexNew->nChainTrust > nBestChainTrust)
				{
					if (!blkPool.Connect(indexdb, pindexNew, NULL))
					{
						printf("##### Block Processor::Connect failed\n");
						indexdb.TxnAbort();
						
						continue; //TODO: Recycle X times (add to holding object)
					}
					
					blkPool.SetState(hash, blkPool.MAINCHAIN);
					
					if(GetArg("-verbose", 0) >= 2)
						printf("##### Block Processor::CONNECTED Block %s Height %u\n", hash.ToString().substr(0, 20).c_str(), block.nHeight);
				}
				else
				{
					//blkPool.SetState(hash, blkPool.FORKCHAIN);
					
					printf("##### Block Processor::FORKED Block %s Height %u\n");
				}
				
				indexdb.TxnCommit();
			}
			
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
