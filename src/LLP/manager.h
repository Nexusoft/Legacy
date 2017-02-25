/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_MANAGER_H
#define NEXUS_LLP_MANAGER_H

#include "node.h"

namespace LLP
{
	
	namespace MANAGER
	{
		
		/* Manager Mutex for thread safety. */
		extern Mutex_t MANAGER_MUTEX;
		
		
		/* Connected Nodes and their Pointer Reference. */
		extern std::vector<CNode*> vNodes;
		
		
		/* Tried Address in the Manager. */
		extern std::vector<CAddrInfo*> vTried;
		
		
		/* New Addresses in the Manager. */
		extern std::vector<CAddrInfo*> vNew;
		
		
		/* The Server Running to Handle Incoming / Outgoing connections. */
		extern Server<CNode> MANAGER_SERVER;
		
		
		/* Connection Manager Thread. */
		void ConnectionManager();
		
		
		/* Handle and Process New Blocks. */
		void BlockManager();
		
		
		/* Handle and Process New Transactions. */
		void TransactionManager();
		
		
		/* Relay a Message to all Connected Nodes. */
		void FloodRelay(const CInv* inv);
		
		
		/* Find a Node in this Manager by Net Address. */
		CNode* FindNode(const CNetAddr& ip)
		{
			{ LOCK_GUARD(MANAGER_MUTEX);
				
				BOOST_FOREACH(CNode* pnode, vNodes)
					if ((CNetAddr)pnode->addrThisNode == ip)
						return (pnode);
			}
			return NULL;
		}

		
		/* Find a Node in this Manager by Sercie Address. */
		CNode* FindNode(const CService& addr)
		{
			{ LOCK_GUARD(MANAGER_MUTEX);

				BOOST_FOREACH(CNode* pnode, vNodes)
					if ((CService)pnode->addrThisNode == addr)
						return (pnode);
			}
			return NULL;
		}
		
	};
}

#endif
