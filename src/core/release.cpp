/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "core.h"

using namespace std;
namespace Core
{
	/** These values reflect the Three Decay Equations.
    50 * e ^ (-0.0000011  * nMinutes) + 1.0
	10 * e ^ (-0.00000055 * nMinutes) + 1.0
	 1 * e ^ (-0.00000059 * nMinutes) + 0.032 **/
	double decay[3][3] = { {50.0, -0.0000011, 1.0}, {10.0, -0.00000055, 1.0}, {1.0, -0.00000059, 0.032} };

	/** Returns the value of a full minutes reward per channel **/
	int64 GetSubsidy(int nMinutes, int nType) { return (((decay[nType][0] * exp(decay[nType][1] * nMinutes)) + decay[nType][2]) * (COIN / 2.0)); }
	
	
	/** Compound the subsidy from a start point to an interval point. **/
	int64 SubsidyInterval(int nMinutes, int nInterval)
	{
		int64 nMoneySupply = 0;
		nInterval += nMinutes;
		
		for(int nMinute = nMinutes; nMinute < nInterval; nMinute++)
			for(int nType = 0; nType < 3; nType++)
				nMoneySupply += (GetSubsidy(nMinute, nType) * 2);
				
		return nMoneySupply;
	}
	
	/** Returns the Calculated Money Supply after nMinutes compounded from the Decay Equations. **/
	int64 CompoundSubsidy(int nMinutes, int nTypes)
    {
		int64 nMoneySupply = 0;
		for(int nMinute = 1; nMinute <= nMinutes; nMinute++)
			for(int nType = (nTypes == 3 ? 0 : nTypes); nType < (nTypes == 3 ? 4 : nTypes + 1); nType++) //nTypes == 3 designates all channels, nTypes == 0, 1, 2 equals miner, ambassador, and developers
				nMoneySupply += (GetSubsidy(nMinute, nType) * 2);
				
		return nMoneySupply;
	}
	
	
	/** Calculates the Actual Money Supply from nReleasedReserve and nMoneySupply. **/
	int64 GetMoneySupply(CBlockIndex* pindex)
	{
		int64 nMoneySupply = pindex->nMoneySupply;
			
		return nMoneySupply;
	}
	
	
	/** Get the Age of the Block Chain in Minutes. **/
	int64 GetChainAge(int64 nTime) { return floor((nTime - (int64)(fTestNet ? NEXUS_TESTNET_TIMELOCK : NEXUS_NETWORK_TIMELOCK)) / 60.0); }
	
	
	/** Get 2% of the Money Supply for interval nMinutes. **/
	int64 GetInflation(int nMinutes, int64 nMoneySupply){ return (nMinutes * nMoneySupply * 0.02) / (365 * 24 * 60); }
	
	
	/** Returns the Time Based value of the Block which is a Fraction of Time's worth of Full Minutes Subsidy. **/
	int64 GetFractionalSubsidy(int nMinutes, int nType, double nFraction)
	{
		int nInterval = floor(nFraction);
		double nRemainder   = nFraction - nInterval;
		
		int64 nSubsidy = 0;
		for(int nMinute = 0; nMinute < nInterval; nMinute++)
			nSubsidy += GetSubsidy(nMinutes + nMinute, nType);
        
        if(GetBoolArg("-logcoinbase", false))
        {
            int64 nReturn = (nSubsidy + (GetSubsidy(nMinutes + nInterval, nType) * nRemainder));
            printf("\nGetFractionalSubsidy(nMinutes=%i, nType=%i, nFraction=%f, nRemainder=%f, nReturn=%lld)\n", nMinutes, nType, nFraction, nRemainder, nReturn);
        }

		return nSubsidy + (GetSubsidy(nMinutes + nInterval, nType) * nRemainder);
	}

	
	/** Returns the Coinbase Reward Generated for Block Timespan **/
	int64 GetCoinbaseReward(const CBlockIndex* pindex, int nChannel, int nType)
	{
		const CBlockIndex* pindexFirst = GetLastChannelIndex(pindex, nChannel);
		if(!pindexFirst->pprev)
			return COIN;
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, nChannel);
		if(!pindexLast->pprev)
			return GetSubsidy(1, nType);
			
			
		int64 nBlockTime = max(pindexFirst->GetBlockTime() - pindexLast->GetBlockTime(), (int64) 1 );
		int64 nMinutes   = ((pindex->nVersion >= 3) ? GetChainAge(pindexFirst->GetBlockTime()) : min(pindexFirst->nChannelHeight,  GetChainAge(pindexFirst->GetBlockTime())));
        
        
        if(GetBoolArg("-logcoinbase", false))
        {
            printf("\nGetCoinbaseReward\n%s\n(nChannel=%i, nType=%i, nBlockTime=%lld, nMinutes=%lld)\n", pindex->ToString().c_str(), nChannel, nType, nBlockTime, nMinutes);
            
            printf("\nFIRST::%s\n", pindexFirst->ToString().c_str());
            CBlock block1;
            if(!block1.ReadFromDisk(pindexFirst))
                error("Failed To Read Block FIRST\n");
            else
                block1.print();
            
            printf("\nLAST ::%s\n", pindexLast->ToString().c_str());
            CBlock block2;
            if(!block2.ReadFromDisk(pindexLast))
                error("Failed To Read Block FIRST\n");
            else
                block2.print();
            
            int64 nBlockTimeTest = max(block1.GetBlockTime() - block2.GetBlockTime(), (int64) 1 );
            int64 nMinutesTest   = GetChainAge(block1.GetBlockTime());
            
            printf("\nBLOCK TEST::(nChannel=%i, nType=%i, nBlockTime=%lld, nMinutes=%lld)\n", nChannel, nType, nBlockTimeTest, nMinutesTest);
            
            printf("\nTime First %lld, Time Second %lld\n", pindexFirst->GetBlockTime(), pindexLast->GetBlockTime());
        }

		
		/** Block Version 3 Coinbase Tx Calculations. **/
		if(pindex->nVersion >= 3)
		{
		
			/** For Block Version 3: Release 3 Minute Reward decayed at Channel Height when Reserves are above 20 Minute Supply. **/
			if(pindexFirst->nReleasedReserve[nType] > GetFractionalSubsidy(pindexFirst->nChannelHeight, nType, 20.0))
				return GetFractionalSubsidy(pindexFirst->nChannelHeight, nType, 3.0);
				
				
			/** Otherwise release 2.5 Minute Reward decayed at Chain Age when Reserves are above 4 Minute Supply. **/
			else if(pindexFirst->nReleasedReserve[nType] > GetFractionalSubsidy(nMinutes, nType, 4.0))
				return GetFractionalSubsidy(nMinutes, nType, 2.5);
				
		}
		
		/** Block Version 1 Coinbase Tx Calculations: Release 2.5 minute reward if supply is behind 4 minutes **/
		else if(pindexFirst->nReleasedReserve[nType] > GetFractionalSubsidy(nMinutes, nType, 4.0))
			return GetFractionalSubsidy(nMinutes, nType, 2.5);

					
		double nFraction = min(nBlockTime / 60.0, 2.5);
        double dFractionTest = min((double) nBlockTime / (double)60, (double)2.5);
        printf("\nFractional Test With Variables nBlockTime=%lld, nFraction=%f, dFractionTest=%f\n", nBlockTime, nFraction, dFractionTest);
        
		if(pindexFirst->nReleasedReserve[nType] == 0 && ReleaseAvailable(pindex, nChannel))
			return GetFractionalSubsidy(nMinutes, nType, nFraction);
			
		return min(GetFractionalSubsidy(nMinutes, nType, nFraction), pindexFirst->nReleasedReserve[nType]);
	}


	/** Releases Nexus into Blockchain for Miners to Create. **/
	int64 ReleaseRewards(int nTimespan, int nStart, int nType)
	{
		int64 nSubsidy = 0;
		for(int nMinutes = nStart; nMinutes < (nStart + nTimespan); nMinutes++)
			nSubsidy += GetSubsidy(nMinutes, nType);
		
		//printf("Reserve %i: %f Nexus | Timespan: %i - %i Minutes\n", nType, (double)nSubsidy / COIN, nStart, (nStart + nTimespan));
		return nSubsidy;
	}
	
	
	/** Releases Nexus into Blockchain [2% of Money Supply] for POS Minting. **/
	int64 ReleaseInflation(int nTimespan, int nStart)
	{
		int64 nSubsidy = GetInflation(nTimespan, CompoundSubsidy(nStart));
		
		printf("Inflation: %f Nexus | Timespan %i - %i Minutes\n", (double)nSubsidy / COIN, nStart, (nStart + nTimespan));
		return nSubsidy;
	}
	
	
	int64 GetReleasedInflation(const CBlockIndex* pindex)
	{
		
		return 0;
	}
	

	/** Calculates the release of new rewards based on the Network Time **/
	int64 GetReleasedReserve(const CBlockIndex* pindex, int nChannel, int nType)
	{
		const CBlockIndex* pindexFirst = GetLastChannelIndex(pindex, nChannel);
		int nMinutes = GetChainAge(pindexFirst->GetBlockTime());
		if(!pindexFirst->pprev)
			return COIN;
		
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindexFirst->pprev, nChannel);
		if(!pindexLast->pprev)
			return ReleaseRewards(nMinutes + 5, 1, nType);
			
		/** Only allow rewards to be released one time per minute **/
		int nLastMinutes = GetChainAge(pindexLast->GetBlockTime());
		if(nMinutes == nLastMinutes)
			return 0;
		
		return ReleaseRewards((nMinutes - nLastMinutes), nLastMinutes, nType);
	}
	
	
	/** If the Reserves are Depleted, this Tells a miner if there is a new Time Interval with their Previous Block which would signal new release into reserve.
		If for some reason this is a false flag, the block will be rejected by the network for attempting to deplete the reserves past 0 **/
	bool ReleaseAvailable(const CBlockIndex* pindex, int nChannel)
	{
		const CBlockIndex* pindexLast = GetLastChannelIndex(pindex, nChannel);
		if(!pindexLast->pprev)
			return true;
			
		return !(GetChainAge(GetUnifiedTimestamp()) == GetChainAge(pindexLast->GetBlockTime()));
	}
}
