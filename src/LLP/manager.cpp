/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "Templates/server.h"
#include "Include/protocol.h"
#include "Include/hosts.h"

namespace LLP
{
	
	namespace MANAGER
	{
		/* Manager Mutex for thread safety. */
		Mutex_t MANAGER_MUTEX;
		
		
		/* Connected Nodes and their Pointer Reference. */
		std::vector<CNode*> vNodes;
		
		
		/* Tried Address in the Manager. */
		std::vector<CAddrInfo*> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<CAddrInfo*> vNew;
		
		
		void TransactionManager()
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
