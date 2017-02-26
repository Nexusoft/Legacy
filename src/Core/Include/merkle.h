/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_INCLUDE_MERKLE_H
#define NEXUS_CORE_INCLUDE_MERKLE_H

namespace Core
{
	
	/** A transaction with a merkle branch linking it to the block chain. */
	class CMerkleTx : public CTransaction
	{
	public:
		uint1024 hashBlock;
		std::vector<uint512> vMerkleBranch;
		int nIndex;

		// memory only
		mutable bool fMerkleVerified;


		CMerkleTx()
		{
			Init();
		}

		CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
		{
			Init();
		}

		void Init()
		{
			hashBlock = 0;
			nIndex = -1;
			fMerkleVerified = false;
		}


		IMPLEMENT_SERIALIZE
		(
			nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
			nVersion = this->nVersion;
			READWRITE(hashBlock);
			READWRITE(vMerkleBranch);
			READWRITE(nIndex);
		)


		int SetMerkleBranch(const CBlock* pblock=NULL);
		int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
		int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
		bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
		int GetBlocksToMaturity() const;
		bool AcceptToMemoryPool(LLD::CIndexDB& indexdb, bool fCheckInputs=true);
		bool AcceptToMemoryPool();
	};
	
}
