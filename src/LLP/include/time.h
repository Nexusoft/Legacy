/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_LLP_INCLUDE_TIME_H
#define NEXUS_LLP_INCLUDE_TIME_H

#include "../templates/types.h"
#include "../Util/include/convert.h"


namespace LLP
{	
	
	/* Old Core Server. TODO: To Be Removed in Tritium ++ */
	class TimeLLP : public Connection
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
		TimeLLP() : Connection(){ }
		TimeLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : Connection( SOCKET_IN, DDOS_IN ) {}
		
		
		/* Handle Event Inheritance. */
		void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		
		
		/** This function is necessary for a template LLP server. It handles your 
			custom messaging system, and how to interpret it from raw packets. **/
		bool ProcessPacket();
	};
	
	
	/* Class to be Deprecated post Tritium.
	 * 
	 * Used for old unified time system.
	 * 
	 */
	class TimeOutbound : public Connection
	{
	public:
		Service_t IO_SERVICE;
		
		TimeOutbound() : Connection() {}
		
		bool ProcessPacket() { return true; }
		void Event(unsigned char EVENT, unsigned int LENGTH = 0) {}
		
		enum
		{
			/** DATA PACKETS **/
			TIME_DATA     = 0,
			ADDRESS_DATA  = 1,
			TIME_OFFSET   = 2,
			
			/** DATA REQUESTS **/
			GET_OFFSET    = 64,
			
			
			/** REQUEST PACKETS **/
			GET_TIME      = 129,
			GET_ADDRESS   = 130,
					

			/** GENERIC **/
			PING          = 253,
			CLOSE         = 254
		};
		
		inline Packet NewPacket() { return this->INCOMING; }
		
		inline Packet GetPacket(unsigned char HEADER)
		{
			Packet PACKET;
			PACKET.HEADER = HEADER;
			return PACKET;
		}
		
		inline void GetOffset(unsigned int nTimestamp)
		{
			Packet REQUEST = GetPacket(GET_OFFSET);
			REQUEST.LENGTH = 4;
			REQUEST.DATA   = uint2bytes(nTimestamp);
			
			this->WritePacket(REQUEST);
		}
		
		inline void GetTime()
		{
			Packet REQUEST = GetPacket(GET_TIME);
			this->WritePacket(REQUEST);
		}
		
		void Close()
		{
			Packet RESPONSE = GetPacket(CLOSE);
			this->WritePacket(RESPONSE);
			this->Disconnect();
		}
		
		inline void GetAddress()
		{
			Packet REQUEST = GetPacket(GET_ADDRESS);
			this->WritePacket(REQUEST);
		}
	};
}


#endif
