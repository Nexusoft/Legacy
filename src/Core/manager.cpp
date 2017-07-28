/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/manager.h"
#include "include/inventory.h"
#include "include/hosts.h"

#include "../LLC/include/random.h"

namespace Core
{
	/* NODE MANAGER NOTE:
	 * 
	 * The Node Manager handles all the connections associated with a specifric node and rates them based on their interactions and trust they have built in th network
	 * It is also intelligent to distinguish the best nodes to connect with and also process blocks in a single location so that there are not more processes that need happen
	 * and that everything can be done in the order it was inteneded.
	 */
	
	/* Time Seed Manager. */
	void TimestampManager();
		
		
	/* Connection Manager Thread. */
	void ConnectionManager();
		
		
	/* Handle and Process New Blocks. */
	void BlockProcessor();
		
		
	/* Add address to the Queue. */
	void NodeManager::AddAddress(LLP::CAddress cAddress)
	{
		LOCK(MANAGER_MUTEX);
		
		vNew.push_back(cAddress);
	}
		
		
	/* Add a node to the manager by pointer reference. This is for if the node has already been connected outside of the class. */
	void NodeManager::AddNode(LLP::CNode* pnode)
	{
		LOCK(MANAGER_MUTEX);
		
		std::vector<LLP::CNode*>::iterator it = find(vNodes.begin(), vNodes.end(), pNode);
		if(it != vNodes.end())
			vNodes.push_back(pnode);
	}
		
		
	/* Remove a node from the manager by its address as reference. */
	void RemoveNode(LLP::CAddress cAddress);
		
		
	/* Remove a node from the manager by its class as reference. */
	void NodeManager::RemoveNode(LLP::CNode* pnode)
	{
		LOCK(MANAGER_MUTEX);
		
		std::vector<LLP::CNode*>::iterator it = find(vNodes.begin(), vNodes.end(), pNode);
		if(it != vNodes.end())
			vNodes.erase(it);
	}
		
		
	/* Find a Node in this Manager by Net Address. */
	LLP::CNode* FindNode(const LLP::CNetAddr& ip);
		
		
	/* Find a Node in this Manager by Sercie Address. */
	LLP::CNode* FindNode(const LLP::CService& addr); 
		
		
	/* Get a random address from the active connections in the manager. */
	LLP::CAddress NodeManager::GetRandAddress(bool fNew)
	{
		std::vector<LLP::CAddress> vSelect;
		if(fNew)
			vSelect.insert(vSelect.begin(), vNew.begin(), vNew.end());
		
		
	}
		
		
	/* Start up the Node Manager. */
	void NodeManager::Start()
	{
		pServer = new LLP::Server<LLP::CNode>(LLP::GetDefaultPort(), 5, true, 2, 50, 30, 30, true, true);
		
		std::vector<LLP::CAddress> vSeeds = LLP::DNS_Lookup(fTestNet ? DNS_SeedNodes_Testnet : DNS_SeedNodes);
		for(int nIndex = 0; nIndex < vSeeds.size(); nIndex++)
			AddAddress(vSeeds[nIndex]);
		
	}
	
	

	
	

	
	

	
	
	/* Remove a node from the manager by its address as reference. */
	void RemoveNode(LLP::CAddress cAddress) 
	{ 
		//TODO: Finish this method
		
	}
	
	
	//TODO: Make the Clock regulator more advanced to do multiple checks on possible clock 
	void NodeManager::ClockRegulator()
	{
		
		
	}
		

	
	
	void NodeManager::TimestampManager()
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
				Sleep(1000);
				
				{ LOCK(vNodes[nIndex]->NODE_MUTEX);
					
				
				
				}
			}
		}
	}
	
	void NodeManager::ConnectionManager()
	{
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}
			
			{ LOCK(MANAGER_MUTEX);
				
				//TODO: Make this tied to port macros
				if(!pServer->AddConnection(vNew[0].ToStringIP(), "9323"))
					vTried.push_back(vNew[0]);
				
				vNew.erase(vNew.begin());
			}
			
			Sleep(5000);
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
				
				{ LOCK(vNodes[nIndex]->NODE_MUTEX);
				
					/* If there are no blocks in the queue continue on the loop. */
					if(vNodes[nIndex]->queueBlocks.size() == 0)
						continue;
                
					/* Extract the block from the queue. */
					CBlock* pblock = &vNodes[nIndex]->queueBlocks.front();
					vNodes[nIndex]->queueBlocks.pop();
					
					if(mapBlockIndex.count(pblock->GetHash()))
					{
						//TODO: Verbose output here verbose > 3 to signal repeat blocks that are already in the blockchain
						
						continue;
					}
					
					
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
