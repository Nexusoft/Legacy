#ifndef Nexus_CORE_SERVER_H
#define Nexus_CORE_SERVER_H

#include "server.h"
#include "../core/unifiedtime.h"
#include "../util/util.h"

namespace LLP
{
	class CoreLLP : public Connection
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
		CoreLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN ) : Connection( SOCKET_IN, DDOS_IN ) 
		{ ADDRESS = parse_ip(SOCKET_IN->remote_endpoint().address().to_string()); }
		
		/** This function is necessary for a template LLP server. It handles your 
			custom messaging system, and how to interpret it from raw packets. **/
		inline bool ProcessPacket()
		{
			Packet PACKET   = this->INCOMING;
			
			if(PACKET.HEADER == GET_OFFSET)
			{
				unsigned int nTimestamp = bytes2uint(PACKET.DATA);
				int   nOffset    = (int)(GetUnifiedTimestamp() - nTimestamp);
				
				Packet RESPONSE;
				RESPONSE.HEADER = TIME_OFFSET;
				RESPONSE.LENGTH = 4;
				RESPONSE.DATA   = int2bytes(nOffset);
				
				printf("***** Core LLP: Sent Offset %i | %u.%u.%u.%u | Unified %I64d\n", nOffset, ADDRESS[0], ADDRESS[1], ADDRESS[2], ADDRESS[3], GetUnifiedTimestamp());
				this->WritePacket(RESPONSE);
				return true;
			}

			if(PACKET.HEADER == GET_TIME)
			{
				Packet RESPONSE;
				RESPONSE.HEADER = TIME_DATA;
				RESPONSE.LENGTH = 4;
				RESPONSE.DATA = uint2bytes((unsigned int)GetUnifiedTimestamp());
				printf("***** Core LLP: Sent Time Sample %I64d to %u.%u.%u.%u\n", GetUnifiedTimestamp(), ADDRESS[0], ADDRESS[1], ADDRESS[2], ADDRESS[3]);
				
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