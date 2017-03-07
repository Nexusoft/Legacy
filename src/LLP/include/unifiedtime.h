/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_UNIFIEDTIME_H
#define NEXUS_LLP_INCLUDE_UNIFIEDTIME_H

#include <vector>
#include <time.h>

#include "network.h"
#include "../templates/types.h"

#include "../../LLU/include/args.h"
#include "../../LLU/include/convert.h"

#include <inttypes.h>

namespace LLP
{

	/* Unified Time Flags */
	extern bool fTimeUnified;


	/* Offset calculated from average Unified Offset collected over time. */
	extern int UNIFIED_AVERAGE_OFFSET;


	/* Vector to Contain list of Unified Time Offset from Time Seeds, Seed Nodes, and Peers. */
	extern std::vector<int> UNIFIED_TIME_DATA;


	/* The Maximum Seconds that a Clock can be Off. This is set to account
		for Network Propogation Times and Normal Hardware level clock drifting. */
	extern int MAX_UNIFIED_DRIFT;
	
	
	/* The Maximum Samples to be stored on a moving average for this node. */
	extern int MAX_UNIFIED_SAMPLES;
	
	
	/* The Maximum Samples to be stored on a moving average per node. */
	extern int MAX_PER_NODE_SAMPLES;


	/** Initializes the Unifed Time System. 
		A] Checks Database for Offset Average List
		B] Gets Periodic Average of 10 Seeds if first Unified Time **/
	void InitializeUnifiedTime();


	/** Gets the Current Unified Time Average. The More Data in Time Average, the less
		a pesky node with a manipulated time seed can influence. Keep in mind too that the
		Unified Time will only be updated upon your outgoing connection... otherwise anyone flooding
		network with false time seed will just be ignored. The average is a moving one, so that iter_swap
		will allow clocks that operate under different intervals to always stay synchronized with the network. **/
	int GetUnifiedAverage();


	/* Gets the UNIX Timestamp from the Nexus Network */
	int64 GetUnifiedTimestamp(bool fMilliseconds = false);
	
	
	/* Old Core Server. TODO: To Be Removed in Tritium ++ */
	class CoreLLP : public Connection
	{	
		enum
		{
			/* DATA PACKETS */
			TIME_DATA     = 0,
			ADDRESS_DATA  = 1,
			TIME_OFFSET   = 2,
			
			
			/* DATA REQUESTS */
			GET_OFFSET    = 64,
			
			
			/* REQUEST PACKETS */
			GET_TIME      = 129,
			GET_ADDRESS   = 130,
					

			/* GENERIC */
			PING          = 253,
			CLOSE         = 254
		};
	
	public:
		CoreLLP() : Connection(){ }
		CoreLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : Connection( SOCKET_IN, DDOS_IN ) {}
		
		
		/* Handle Event Inheritance. */
		void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		
		
		/** This function is necessary for a template LLP server. It handles your 
			custom messaging system, and how to interpret it from raw packets. **/
		bool ProcessPacket();
	};
}


#endif
