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
	
	class Manager
	{
		Mutex_t MANAGER_MUTEX;
		
		/* Connected Nodes and their Pointer Reference. */
		std::vector<CNode*> vNodes;
		
		
		/* Tried Address in the Manager. */
		std::vector<CAddress> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<CAddress> vNew;
		
		
	public:
		
		NodeManager(int nTotalThreads, bool fImplementCore = true)
		{
			
		}
		
		
		/* Find a Node in this Manager. */
		CNode* FindNode(const CNetAddr& ip)
		{
			{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
					if ((CNetAddr)pnode->addrThisNode == ip)
						return (pnode);
			}
			return NULL;
		}

		
		CNode* FindNode(const CService& addr)
		{
			{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
					if ((CService)pnode->addrThisNode == addr)
						return (pnode);
			}
			return NULL;
		}
	
		/* Last Node Status. */
		
	};
}

#endif
