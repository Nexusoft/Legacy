/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_CORE_INCLUDE_MANAGER_H
#define NEXUS_CORE_INCLUDE_MANAGER_H

#include "../../LLP/templates/server.h"
#include "../../LLP/include/network.h"
#include "../../LLP/include/node.h"

#include "../network/include/txpool.h"
#include "../network/include/blkpool.h"

namespace Core
{

	/** Node Manager Class:
	 * 
	 * This is resonsilbe for the managing of all the nodes in the Tritum Protocol.
	 * It is responsible for overseeing all the connections, processing blocks, and handling the network wide relays.
	 * 
	 * This is necessary to keep all the main processing for a node here in this specifric class so that the other services a node can provide be easy to integrate and extend.
	 * 
	 * This is also where a node will be keeping track of the differences in the time seeking and also the intelligence of the trust that is seen indepent of any of the network wide trust.
	 * 
	 * TODO: 
	 */
	class Manager : public LLP::Server<LLP::CNode>
	{
		
		Thread_t ConnectionThread;
		Thread_t ProcessorThread;
		
	public:
		
		Manager() : LLP::Server<LLP::CNode> (LLP::GetDefaultPort(), 10, false, 1, 20, 30, 30, true, true), txPool(), blkPool(), ConnectionThread(boost::bind(&Manager::ConnectionManager, this)),  ProcessorThread(boost::bind(&Manager::BlockProcessor, this)) {}
		
		
		/* Time Seed Manager. */
		void TimestampManager();
		
		
		/* Connection Manager Thread. */
		void ConnectionManager();
		
		
		/* Handle and Process New Blocks. */
		void BlockProcessor();
		
		
		/* Add address to the Queue. */
		void AddAddress(LLP::CAddress cAddress);
		
		
		/* Get a random address from the active connections in the manager. */
		LLP::CAddress GetRandAddress(bool fNew = false);
		
		
		/* Start up the Node Manager. */
		void Start();
		
		
		/* Transaction Holding Pool. */
		CTxPool txPool;
		
		
		/* Block Holding Pool. */
		CBlkPool blkPool;
		
		
	private:

		
		/* Tried Address in the Manager. */
		std::vector<LLP::CAddress> vTried;
		
		
		/* New Addresses in the Manager. */
		std::vector<LLP::CAddress> vNew;
		
	};
	
	extern Manager* pManager;
	
	
	
	/* Sorting Algorithm for Blocks. */
	bool SortByHeight(const uint1024& nFirst, const uint1024& nSecond);
	
}

#endif
