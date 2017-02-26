/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_MERKLE_H
#define NEXUS_CORE_INCLUDE_MERKLE_H

namespace Core
{
	
	/** Used to marshal pointers into hashes for db storage. */
	class CDiskBlockIndex : public CBlockIndex
	{
	public:
		uint1024 hashPrev;
		uint1024 hashNext;

		CDiskBlockIndex()
		{
			hashPrev = 0;
			hashNext = 0;
		}

		explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
		{
			hashPrev = (pprev ? pprev->GetBlockHash() : 0);
			hashNext = (pnext ? pnext->GetBlockHash() : 0);
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);

			if (!(nType & SER_LLD)) 
			{
				READWRITE(hashNext);
				READWRITE(nFile);
				READWRITE(nBlockPos);
				READWRITE(nMint);
				READWRITE(nMoneySupply);
				READWRITE(nFlags);
				READWRITE(nStakeModifier);
			}
			else
			{
				READWRITE(hashNext);
				READWRITE(nFile);
				READWRITE(nBlockPos);
				READWRITE(nMint);
				READWRITE(nMoneySupply);
				READWRITE(nChannelHeight);
				READWRITE(nChainTrust);
				
				READWRITE(nCoinbaseRewards[0]);
				READWRITE(nCoinbaseRewards[1]);
				READWRITE(nCoinbaseRewards[2]);
				READWRITE(nReleasedReserve[0]);
				READWRITE(nReleasedReserve[1]);
				READWRITE(nReleasedReserve[2]);
				
				//READWRITE(PendingCheckpoint);
			}

			// block header
			READWRITE(this->nVersion);
			READWRITE(hashPrev);
			READWRITE(hashMerkleRoot);
			READWRITE(nChannel);
			READWRITE(nHeight);
			READWRITE(nBits);
			READWRITE(nNonce);
			READWRITE(nTime);
			
		)

		uint1024 GetBlockHash() const
		{
			CBlock block;
			block.nVersion        = nVersion;
			block.hashPrevBlock   = hashPrev;
			block.hashMerkleRoot  = hashMerkleRoot;
			block.nChannel        = nChannel;
			block.nHeight         = nHeight;
			block.nBits           = nBits;
			block.nNonce          = nNonce;
			
			return block.GetHash();
		}


		std::string ToString() const
		{
			std::string str = "CDiskBlockIndex(";
			str += CBlockIndex::ToString();
			str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashNext=%s)",
				GetBlockHash().ToString().c_str(),
				hashPrev.ToString().substr(0,20).c_str(),
				hashNext.ToString().substr(0,20).c_str());
			return str;
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};
}
