/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_MANAGER_H
#define NEXUS_CORE_INCLUDE_MANAGER_H

#include "../../LLP/templates/server.h"

#include "../../LLP/include/node.h"
#include "../../Util/include/mutex.h"

#include <boost/thread/thread.hpp>

namespace Core
{
	
	template<typename T1>
	void RelayMessage(LLP::CInv inv, const T1& obj)
	{
		//TODO: connect with Node Manager / LLP Relay Layer
	
	}
	
	
	/** Node Manager Class:
	 * 
	 * This is resonsilbe for the managing of all the nodes in the Tritum Protoco.
	 * It is responsible for overseeing all the connections, processing blocks, and handling the network wide relays.
	 * 
	 * This is necessary to keep all the main processing for a node here in this specifric class so that the other services a node can provide be easy to integrate and extend.
	 * 
	 * This is also where a node will be keeping track of the differences in the time seeking and also the intelligence of the trust that is seen indepent of any of the network wide trust.
	 */
	class Manager : public LLP::Server<LLP::CNode>
	{
	public:
		
		Manager() {}
		
		/* Manager Mutex for thread safety. */
		Mutex_t MANAGER_MUTEX;
		
		
		/* Time Seed Manager. */
		void TimestampManager();
		
		
		/* Connection Manager Thread. */
		void ConnectionManager();
		
		
		/* Handle and Process New Blocks. */
		void BlockProcessor();
		
		
		/* Add address to the Queue. */
		void AddAddress(LLP::CAddress cAddress);
		
		
		/* Add a node to the manager by pointer reference. This is for if the node has already been connected outside of the class. */
		void AddNode(LLP::CNode* pnode);
		
		
		/* Remove a node from the manager by its address as reference. */
		void RemoveNode(LLP::CAddress cAddress);
		
		
		/* Remove a node from the manager by its class as reference. */
		void RemoveNode(LLP::CNode* pnode);
		
		
		/* Find a Node in this Manager by Net Address. */
		LLP::CNode* FindNode(const LLP::CNetAddr& ip);
		
		
		/* Find a Node in this Manager by Sercie Address. */
		LLP::CNode* FindNode(const LLP::CService& addr); 
		
		
		/* Get a random address from the active connections in the manager. */
		LLP::CAddress GetRandAddress(bool fNew = false);
		
		
		/* Start up the Node Manager. */
		void Start();
		
		
	private:
		
		/* Connected Nodes and their Pointer Reference. */
		std::vector<LLP::CAddress> vNodes;
		
		
		/* Tried Address in the Manager. */
		std::vector<LLP::CAddress> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<LLP::CAddress> vNew;
		
		
	};
}

#endif
