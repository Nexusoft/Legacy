/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_TIME_H
#define NEXUS_LLP_INCLUDE_TIME_H

#include "../templates/types.h"

namespace LLP
{	
	
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
