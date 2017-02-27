/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_DIFFICULTY_H
#define NEXUS_CORE_DIFFICULTY_H

#include "core.h"

using namespace std;

namespace Core
{
	
	/* Target Timespan of 300 Seconds. */
	static const unsigned int nTargetTimespan = 300;
	
	
	/* Determines the Decimal of nBits per Channel for a decent "Frame of Reference".
		Has no functionality in Network Operation. */
	double GetDifficulty(unsigned int nBits, int nChannel)
	{
		
		/* Prime Channel is just Decimal Held in Integer
			Multiplied and Divided by Significant Digits. */
		if(nChannel == 1)
			return nBits / 10000000.0;
		
		/* Get the Proportion of the Bits First. */
		double dDiff =
			(double)0x0000ffff / (double)(nBits & 0x00ffffff);
			
		/* Calculate where on Compact Scale Difficulty is. */
		int nShift = nBits >> 24;
		
		/* Shift down if Position on Compact Scale is above 124. */
		while(nShift > 124)
		{
			dDiff = dDiff / 256.0;
			nShift--;
		}
		
		/* Shift up if Position on Compact Scale is below 124. */
		while(nShift < 124)
		{
			dDiff = dDiff * 256.0;
			nShift++;
		}
		
		/* Offset the number by 64 to give larger starting reference. */
		return dDiff * ((nChannel == 2) ? 64 : 1024 * 1024 * 256);
	}
	
	
	/* Break the Chain Age in Minutes into Days, Hours, and Minutes. */
	void GetChainTimes(int64 nAge, int64& nDays, int64& nHours, int64& nMinutes)
	{
		nDays = nAge / 1440;
		nHours = (nAge - (nDays * 1440)) / 60;
		nMinutes = nAge % 60;
	}
	
	
	/* Minimum work required after nTime from last checkpoint
		Used to compare blocks difficulty to a minimum probable difficulty after nTime */
	unsigned int ComputeMinWork(const CBlockIndex* pcheckpoint, int64 nTime, int nChannel)
	{
	
		//TODO: Precise Calculation on Maximum Decrease of Difficulty from Version 3 +
		
	}
	
	
	/* Get Weighted Times functions to weight the average on an iterator to give more weight to the most recent blocks
		in the average to let previous block nDepth back still influence difficulty, but to let the most recent block
		have the most influence in the adjustment. */
	int64 GetWeightedTimes(const CBlockIndex* pindex, unsigned int nDepth)
	{
		int64 nWeightedAverage = 0;
		unsigned int nIterator = 0;
		
		const CBlockIndex* pindexFirst = pindex;
		for(int nIndex = nDepth; nIndex > 0; nIndex--)
		{
			const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, pindex->GetChannel());
			if(!pindexLast->pprev)
				break;
				
			int64 nTime = max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1) * nIndex * 3;
			pindexFirst = pindexLast;
			
			nIterator += (nIndex * 3);
			nWeightedAverage += nTime;
		}
			
		nWeightedAverage /= nIterator;
		
		return nWeightedAverage;
	}

	
	/* Switching function for each difficulty re-target [each channel uses their own version] */
	unsigned int GetNextTargetRequired(const CBlockIndex* pindex, int nChannel, bool output)
	{
		if(nChannel == 0)
			return RetargetTrust(pindex, output);
			
		else if(nChannel == 1)
			return RetargetPrime(pindex, output);
			
		else if(nChannel == 2)
			return RetargetHash(pindex, output);
		
		return 0;
	}
	
	
	/* Trust Channel Retargeting: Modulate Difficulty based on production rate. */
	unsigned int RetargetTrust(const CBlockIndex* pindex, bool output)
	
	
	/* Prime Channel Retargeting. Very different than GPU or POS retargeting. Scales the Maximum
		Increase / Decrease by Network Difficulty. This helps to keep increases more time based than
		mathematically based. This means that as the difficulty rises, the maximum up/down in difficulty
		will decrease keeping the time difference in difficulty jumps the same from diff 1 - 100. */
	unsigned int RetargetPrime(const CBlockIndex* pindex, bool output)

	
	/* Hash Channel Retargeting: Modulate Difficulty based on production rate. */
	unsigned int RetargetHash(const CBlockIndex* pindex, bool output);
}
