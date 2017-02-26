/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_TEMPLATES_TRUST_H
#define NEXUS_CORE_TEMPLATES_TRUST_H

namespace Core
{
	
	/** Class to Store the Trust Key and It's Interest Rate. **/
	class CTrustKey
	{
	public:
	
		/** The Public Key associated with Trust Key. **/
		std::vector<unsigned char> vchPubKey;
		
		unsigned int nVersion;
		uint1024  hashGenesisBlock;
		uint512   hashGenesisTx;
		unsigned int nGenesisTime;
		
		/** Previous Blocks Vector to store list of blocks of this Trust Key. **/
		mutable std::vector<uint1024> hashPrevBlocks;
		
		CTrustKey() { SetNull(); }
		CTrustKey(std::vector<unsigned char> vchPubKeyIn, uint1024 hashBlockIn, uint512 hashTxIn, unsigned int nTimeIn)
		{
			SetNull();
			
			nVersion               = 1;
			vchPubKey              = vchPubKeyIn;
			hashGenesisBlock       = hashBlockIn;
			hashGenesisTx          = hashTxIn;
			nGenesisTime           = nTimeIn;
		}
		
		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			
			READWRITE(vchPubKey);
			READWRITE(hashGenesisBlock);
			READWRITE(hashGenesisTx);
			READWRITE(nGenesisTime);
		)
		
		
		/** Set the Data structure to Null. **/
		void SetNull() 
		{ 
			nVersion             = 1;
			hashGenesisBlock     = 0;
			hashGenesisTx        = 0;
			nGenesisTime         = 0;
			
			vchPubKey.clear();
		}
		
		/** Hash of a Trust Key to Verify the Key's Root. **/
		uint512 GetHash() const { return LLH::SK512(vchPubKey, BEGIN(hashGenesisBlock), END(nGenesisTime)); }
		
		/** Determine how old the Trust Key is From Timestamp. **/
		uint64 Age(unsigned int nTime) const;
		
		/** Time Since last Trust Block. **/
		uint64 BlockAge(unsigned int nTime) const;
		
		/** Flag to Determine if Class is Empty and Null. **/
		bool IsNull()  const { return (hashGenesisBlock == 0 || hashGenesisTx == 0 || nGenesisTime == 0 || vchPubKey.empty()); }
		bool Expired(unsigned int nTime) const;
		bool CheckGenesis(CBlock cBlock) const;
		
		void Print()
		{
			uint576 cKey;
			cKey.SetBytes(vchPubKey);
			
			printf("CTrustKey(Hash = %s, Key = %s, Genesis = %s, Tx = %s, Time = %u, Age = %"PRIu64", BlockAge = %"PRIu64", Expired = %s)\n", GetHash().ToString().c_str(), cKey.ToString().c_str(), hashGenesisBlock.ToString().c_str(), hashGenesisTx.ToString().c_str(), nGenesisTime, Age(GetUnifiedTimestamp()), BlockAge(GetUnifiedTimestamp()), Expired(GetUnifiedTimestamp()) ? "TRUE" : "FALSE");
		}
	};	
	
	
	/** Holding Class Structure to contain the Trust Keys. **/
	class CTrustPool
	{
	private:
		mutable CCriticalSection cs;
		mutable std::map<uint576, CTrustKey> mapTrustKeys;

	public:
		/** The Trust Key Owned By Current Node. **/
		std::vector<unsigned char>   vchTrustKey;
		
		/** Helper Function to Find Trust Key. **/
		bool HasTrustKey(unsigned int nTime);
		
		bool Check(CBlock cBlock);
		bool Accept(CBlock cBlock, bool fInit = false);
		bool Remove(CBlock cBlock);
		
		bool Exists(uint576 cKey)    const { return mapTrustKeys.count(cKey); }
		bool IsGenesis(uint576 cKey) const { return mapTrustKeys[cKey].hashPrevBlocks.empty(); }
		
		double InterestRate(uint576 cKey, unsigned int nTime) const;
		
		/* The Trust score of the Trust Key. Determines the Age and Interest Rates. */
		uint64 TrustScore(uint576 cKey, unsigned int nTime) const;
		
		/* Locate a Trust Key in the Trust Pool. */
		CTrustKey Find(uint576 cKey) const { return mapTrustKeys[cKey]; }
	};
	
	
}
