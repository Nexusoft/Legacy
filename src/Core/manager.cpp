/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../LLP/templates/server.h"
#include "../LLP/include/protocol.h"
#include "../LLP/include/hosts.h"

namespace LLP
{
	
	void Manager::ConnectionManager((void* parg)
	{
		
		/* TODO: Search through all added connections and attempt to make connections withup to max_connection_count nodes. */
		
	}
	
	
	void Manager::BlockProcessor((void* parg)
	{
		
		/* TODO: Handle a loop of the block queue to handle block processing. */
		
		// Check for duplicate
		uint1024 hash = pblock->GetHash();
		if (mapBlockIndex.count(hash))
			return error("ProcessBlock() : already have block %d %s", mapBlockIndex[hash]->nHeight, hash.ToString().substr(0,20).c_str());
		if (mapOrphanBlocks.count(hash))
			return error("ProcessBlock() : already have block (orphan) %s", hash.ToString().substr(0,20).c_str());

		// Preliminary checks
		if (!pblock->CheckBlock())
			return error("ProcessBlock() : CheckBlock FAILED");
		

		// If don't already have its previous block, shunt it off to holding area until we get it
		if (!mapBlockIndex.count(pblock->hashPrevBlock))
		{
			if(GetArg("-verbose", 0) >= 0)
				printf("ProcessBlock: ORPHAN BLOCK, prev=%s\n", pblock->hashPrevBlock.ToString().substr(0,20).c_str());
			
			CBlock* pblock2 = new CBlock(*pblock);
			mapOrphanBlocks.insert(make_pair(hash, pblock2));
			mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrevBlock, pblock2));
			
			/** Simple Catch until I finish Checkpoint Syncing. **/
            if(pfrom)
                pfrom->PushGetBlocks(pindexBest, 0);
			
			return true;
		}

		// Store to disk
		if (!pblock->AcceptBlock())
			return error("ProcessBlock() : AcceptBlock FAILED");


		// Recursively process any orphan blocks that depended on this one
		vector<uint1024> vWorkQueue;
		vWorkQueue.push_back(hash);
		for (unsigned int i = 0; i < vWorkQueue.size(); i++)
		{
			uint1024 hashPrev = vWorkQueue[i];
			for (multimap<uint1024, CBlock*>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev);
				 mi != mapOrphanBlocksByPrev.upper_bound(hashPrev);
				 ++mi)
			{
				CBlock* pblockOrphan = (*mi).second;
				if (pblockOrphan->AcceptBlock())
					vWorkQueue.push_back(pblockOrphan->GetHash());
					
				mapOrphanBlocks.erase(pblockOrphan->GetHash());
				delete pblockOrphan;
			}
			mapOrphanBlocksByPrev.erase(hashPrev);
		}

		printg("ProcessBlock: ACCEPTED %s\n", pblock->GetHash().ToString().substr(0, 10).c_str());

		return true;
		
	}
	
	
	void Manager::TransactionManager((void* parg)
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
	}
}
