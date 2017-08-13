/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_INVENTORY_H
#define NEXUS_CORE_INCLUDE_INVENTORY_H

#include "../../LLP/include/node.h"
#include "../../LLP/include/inv.h"
#include "../../Util/include/mutex.h"

#include <boost/thread/thread.hpp>

namespace Core
{	
	
	template<typename T1>
	void RelayMessage(LLP::CInv inv, const T1& obj)
	{
		//TODO: connect with Node Manager / LLP Relay Layer
		//TODO: this should be in its own relay layer as relay.h / relay.cpp in the LLP namespace
	
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
	class Inventory
	{
	public:
	
		/* State level messages to hold information about all inventory. */
		enum
		{
			//Basic Checks
			UNVERIFIED = 0,
			ACCEPTED   = 1,
			ORPHANED   = 2,
			
			ON_CHAIN   = 3,
			MEMORY     = 4,
			DISK       = 5,
			
			
			//TODO: Leave room for more invalid states
			INVALID    = 128
			
		};
		
		Inventory() {}
		
		
		/* Class Mutex. */
		Mutex_t MUTEX;
		
		
		/* Check for Tranasction. */
		bool Has(uint512 hashTX) { return mapTransactions.count(hashTX); }
		
		
		/* Check for Blocks. */
		bool Has(uint1024 hashBlock) { return mapBlocks.count(hashBlock); }
		
		
		/* Check based on CInv. */
		bool Has(LLP::CInv cInv)
		{
			if(cInv.type == LLP::MSG_TX)
				return Has(cInv.hash.getuint512());
		
			return Has(cInv.hash);
		}
		
		
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
}

#endif
