/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_MANAGER_H
#define NEXUS_CORE_INCLUDE_MANAGER_H

#include "../../LLP/include/node.h"
#include "../../LLU/include/mutex.h"

#include <boost/thread/thread.hpp>  

namespace Core
{

	class NodeManager
	{
	public:
		
		NodeManager() {}
		
		/* Manager Mutex for thread safety. */
		Mutex_t MANAGER_MUTEX;
		
		
		/* Connection Manager Thread. */
		void ConnectionManager();
		
		
		/* Handle and Process New Blocks. */
		void BlockProcessor();
		
		
		/* Handle and Process New Transactions. */
		void TransactionManager();
		
		
		/* Find a Node in this Manager by Net Address. */
		LLP::CNode* FindNode(const LLP::CNetAddr& ip);

		
		/* Find a Node in this Manager by Sercie Address. */
		LLP::CNode* FindNode(const LLP::CService& addr);
		
	private:
		
		/* Connected Nodes and their Pointer Reference. */
		std::map<LLP::CAddress, LLP::CNode*> mapNodes;
		
		
		/* Tried Address in the Manager. */
		std::vector<LLP::CAddrInfo> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<LLP::CAddrInfo> vNew;
		
		
		/* The Server Running to Handle Incoming / Outgoing connections. */
		//Server<CNode> DATA_SERVER;
		
	};
}

#endif
