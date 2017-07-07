/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_MINING_H
#define NEXUS_LLP_INCLUDE_MINING_H

#include <vector>
#include <string>
#include <map>

#include "../../Util/include/debug.h"

namespace LLP
{
	
		/** Class to Create and Manage the Pool Payout Coinbase Tx. **/
	class Coinbase
	{
	public:
		/** The Transaction Outputs to be Serialized to Mining LLP. **/
		std::map<std::string, uint64> vOutputs;
		
		/** The Value of this current Coinbase Payout. **/
		uint64 nMaxValue, nPoolFee;
		
		
		/** Constructor to Class. **/
		Coinbase(std::vector<unsigned char> vData, uint64 nValue){ Deserialize(vData, nValue); }
		
		
		/** Deserialize the Coinbase Transaction. **/
		void Deserialize(std::vector<unsigned char> vData, uint64 nValue)
		{
			/** Set the Max Value for this Transaction. **/
			nMaxValue = nValue;
					
			/** First byte of Serialization Packet is the Number of Records. **/
			unsigned int nSize = vData[0], nIterator = 9;
			
			/** Bytes 1 - 8 is the Pool Fee for that Round. **/
			nPoolFee  = bytes2uint64(vData, 1);
					
			/** Loop through every Record. **/
			for(unsigned int nIndex = 0; nIndex < nSize; nIndex++)
			{
				/** De-Serialize the Address String and uint64 nValue. **/
				unsigned int nLength = vData[nIterator];
					
				std::string strAddress = bytes2string(std::vector<unsigned char>(vData.begin() + nIterator + 1, vData.begin() + nIterator + 1 + nLength));
				uint64 nValue = bytes2uint64(std::vector<unsigned char>(vData.begin() + nIterator + 1 + nLength, vData.begin() + nIterator + 1 + nLength + 8));
						
				/** Add the Transaction as an Output. **/
				vOutputs[strAddress] = nValue;
						
				/** Increment the Iterator. **/
				nIterator += (nLength + 9);
			}
		}
		
		
		/** Flag to Know if the Coinbase Tx has been built Successfully. **/
		bool IsValid()
		{
			uint64 nCurrentValue = nPoolFee;
			for(std::map<std::string, uint64>::iterator nIterator = vOutputs.begin(); nIterator != vOutputs.end(); nIterator++)
				nCurrentValue += nIterator->second;
				
			return nCurrentValue == nMaxValue;
		}
		
		/** Output the Transactions in the Coinbase Container. **/
		void Print()
		{
			printf("\n\n +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\n");
			uint64 nTotal = 0;
			for(std::map<std::string, uint64>::iterator nIterator = vOutputs.begin(); nIterator != vOutputs.end(); nIterator++)
			{
				printf("%s:%f\n", nIterator->first.c_str(), nIterator->second / 1000000.0);
				nTotal += nIterator->second;
			}
			
			printf("Total Value of Coinbase = %f\n", nTotal / 1000000.0);
			printf("Set Value of Coinbase = %f\n", nMaxValue / 1000000.0);
			printf("PoolFee in Coinbase %f\n", nPoolFee / 1000000.0);
			printf("\n\nIs Complete: %s\n", IsValid() ? "TRUE" : "FALSE");
			printf("\n\n +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\n");
		}
		
	};
	
	
}


#endif
