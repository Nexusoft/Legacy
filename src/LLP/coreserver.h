/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_LLP_H
#define NEXUS_CORE_LLP_H

#include "server.h"
#include "../core/unifiedtime.h"
#include "../util/util.h"
#include <inttypes.h>

namespace LLP
{

	class CoreOutbound : public Outbound
	{
		Service_t IO_SERVICE;
		
	public:
		CoreOutbound(std::string ip, std::string port) : Outbound(ip, port){}
		
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
	
	class CoreLLP : public Connection<>
	{	
		std::vector<unsigned char> ADDRESS;
		
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
	
	public:
		CoreLLP() : Connection(){ }
		CoreLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : Connection( SOCKET_IN, DDOS_IN ) 
		{ ADDRESS = parse_ip(SOCKET_IN->remote_endpoint().address().to_string()); }
		
		/** Handle Event Inheritance. **/
		void Event(unsigned char EVENT, unsigned int LENGTH = 0)
		{
			/** Handle any DDOS Packet Filters. **/
			if(EVENT == EVENT_HEADER)
			{
				if(fDDOS)
				{
					Packet PACKET   = this->INCOMING;
					
					if(PACKET.HEADER == TIME_DATA)
						DDOS->Ban();
					
					if(PACKET.HEADER == ADDRESS_DATA)
						DDOS->Ban();
					
					if(PACKET.HEADER == TIME_OFFSET)
						DDOS->Ban();
					
					if(PACKET.HEADER == GET_OFFSET && PACKET.LENGTH > 4)
						DDOS->Ban();
					
					if(DDOS->Banned())
						return;
					
				}
			}
			
			
			/** Handle for a Packet Data Read. **/
			if(EVENT == EVENT_PACKET)
				return;
			
			
			/** On Generic Event, Broadcast new block if flagged. **/
			if(EVENT == EVENT_GENERIC)
				return;
			
			/** On Connect Event, Assign the Proper Daemon Handle. **/
			if(EVENT == EVENT_CONNECT)
				return;
			
			/** On Disconnect Event, Reduce the Connection Count for Daemon **/
			if(EVENT == EVENT_DISCONNECT)
				return;
		
		}
		
		/** This function is necessary for a template LLP server. It handles your 
			custom messaging system, and how to interpret it from raw packets. **/
		bool ProcessPacket()
		{
			Packet PACKET   = this->INCOMING;
			
			//TODO: Calculate the latency of the request and put that into the time equation
			if(PACKET.HEADER == GET_OFFSET)
			{
				unsigned int nTimestamp = bytes2uint(PACKET.DATA);
				int   nOffset    = (int)(GetUnifiedTimestamp() - nTimestamp);
				
				Packet RESPONSE;
				RESPONSE.HEADER = TIME_OFFSET;
				RESPONSE.LENGTH = 4;
				RESPONSE.DATA   = int2bytes(nOffset);
				
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Core LLP: Sent Offset %i | %u.%u.%u.%u | Unified %"PRId64"\n", nOffset, ADDRESS[0], ADDRESS[1], ADDRESS[2], ADDRESS[3], GetUnifiedTimestamp());
				
				this->WritePacket(RESPONSE);
				return true;
			}

			if(PACKET.HEADER == GET_TIME)
			{
				Packet RESPONSE;
				RESPONSE.HEADER = TIME_DATA;
				RESPONSE.LENGTH = 4;
				RESPONSE.DATA = uint2bytes((unsigned int)GetUnifiedTimestamp());
				
				if(GetArg("-verbose", 0) >= 3)
					printf("***** Core LLP: Sent Time Sample %"PRId64" to %u.%u.%u.%u\n", GetUnifiedTimestamp(), ADDRESS[0], ADDRESS[1], ADDRESS[2], ADDRESS[3]);
				
				this->WritePacket(RESPONSE);
				return true;
			}
			
			if(PACKET.HEADER == GET_ADDRESS)
			{
				Packet RESPONSE;
				RESPONSE.HEADER = ADDRESS_DATA;
				RESPONSE.LENGTH = 4;
				RESPONSE.DATA   = ADDRESS;
				
				this->WritePacket(RESPONSE);
				return true;
			}
			
			return false;
		}
	};
}

#endif
