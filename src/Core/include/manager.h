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
	
	class InventoryManager
	{
    public:
        
        /* State level messages to hold information about all inventory. */
        enum
        {
            //TODO: Leave room for more valid states (passes basic checks)
            UNVERIFIED = 0,
            ACCEPTED   = 1,
            ORPHANED   = 2,
            
            ON_CHAIN   = 3,
            MEMORY     = 4,
            DISK       = 5,
            
            
            //TODO: Leave room for more invalid states
            INVALID    = 128
            
        };
        
        InventoryManager() {}
        
        /* Class Mutex. */
        Mutex_t MUTEX;
        
        
        /* Check for Tranasction. */
        bool Has(uint512 hashTX) { return mapTransactions.count(hashTX); }
        
        
        /* Check for Blocks. */
        bool Has(uint1024 hashBlock) { return mapBlocks.count(hashBlock); }
        
        
        /* Add a block to know inventory. */
        void Set(uint1024 hashBlock, unsigned char nState = UNVERIFIED)
        {
            LOCK(MUTEX);
            
            mapBlocks[hashBlock] = nState;
        }
        
        
        /* Add a transaction to known inventory. */
        void Set(uint512 hashTX, unsigned char nState = UNVERIFIED)
        {
            LOCK(MUTEX);
            
            mapTransactions[hashTX] = nState;
        }
        
        
        /* Get the State of the Transaction from the Inventory. */
        unsigned char State(uint512 hashTX) { return mapTransactions[hashTX]; }
        
        
        /* Get the State of the Block from the Inventory. */
        unsigned char State(uint1024 hashBlock) { return mapBlocks[hashBlock]; }
        
        
    private:
        
        /* Map of the current blocks recieved. */
        std::map<uint1024, unsigned char> mapBlocks;
        
        
        /* Map of the current transaction received. */
        std::map<uint512, unsigned char> mapTransactions;
        
    };
	
	
	class NodeManager
	{
	public:
		
		NodeManager() {}
		
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
		
		
		/* Add a node to the manager. */
		void AddNode(LLP::CNode* pNode);
		
		
		/* Remove a node from the manager. */
		void RemoveNode(LLP::CNode* pNode);
		
		
		/* Find a Node in this Manager by Net Address. */
		LLP::CNode* FindNode(const LLP::CNetAddr& ip);

		
		/* Find a Node in this Manager by Sercie Address. */
		LLP::CNode* FindNode(const LLP::CService& addr);
		
		
		/* Start up the Node Manager. */
		void Start();
		
	private:
		
		/* Connected Nodes and their Pointer Reference. */
		std::vector<LLP::CNode*> vNodes;
		
		
		/* Tried Address in the Manager. */
		std::vector<LLP::CAddress> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<LLP::CAddress> vNew;
        
        
        /* Server that process data connections. */
        LLP::Server<LLP::CNode>* pServer;
        
	};
}

#endif
