/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/block.h"
#include "include/difficulty.h"
#include "include/supply.h"
#include "include/prime.h"

using namespace std;

/* Diffiuculty is Linear
 * Difficulty 0.000016 per Second 0.133239 Total 1227 [0.737034 %]
 * 
 * Difficulty 0.000534 Total 122 [0.118752 %]
 * Difficulty 0.000507 Total 130 [0.126539 %]
 * Difficulty 0.000482 Total 150 [0.146007 %]
 * Difficulty 0.000457 Total 147 [0.143087 %]
 * Difficulty 0.000435 Total 155 [0.150874 %]
 * Difficulty 0.000413 Total 174 [0.169368 %]
 * Difficulty 0.000392 Total 187 [0.182022 %
Difficulty 0.000373 Total 192 [0.186889 %]
Difficulty 0.000354 Total 217 [0.211223 %]
Difficulty 0.000336 Total 179 [0.174235 %]
Difficulty 0.000319 Total 197 [0.191755 %]
Difficulty 0.000303 Total 241 [0.234584 %]
Difficulty 0.000288 Total 237 [0.230691 %]
Difficulty 0.000274 Total 256 [0.249185 %]
Difficulty 0.000260 Total 272 [0.264759 %]
Difficulty 0.000247 Total 286 [0.278386 %]
Difficulty 0.000235 Total 296 [0.288120 %]
Difficulty 0.000223 Total 295 [0.287147 %]
Difficulty 0.000212 Total 317 [0.308561 %]
Difficulty 0.000201 Total 338 [0.329002 %]
Difficulty 0.000191 Total 367 [0.357230 %]
Difficulty 0.000182 Total 396 [0.385458 %]
Difficulty 0.000173 Total 398 [0.387404 %]
Difficulty 0.000164 Total 394 [0.383511 %]
Difficulty 0.000156 Total 462 [0.449701 %]
Difficulty 0.000148 Total 448 [0.436073 %]
Difficulty 0.000141 Total 474 [0.461381 %]
Difficulty 0.000134 Total 505 [0.491556 %]
Difficulty 0.000127 Total 530 [0.515890 %]
Difficulty 0.000121 Total 539 [0.524651 %]
Difficulty 0.000115 Total 569 [0.553852 %]
Difficulty 0.000109 Total 621 [0.604468 %]
Difficulty 0.000103 Total 624 [0.607388 %]
Difficulty 0.000098 Total 694 [0.675524 %]
Difficulty 0.000093 Total 765 [0.744634 %]
Difficulty 0.000089 Total 805 [0.783569 %]
Difficulty 0.000084 Total 777 [0.756315 %]
Difficulty 0.000080 Total 844 [0.821531 %]
Difficulty 0.000076 Total 881 [0.857546 %]
Difficulty 0.000072 Total 944 [0.918869 %]
Difficulty 0.000069 Total 1018 [0.990899 %]
Difficulty 0.000065 Total 1065 [1.036648 %]
Difficulty 0.000062 Total 1120 [1.090183 %]
Difficulty 0.000059 Total 1159 [1.128145 %]
Difficulty 0.000056 Total 1169 [1.137879 %]
Difficulty 0.000053 Total 1316 [1.280966 %]
Difficulty 0.000050 Total 1372 [1.335475 %]
Difficulty 0.000048 Total 1445 [1.406531 %]
Difficulty 0.000045 Total 1528 [1.487322 %]
Difficulty 0.000043 Total 1546 [1.504843 %]
Difficulty 0.000041 Total 1653 [1.608994 %]
Difficulty 0.000039 Total 1712 [1.666423 %]
Difficulty 0.000037 Total 1879 [1.828977 %]
Difficulty 0.000035 Total 1876 [1.826057 %]
Difficulty 0.000033 Total 2094 [2.038254 %]
Difficulty 0.000032 Total 2177 [2.119044 %]
Difficulty 0.000030 Total 2254 [2.193994 %]
Difficulty 0.000029 Total 2422 [2.357522 %]
Difficulty 0.000027 Total 2577 [2.508395 %]
Difficulty 0.000026 Total 2603 [2.533703 %]
Difficulty 0.000025 Total 2731 [2.658296 %]
Difficulty 0.000023 Total 2972 [2.892880 %]
Difficulty 0.000022 Total 2937 [2.858812 %]
Difficulty 0.000021 Total 3193 [3.107996 %]
Difficulty 0.000020 Total 3404 [3.313379 %]
Difficulty 0.000019 Total 3563 [3.468146 %]
Difficulty 0.000018 Total 3803 [3.701757 %]
Difficulty 0.000017 Total 4046 [3.938288 %]
Difficulty 0.000016 Total 4212 [4.099869 %]

*/

namespace Core
{
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
	void GetChainTimes(unsigned int nAge, unsigned int& nDays, unsigned int& nHours, unsigned int& nMinutes)
	{
		nDays = nAge / 1440;
		nHours = (nAge - (nDays * 1440)) / 60;
		nMinutes = nAge % 60;
	}
	
	
	/* Minimum work required after nTime from last checkpoint
		Used to compare blocks difficulty to a minimum probable difficulty after nTime */
	unsigned int ComputeMinWork(const CBlockIndex* pcheckpoint, unsigned int nTime, int nChannel)
	{
	
		//TODO: Precise Calculation on Maximum Decrease of Difficulty from Version 3 +
		
		return 0;
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
	
	
	/* Trust Retargeting: Modulate Difficulty based on production rate. */
	unsigned int RetargetTrust(const CBlockIndex* pindex, bool output)
	{

		/* Get Last Block Index [1st block back in Channel]. **/
		const CBlockIndex* pindexFirst = GetLastChannelIndex(pindex, 0);
		if (pindexFirst->pprev == NULL)
			return bnProofOfWorkStart[0].GetCompact();
			
			
		/* Get Last Block Index [2nd block back in Channel]. */
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, 0);
		if (pindexLast->pprev == NULL)
			return bnProofOfWorkStart[0].GetCompact();
		
		
		/* Get the Block Time and Target Spacing. */
		int64 nBlockTime = GetWeightedTimes(pindexFirst, 5);
		int64 nBlockTarget = STAKE_TARGET_SPACING;
		
		
		/* The Upper and Lower Bound Adjusters. */
		int64 nUpperBound = nBlockTarget;
		int64 nLowerBound = nBlockTarget;
		
			
		/* If the time is above target, reduce difficulty by modular
			of one interval past timespan multiplied by maximum decrease. */
		if(nBlockTime >= nBlockTarget)
		{
			/* Take the Minimum overlap of Target Timespan to make that maximum interval. */
			uint64 nOverlap = (uint64)min((nBlockTime - nBlockTarget), (nBlockTarget * 2));
				
			/* Get the Mod from the Proportion of Overlap in one Interval. */
			double nProportions = (double)nOverlap / (nBlockTarget * 2);
				
			/* Get Mod from Maximum Decrease Equation with Decimal portions multiplied by Propotions. */
			double nMod = 1.0 - (0.15 * nProportions);
			nLowerBound = nBlockTarget * nMod;
		}
			
			
		/* If the time is below target, increase difficulty by modular
			of interval of 1 - Block Target with time of 1 being maximum increase */
		else
		{
			/* Get the overlap in reference from Target Timespan. */
			uint64 nOverlap = nBlockTarget - nBlockTime;
				
			/* Get the mod from overlap proportion. Time of 1 will be closest to mod of 1. */
			double nProportions = (double) nOverlap / nBlockTarget;
				
			/* Get the Mod from the Maximum Increase Equation with Decimal portion multiplied by Proportions. */
			double nMod = 1.0 + (nProportions * 0.075);
			nLowerBound = nBlockTarget * nMod;
		}
			
			
		/* Get the Difficulty Stored in Bignum Compact. */
		CBigNum bnNew;
		bnNew.SetCompact(pindexFirst->nBits);
		
		
		/* Change Number from Upper and Lower Bounds. */
		bnNew *= nUpperBound;
		bnNew /= nLowerBound;
		
		
		/* Don't allow Difficulty to decrease below minimum. */
		if (bnNew > bnProofOfWorkLimit[0])
			bnNew = bnProofOfWorkLimit[0];
			
			
		/* Verbose Debug Output. */
		if(GetArg("-verbose", 0) >= 1 && output)
		{
			unsigned int nDays, nHours, nMinutes;
			GetChainTimes(GetChainAge(pindexFirst->GetBlockTime()), nDays, nHours, nMinutes);
			
			printf("RETARGET [TRUST] weighted time=%" PRId64 " actual time =%" PRId64 "[%f %%]\n\tchain time: [%" PRId64 " / %" PRId64 "]\n\tdifficulty: [%f to %f]\n\ttrust height: %" PRId64 " [AGE %u days, %u hours, %u minutes]\n\n", 
			nBlockTime, max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1), ((100.0 * nLowerBound) / nUpperBound), nBlockTarget, nBlockTime, GetDifficulty(pindexFirst->nBits, 0), GetDifficulty(bnNew.GetCompact(), 0), pindexFirst->nChannelHeight, nDays, nHours, nMinutes);
		}
		
		return bnNew.GetCompact();
	}
	
	
	
	/* Prime Retargeting: Modulate Difficulty based on production rate. */
	unsigned int RetargetPrime(const CBlockIndex* pindex, bool output)
	{
		
		/* Get Last Block Index [1st block back in Channel]. */
		const CBlockIndex* pindexFirst = GetLastChannelIndex(pindex, 1);
		if (!pindexFirst->pprev)
			return bnProofOfWorkStart[1].getuint();
			
		
		/* Get Last Block Index [2nd block back in Channel]. */
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, 1);
		if (!pindexLast->pprev)
			return bnProofOfWorkStart[1].getuint();
		
		
		/* Standard Time Proportions */
		int64 nBlockTime = ((pindex->nVersion >= 4) ? GetWeightedTimes(pindexFirst, 5) : max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1));
		int64 nBlockTarget = nTargetTimespan;
		
		
		/* Chain Mod: Is a proportion to reflect outstanding released funds. Version 1 Deflates difficulty slightly
			to allow more blocks through when blockchain has been slow, Version 2 Deflates Target Timespan to lower the minimum difficulty. 
			This helps stimulate transaction processing while helping get the Nexus production back on track */
		double nChainMod = GetFractionalSubsidy(GetChainAge(pindexFirst->GetBlockTime()), 0, ((pindex->nVersion >= 3) ? 40.0 : 20.0)) / (pindexFirst->nReleasedReserve[0] + 1);
		nChainMod = min(nChainMod, 1.0);
		nChainMod = max(nChainMod, (pindex->nVersion == 1) ? 0.75 : 0.5);
		
		
		/* Enforce Block Version 2 Rule. Chain mod changes block time requirements, not actual mod after block times. */
		if(pindex->nVersion >= 2)
			nBlockTarget *= nChainMod;
		
		
		/* These figures reduce the increase and decrease max and mins as difficulty rises
			this is due to the time difference between each cluster size [ex. 1, 2, 3] being 50x */
		double nDifficulty = GetDifficulty(pindexFirst->nBits, 1);
		
		
		/* The Mod to Change Difficulty. */
		double nMod = 1.0;
		
		
		/* Handle for Version 3 Blocks. Mod determined by time multiplied by max / min. */
		if(pindex->nVersion >= 3)
		{

			/* If the time is above target, reduce difficulty by modular
				of one interval past timespan multiplied by maximum decrease. */
			if(nBlockTime >= nBlockTarget)
			{
				/* Take the Minimum overlap of Target Timespan to make that maximum interval. */
				uint64 nOverlap = (uint64)min((nBlockTime - nBlockTarget), (nBlockTarget * 2));
				
				/* Get the Mod from the Proportion of Overlap in one Interval. */
				double nProportions = (double)nOverlap / (nBlockTarget * 2);
				
				/* Get Mod from Maximum Decrease Equation with Decimal portions multiplied by Propotions. */
				nMod = 1.0 - (nProportions * (0.5 / ((nDifficulty - 1) * 5.0)));
			}
			
			/* If the time is below target, increase difficulty by modular
				of interval of 1 - Block Target with time of 1 being maximum increase */
			else
			{
				/* Get the overlap in reference from Target Timespan. */
				uint64 nOverlap = nBlockTarget - nBlockTime;
				
				/* Get the mod from overlap proportion. Time of 1 will be closest to mod of 1. */
				double nProportions = (double) nOverlap / nBlockTarget;
				
				/* Get the Mod from the Maximum Increase Equation with Decimal portion multiplied by Proportions. */
				nMod = 1.0 + (nProportions * (0.125 / ((nDifficulty - 1) * 10.0)));
			}
			
		}
		
		/* Handle for Block Version 2 Difficulty Adjustments. */
		else
		{
			/* Equations to Determine the Maximum Increase / Decrease. */
			double nMaxDown = 1.0 - (0.5 / ((nDifficulty - 1) * ((pindex->nVersion == 1) ? 10.0 : 25.0)));
			double nMaxUp = (0.125 / ((nDifficulty - 1) * 50.0)) + 1.0;
			
			/* Block Modular Determined from Time Proportions. */
			double nBlockMod = (double) nBlockTarget / nBlockTime; 
			nBlockMod = min(nBlockMod, 1.125);
			nBlockMod = max(nBlockMod, 0.50);
			
			/* Version 1 Block, Chain Modular Modifies Block Modular. **/
			nMod = nBlockMod;
			if(pindex->nVersion == 1)
				nMod *= nChainMod;
				
			/* Set Modular to Max / Min values. */
			nMod = min(nMod, nMaxUp);
			nMod = max(nMod, nMaxDown);
		}
		
		
		/* If there is a change in difficulty, multiply by mod. */
		nDifficulty *= nMod;

		
		/* Set the Bits of the Next Difficulty. */
		unsigned int nBits = SetBits(nDifficulty);
		if(nBits < bnProofOfWorkLimit[1].getuint())
			nBits = bnProofOfWorkLimit[1].getuint();
			
		/* Console Output */
		if(GetArg("-verbose", 0) >= 1 && output)
		{
			unsigned int nDays, nHours, nMinutes;
			GetChainTimes(GetChainAge(pindexFirst->GetBlockTime()), nDays, nHours, nMinutes);
			
			printf("RETARGET [PRIME] weighted time=%" PRId64 " actual time %" PRId64 ", [%f %%]\n\tchain time: [%" PRId64 " / %" PRId64 "]\n\treleased reward: %" PRId64 " [%f %%]\n\tdifficulty: [%f to %f]\n\tprime height: %" PRId64 " [AGE %u days, %u hours, %u minutes]\n\n", 
			nBlockTime, max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1), nMod * 100.0, nBlockTarget, nBlockTime, pindexFirst->nReleasedReserve[0] / COIN, 100.0 * nChainMod, GetDifficulty(pindexFirst->nBits, 1), GetDifficulty(nBits, 1), pindexFirst->nChannelHeight, nDays, nHours, nMinutes);
		}
		
		
		return nBits;
	}

	
	
	/* Trust Retargeting: Modulate Difficulty based on production rate. */
	unsigned int RetargetHash(const CBlockIndex* pindex, bool output)
	{
	
		/* Get the Last Block Index [1st block back in Channel]. */
		const CBlockIndex* pindexFirst = GetLastChannelIndex(pindex, 2);
		if (pindexFirst->pprev == NULL)
			return bnProofOfWorkStart[2].GetCompact();
			
			
		/* Get Last Block Index [2nd block back in Channel]. */
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, 2);
		if (pindexLast->pprev == NULL)
			return bnProofOfWorkStart[2].GetCompact();

			
		/* Get the Block Times with Minimum of 1 to Prevent Time Warps. */
		int64 nBlockTime = ((pindex->nVersion >= 4) ? GetWeightedTimes(pindexFirst, 5) : max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1));
		int64 nBlockTarget = nTargetTimespan;
		
		
		/* Get the Chain Modular from Reserves. */
		double nChainMod = GetFractionalSubsidy(GetChainAge(pindexFirst->GetBlockTime()), 0, ((pindex->nVersion >= 3) ? 40.0 : 20.0)) / (pindexFirst->nReleasedReserve[0] + 1);
		nChainMod = min(nChainMod, 1.0);
		nChainMod = max(nChainMod, (pindex->nVersion == 1) ? 0.75 : 0.5);
		
		
		/* Enforce Block Version 2 Rule. Chain mod changes block time requirements, not actual mod after block times. */
		if(pindex->nVersion >= 2)
			nBlockTarget *= nChainMod;
			
			
		/* The Upper and Lower Bound Adjusters. */
		int64 nUpperBound = nBlockTarget;
		int64 nLowerBound = nBlockTarget;
		
			
		/* Handle for Version 3 Blocks. Mod determined by time multiplied by max / min. */
		if(pindex->nVersion >= 3)
		{

			/* If the time is above target, reduce difficulty by modular
				of one interval past timespan multiplied by maximum decrease. */
			if(nBlockTime >= nBlockTarget)
			{
				/* Take the Minimum overlap of Target Timespan to make that maximum interval. */
				uint64 nOverlap = (uint64)min((nBlockTime - nBlockTarget), (nBlockTarget * 2));
				
				/* Get the Mod from the Proportion of Overlap in one Interval. */
				double nProportions = (double)nOverlap / (nBlockTarget * 2);
				
				/* Get Mod from Maximum Decrease Equation with Decimal portions multiplied by Propotions. */
				double nMod = 1.0 - (((pindex->nVersion >= 4) ? 0.15 : 0.75) * nProportions);
				nLowerBound = nBlockTarget * nMod;
			}
			
			/* If the time is below target, increase difficulty by modular
				of interval of 1 - Block Target with time of 1 being maximum increase */
			else
			{
				/* Get the overlap in reference from Target Timespan. */
				uint64 nOverlap = nBlockTarget - nBlockTime;
				
				/* Get the mod from overlap proportion. Time of 1 will be closest to mod of 1. */
				double nProportions = (double) nOverlap / nBlockTarget;
				
				/* Get the Mod from the Maximum Increase Equation with Decimal portion multiplied by Proportions. */
				double nMod = 1.0 + (nProportions * 0.075);
				nLowerBound = nBlockTarget * nMod;
			}
		}
		
		
		/* Handle for Version 2 Difficulty Adjustments. */
		else
		{
			double nBlockMod = (double) nBlockTarget / nBlockTime; 
			nBlockMod = min(nBlockMod, 1.125);
			nBlockMod = max(nBlockMod, 0.75);
			
			/* Calculate the Lower Bounds. */
			nLowerBound = nBlockTarget * nBlockMod;
			
			/* Version 1 Blocks Change Lower Bound from Chain Modular. */
			if(pindex->nVersion == 1)
				nLowerBound *= nChainMod;
			
			/* Set Maximum [difficulty] up to 8%, and Minimum [difficulty] down to 50% */
			nLowerBound = min(nLowerBound, (int64)(nUpperBound + (nUpperBound / 8)));
			nLowerBound = max(nLowerBound, (3 * nUpperBound ) / 4);
		}
			
			
		/* Get the Difficulty Stored in Bignum Compact. */
		CBigNum bnNew;
		bnNew.SetCompact(pindexFirst->nBits);
		
		
		/* Change Number from Upper and Lower Bounds. */
		bnNew *= nUpperBound;
		bnNew /= nLowerBound;
		
		
		/* Don't allow Difficulty to decrease below minimum. */
		if (bnNew > bnProofOfWorkLimit[2])
			bnNew = bnProofOfWorkLimit[2];
			
			
		/* Console Output if Flagged. */
		if(GetArg("-verbose", 0) >= 1 && output)
		{
			unsigned int nDays, nHours, nMinutes;
			GetChainTimes(GetChainAge(pindexFirst->GetBlockTime()), nDays, nHours, nMinutes);
			
			printf("RETARGET [HASH] weighted time=%" PRId64 " actual time %" PRId64 " [%f %%]\n\tchain time: [%" PRId64 " / %" PRId64 "]\n\treleased reward: %" PRId64 " [%f %%]\n\tdifficulty: [%f to %f]\n\thash height: %" PRId64 " [AGE %u days, %u hours, %u minutes]\n\n", 
			nBlockTime, max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1), (100.0 * nLowerBound) / nUpperBound, nBlockTarget, nBlockTime, pindexFirst->nReleasedReserve[0] / COIN, 100.0 * nChainMod, GetDifficulty(pindexFirst->nBits, 2), GetDifficulty(bnNew.GetCompact(), 2), pindexFirst->nChannelHeight, nDays, nHours, nMinutes);
		}
		
		return bnNew.GetCompact();
	}

}
