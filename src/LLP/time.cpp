/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/time.h"

#include "../Core/include/unifiedtime.h"

namespace LLP
{
	
	/* Handle Event Inheritance. */
	void TimeLLP::Event(unsigned char EVENT, unsigned int LENGTH)
	{
		
		/* Handle any DDOS Packet Filters. */
		if(EVENT == EVENT_HEADER)
		{
			if(fDDOS)
			{
				if(INCOMING.HEADER == TIME_DATA)
					DDOS->Ban();
					
				if(INCOMING.HEADER == ADDRESS_DATA)
					DDOS->Ban();
					
				if(INCOMING.HEADER == TIME_OFFSET)
					DDOS->Ban();
					
				if(INCOMING.HEADER == GET_OFFSET && INCOMING.LENGTH > 4)
					DDOS->Ban();
					
				if(DDOS->Banned())
					return;
					
			}
		}
			
		/* Handle for a Packet Data Read. */
		if(EVENT == EVENT_PACKET)
			return;
			
		/* On Generic Event, Broadcast new block if flagged. */
		if(EVENT == EVENT_GENERIC)
			return;
			
		/* On Connect Event, Assign the Proper Daemon Handle. */
		if(EVENT == EVENT_CONNECT)
			return;
			
		/* On Disconnect Event, Reduce the Connection Count for Daemon */
		if(EVENT == EVENT_DISCONNECT)
			return;
		
	}
	
		
	/** This function is necessary for a template LLP server. It handles your 
		custom messaging system, and how to interpret it from raw packets. **/
	bool TimeLLP::ProcessPacket()
	{
		if(INCOMING.HEADER == GET_OFFSET)
		{
			unsigned int nTimestamp = bytes2uint(INCOMING.DATA);
			int   nOffset    = (int)(Core::UnifiedTimestamp() - nTimestamp);
				
			Packet RESPONSE;
			RESPONSE.HEADER = TIME_OFFSET;
			RESPONSE.LENGTH = 4;
			RESPONSE.DATA   = int2bytes(nOffset);
				
			this->WritePacket(RESPONSE);
			
			return true;
		}

		if(INCOMING.HEADER == GET_TIME)
		{
			Packet RESPONSE;
			RESPONSE.HEADER = TIME_DATA;
			RESPONSE.LENGTH = 4;
			RESPONSE.DATA = uint2bytes((unsigned int)Core::UnifiedTimestamp());
				
			this->WritePacket(RESPONSE);
			
			return true;
		}
			
		return false;
	}
}



