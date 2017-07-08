/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/manager.h"
#include "include/hosts.h"

namespace Core
{
	
	std::vector<std::string> DNS_SeedNodes =
	{
		"node1.nexusearth.com",
		"node1.mercuryminer.com",
		"node1.nexusminingpool.com",
		"node1.nxs.efficienthash.com",
		"node2.nexusearth.com",
		"node2.mercuryminer.com",
		"node2.nexusminingpool.com",
		"node2.nxs.efficienthash.com",
		"node3.nexusearth.com",
		"node3.mercuryminer.com",
		"node3.nxs.efficienthash.com",
		"node4.nexusearth.com",
		"node4.mercuryminer.com",
		"node4.nxs.efficienthash.com",
		"node5.nexusearth.com",
		"node5.mercuryminer.com",
		"node5.nxs.efficienthash.com",
		"node6.nexusearth.com",
		"node6.mercuryminer.com",
		"node6.nxs.efficienthash.com",
		"node7.nexusearth.com",
		"node7.mercuryminer.com",
		"node7.nxs.efficienthash.com",
		"node8.nexusearth.com",
		"node8.mercuryminer.com",
		"node8.nxs.efficienthash.com",
		"node9.nexusearth.com",
		"node9.mercuryminer.com",
		"node9.nxs.efficienthash.com",
		"node10.nexusearth.com",
		"node10.mercuryminer.com",
		"node10.nxs.efficienthash.com",
		"node11.nexusearth.com",
		"node11.mercuryminer.com",
		"node11.nxs.efficienthash.com",
		"node12.nexusearth.com",
		"node12.mercuryminer.com",
		"node12.nxs.efficienthash.com",
		"node13.nexusearth.com",
		"node13.mercuryminer.com",
		"node13.nxs.efficienthash.com",
	};
	
	
	std::vector<std::string> DNS_SeedNodes_Testnet = 
	{
		"test1.nexusoft.io"
	};
	
	
	void NodeManager::Start()
	{
		pServer = new LLP::Server<LLP::CNode>(9323, 5, true, 2, 50, 30, 30, true, true);
	    
		std::vector<LLP::CAddress> vSeeds    = LLP::DNS_Lookup(fTestNet ? DNS_SeedNodes_Testnet : DNS_SeedNodes);
		for(int nIndex = 0; nIndex < vSeeds.size(); nIndex++)
			AddAddress(vSeeds[nIndex]);
        
	}
	
	
	void NodeManager::AddAddress(LLP::CAddress cAddress)
	{
		LOCK(MANAGER_MUTEX);
        
		vNew.push_back(cAddress);
	}
	
	
	void NodeManager::AddNode(LLP::CNode* pnode)
	{
		LOCK(MANAGER_MUTEX);
		
		vNodes.push_back(pnode);
	}
	
	void NodeManager::RemoveNode(LLP::CNode* pNode)
	{
		LOCK(MANAGER_MUTEX);
		
		std::vector<LLP::CNode*>::iterator it = find(vNodes.begin(), vNodes.end(), pNode);
		if(it != vNodes.end())
			vNodes.erase(it);
	}

	
	void NodeManager::TimestampManager()
	{
	
		/* These checks are for after the first time seed has been established. 
		if(fTimeUnified)
		{
				

			if(nOffset > GetUnifiedAverage() + LLP::MAX_UNIFIED_DRIFT || nOffset < GetUnifiedAverage() - LLP::MAX_UNIFIED_DRIFT ) {
				printf("***** Core LLP: Unified Samples Out of Drift Scope Current (%u) Samples (%u)\n", GetUnifiedAverage(), nOffset);
						
				DDOS->rSCORE += 10;
						
				if(mapBadResponse.count("offset"))
					mapBadResponse["offset"] = 1;
				else
					mapBadResponse["offset"] ++ ;
						
				return true;
			}
		}
		*/
	}
	
	void NodeManager::ConnectionManager()
	{
        while(!fShutdown)
        {
            if(vNodes.size() == 0){
                Sleep(1000);
                
                continue;
            }
            
        }
	}
	
	
	/* Blocks are checked in the order they are recieved. */
	void NodeManager::BlockProcessor()
	{
        while(!fShutdown)
        {
            if(vNodes.size() == 0){
                Sleep(1000);
                
                continue;
            }
            
            
            /* Loop the queues of the nodes that are currently connected. */
            for(int nIndex = 0; nIndex < vNodes.size(); nIndex++)
            {
                Sleep(1);
                
                { LOCK(MANAGER_MUTEX);
                
                    /* If there are no blocks in the queue continue on the loop. */
                    if(vNodes[nIndex]->queueBlocks.size() == 0)
                        continue;
                
                    /* Extract the block from the queue. */
                    CBlock* pblock = &vNodes[nIndex]->queueBlocks.front();
                    vNodes[nIndex]->queueBlocks.pop();
                
                    /* Check the Block. */
                    if (!CheckBlock(pblock, vNodes[nIndex]))
                        continue;
                    
                    /* Accept the Block. */
                    if (!AcceptBlock(pblock, vNodes[nIndex]))
                        continue;
                
                }
            }
        }
	}
	
	/*
	
	void NodeManager::TransactionManager()
	{
		vector<uint512> vWorkQueue;
		vector<uint512> vEraseQueue;
				
		CDataStream vMsg(vRecv);
		LLD::CIndexDB indexdb("r");
			
		Core::CTransaction tx;
		ssMessage >> tx;

		CInv inv(MSG_TX, tx.GetHash());
		AddInventoryKnown(inv);

		bool fMissingInputs = false;
		if (tx.AcceptToMemoryPool(indexdb, true, &fMissingInputs))
			{
				Core::SyncWithWallets(tx, NULL, true);
				
				RelayMessage(inv, vMsg);
			
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
	*/
}
