/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/unifiedtime.h"
#include "../LLU/include/runtime.h"

namespace LLP
{
	
	/* Flag Declarations */
	bool fTimeUnified = false;

	
	/* The Average Offset of all Nodes. */
	int UNIFIED_AVERAGE_OFFSET = 0;
	
	
	/* The Current Record Iterating for Moving Average. */
	int UNIFIED_MOVING_ITERATOR = 0;

	
	/* Unified Time Declarations */
	std::vector<int> UNIFIED_TIME_DATA;

	
	/* Maximum number of seconds that a clock can be from one another. */
	int MAX_UNIFIED_DRIFT		= 1;

	
	/* Maximum numver of samples in the Unified Time moving Average. */
	int MAX_UNIFIED_SAMPLES		= 200;
	
	
	/* Maximum numver of samples in the Unified Time moving Average. */
	int MAX_PER_NODE_SAMPLES	= 20;

	
	/** Gets the UNIX Timestamp from the Nexus Network **/
	int64 GetUnifiedTimestamp(bool fMilliseconds){ return Timestamp(fMilliseconds) + (UNIFIED_AVERAGE_OFFSET / (fMilliseconds ? 1 : 1000)); }


	/* Calculate the Average Unified Time. Called after Time Data is Added */
	int GetUnifiedAverage()
	{
		if(UNIFIED_TIME_DATA.empty())
			return UNIFIED_AVERAGE_OFFSET;
			
		int nAverage = 0;
		for(int index = 0; index < std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()); index++)
			nAverage += UNIFIED_TIME_DATA[index];
			
		return std::round(nAverage / (double) std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()));
	}
	
	
	/* Handle Event Inheritance. */
	void CoreLLP::Event(unsigned char EVENT, unsigned int LENGTH)
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
	bool CoreLLP::ProcessPacket()
	{
		if(INCOMING.HEADER == GET_OFFSET)
		{
			unsigned int nTimestamp = bytes2uint(INCOMING.DATA);
			int   nOffset    = (int)(LLP::GetUnifiedTimestamp() - nTimestamp);
				
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
			RESPONSE.DATA = uint2bytes((unsigned int)LLP::GetUnifiedTimestamp());
				
			this->WritePacket(RESPONSE);
			
			return true;
		}
			
		return false;
	}
}



