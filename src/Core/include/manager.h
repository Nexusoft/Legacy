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
	
	
	/** Inventory Manager Class:
     * 
     * This class is responsible for overseeing all of the inventory that has been recieved by a node.
     * Inventory can be seen as items such as blocks and transactions are accurately reported and understood
     * by the network to know when to relay or when the request for the data of one needs to be executed.
     * 
     * NOTE: This will most likely be deprecated in Tritium ++. It is only here to allow simple integration with
     * the older protocol versions to allow the backwards communication on older protocol versions.
     */
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
		
		
		/* Check based on CInv. */
		bool Has(LLP::CInv cInv);
        
        
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
	
    
	/** Node Manager Class:
     * 
     * This is resonsilbe for the managing of all the nodes in the Tritum Protoco.
     * It is responsible for overseeing all the connections, processing blocks, and handling the network wide relays.
     * 
     * This is necessary to keep all the main processing for a node here in this specifric class so that the other services a node can provide be easy to integrate and extend.
     * 
     * This is also where a node will be keeping track of the differences in the time seeking and also the intelligence of the trust that is seen indepent of any of the network wide trust.
     */
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
        
        
        /* Variables to set the state of the node manager regarding Tritium Protocol vs the 2.0 protocol previous. */
		bool fEstablishedTritium;
        
        /* Switch to tell if on the Tritium Time servers or the original 2.0 LLP Time server. */
        bool fUnifiedTimeTritium;
        
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
