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
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}

			//TODO: Make this tied to port macros
			if(!AddConnection(vNew[0].ToStringIP(), "9323"))
				vTried.push_back(vNew[0]);
				
			vNew.erase(vNew.begin());
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
		
		
	/* Blocks are checked in the order they are recieved. */
	void Manager::BlockProcessor()
	{
		while(!fShutdown)
		{
			Sleep(100);
			
			/* Handle Orphan Checking. */
			std::vector<uint1024> vOrphans;
			if(blkPool.GetIndexes(blkPool.ORPHANED, vOrphans))
			{
				if(GetArg("-verbose", 0) >= 2)
					printf("##### Block Processor::queued Job of %u Orphans.\n", vOrphans.size());
				
				std::sort(vOrphans.begin(), vOrphans.end(), SortByHeight);
				for(auto hash : vOrphans)
				{
					CBlock block;
					blkPool.Get(hash, block);
					
					if(!blkPool.Has(block.hashPrevBlock) && !Core::mapBlockIndex.count(block.hashPrevBlock))
						continue;
						
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::PROCESSING %s\n", hash.ToString().substr(0, 20).c_str());
					
					/* Timer object for runtime calculations. */
					Timer cTimer;
					cTimer.Reset();
					
					/* Check the Block validity. TODO: Send process message for Trust Depreciation (LLP:Dos). */
					if(blkPool.Check(block, NULL))
					{
						if(GetArg("-verbose", 0) >= 3)
							printf("PASSED checks in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
						
						/* Set the proper state for the new block. */
						blkPool.SetState(hash, blkPool.VERIFIED);
					}
					else
					{
						if(GetArg("-verbose", 0) >= 3)
							printf("INVALID checks in %" PRIu64 " us", cTimer.ElapsedMicroseconds());
						
						/* Set the proper state for the new block. */
						blkPool.SetState(hash, blkPool.INVALID);
						
						continue;
					}
				}
			}
			
			std::vector<uint1024> vBlocks;
			if(!blkPool.GetIndexes(blkPool.VERIFIED, vBlocks))
				continue;
			
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
				
				if (!CheckDiskSpace(::GetSerializeSize(block, SER_DISK, DATABASE_VERSION)))
				{
					printf("##### Block Processor::out of disk space");
					
					continue; //TODO: Recycle X times (add to holding object)
				}
				
				
				/* Check the block to the blockchain. */
				Timer cTimer;
				cTimer.Reset();
				if(blkPool.Accept(block, NULL))
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::ACCEPTED in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
					
					/* Set the proper state for the new block. */
					blkPool.SetState(hash, blkPool.ACCEPTED);
				}
				else
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::REJECTED in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
					
					/* Set the proper state for the new block. */
					blkPool.SetState(hash, blkPool.REJECTED);
					
					continue;
				}
				
				
				/* Populate Index Data
				* 
				* TODO: Remove this once block indexing is done in LLD
				*/
				unsigned int nFile = 0;
				unsigned int nBlockPos = 0;
				if (!block.WriteToDisk(nFile, nBlockPos))
				{
					printf("##### Block Processor::WriteToDisk failed");
					
					continue; //TODO: Recylce X times (add to holding object)
				}
				
				
				indexdb.TxnBegin();
				
				Core::CBlockIndex* pindexNew = new Core::CBlockIndex(nFile, nBlockPos, block);
				if (!pindexNew || !blkPool.Index(block, pindexNew, NULL))
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::FAILED INDEX in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
					
					continue;
				}
				
				if(!indexdb.WriteBlockIndex(CDiskBlockIndex(pindexNew)))
				{
					if(GetArg("-verbose", 0) >= 3)
						printf("##### Block Processor::FAILED WRITE INDEX in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
					
					continue;
				}
					

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
						printf("##### Block Processor::CONNECTED Block %s Height %u (%" PRIu64 ")\n", hash.ToString().substr(0, 20).c_str(), block.nHeight, cTimer.ElapsedMicroseconds());
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
		std::vector<LLP::CAddress> vSeeds = LLP::DNS_Lookup(fTestNet ? LLP::DNS_SeedNodes_Testnet : LLP::DNS_SeedNodes);
		for(int nIndex = 0; nIndex < vSeeds.size(); nIndex++)
			AddAddress(vSeeds[nIndex]);
		
	}
}
