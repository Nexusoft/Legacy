/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/global.h"
#include "include/unifiedtime.h"

#include "../LLU/include/runtime.h"

namespace Core
{
	
	/* Flag Declarations */
	bool fTimeUnified = false;

	
	/* The Average Offset of all Nodes. */
	int UNIFIED_AVERAGE_OFFSET = 0;
	
	
	/* The Current Record Iterating for Moving Average. */
	int UNIFIED_MOVING_ITERATOR = 0;

	
	/* Unified Time Declarations */
	std::vector<int> UNIFIED_TIME_DATA;
	
	/** Gets the UNIX Timestamp from the Nexus Network **/
	uint64 UnifiedTimestamp(bool fMilliseconds){ return Timestamp(fMilliseconds) + (UNIFIED_AVERAGE_OFFSET / (fMilliseconds ? 1 : 1000)); }


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
}



