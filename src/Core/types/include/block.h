/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_CORE_INCLUDE_BLOCK_H
#define NEXUS_CORE_INCLUDE_BLOCK_H

#include "transaction.h"
#include "../../../LLC/hash/macro.h"


class CBigNum;

namespace LLP 
{ 
	class Coinbase; 
	class CLegacyNode;
}

namespace LLD 
{
	class CIndexDB;
}

namespace Wallet 
{ 
	class CWallet; 
	class CReserveKey; 
}

namespace Core
{
	class CBlockIndex;

	
	/** Nodes collect new transactions into a block, hash them into a hash tree,
	 * and scan through nonce values to make the block's hash satisfy proof-of-work
	 * requirements.  When they solve the proof-of-work, they broadcast the block
	 * to everyone and the block is added to the block chain.  The first transaction
	 * in the block is a special one that creates a new coin owned by the creator
	 * of the block.
	 *
	 * Blocks are appended to blk0001.dat files on disk.  Their location on disk
	 * is indexed by CBlockIndex objects in memory.
	 */
	class CBlock
	{
	public:
		
		/* Core Block Header. */
		unsigned int nVersion;
		uint1024 hashPrevBlock;
		uint512 hashMerkleRoot;
		unsigned int nChannel;
		unsigned int nHeight;
		unsigned int nBits;
		uint64 nNonce;
		
		
		/* The Block Time locked in Block Signature. */
		unsigned int nTime;

		
		/* Nexus block signature */
		std::vector<unsigned char> vchBlockSig;
		
		
		/* Transactions for Current Block. */
		std::vector<CTransaction> vtx;

		
		/* Memory Only Data. */
		mutable std::vector<uint512> vMerkleTree;
		uint512 hashPrevChecksum;

		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(hashPrevBlock);
			READWRITE(hashMerkleRoot);
			READWRITE(nChannel);
			READWRITE(nHeight);
			READWRITE(nBits);
			READWRITE(nNonce);
			READWRITE(nTime);

			// ConnectBlock depends on vtx following header to generate CDiskTxPos
			if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
			{
				READWRITE(vtx);
				READWRITE(vchBlockSig);
			}
			else if (fRead)
			{
				const_cast<CBlock*>(this)->vtx.clear();
				const_cast<CBlock*>(this)->vchBlockSig.clear();
			}
		)

		CBlock()
		{
			SetNull();
		}
		
		CBlock(unsigned int nVersionIn, uint1024 hashPrevBlockIn, unsigned int nChannelIn, unsigned int nHeightIn)
		{
			SetNull();
			
			nVersion = nVersionIn;
			hashPrevBlock = hashPrevBlockIn;
			nChannel = nChannelIn;
			nHeight  = nHeightIn;
		}


		/* Set block to a NULL state. */
		void SetNull()
		{
			/* bdg question: should this use the current version instead of hard coded 3? */
			nVersion = 3;
			hashPrevBlock = 0;
			hashMerkleRoot = 0;
			nChannel = 0;
			nHeight = 0;
			nBits = 0;
			nNonce = 0;
			nTime = 0;

			vtx.clear();
			vchBlockSig.clear();
			vMerkleTree.clear();
		}
		

		/* bdg note: SetChannel is never used */
		/* Set the Channel for block. */
		void SetChannel(unsigned int nNewChannel)
		{
			nChannel = nNewChannel;
		}
		
		
		/* Get the Channel block is produced from. */
		int GetChannel() const
		{
			return nChannel;
		}
		
		
		/* Check the NULL state of the block. */
		bool IsNull() const
		{
			return (nBits == 0);
		}
		
		
		/* Return the Block's current UNIX Timestamp. */
		int64 GetBlockTime() const
		{
			return (int64)nTime;
		}
		

		/* bdg question: should this check if the block is in the prime channel first? */
		/* Get the prime number of the block. */
		CBigNum GetPrime() const
		{
			return CBigNum(GetHash() + nNonce);
		}
		
		
		/* Generate a Hash For the Block from the Header. */
		uint1024 GetHash() const
		{
			/** Hashing template for CPU miners uses nVersion to nBits **/
			if(GetChannel() == 1)
				return LLC::HASH::SK1024(BEGIN(nVersion), END(nBits));
				
			/** Hashing template for GPU uses nVersion to nNonce **/
			return LLC::HASH::SK1024(BEGIN(nVersion), END(nNonce));
		}
		
		
		/* Generate the Signature Hash Required After Block completes Proof of Work / Stake. */
		uint1024 SignatureHash() const
		{
			if(nVersion < 5)
				return LLC::HASH::SK1024(BEGIN(nVersion), END(nTime));
			else
				return LLC::HASH::SK1024(BEGIN(nVersion), END(hashPrevChecksum));
		}
		

		/* Update the nTime of the current block. */
		void UpdateTime();
		
		
		/* Check flags for nPoS block. */
		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}
		
		
		/* Check flags for PoW block. */
		bool IsProofOfWork() const
		{
			return (nChannel == 1 || nChannel == 2);
		}

		
		/* Generate the Merkle Tree from uint512 hashes. */
		uint512 BuildMerkleTree() const
		{
			vMerkleTree.clear();
			BOOST_FOREACH(const CTransaction& tx, vtx)
				vMerkleTree.push_back(tx.GetHash());
			int j = 0;
			for (int nSize = (int)vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				for (int i = 0; i < nSize; i += 2)
				{
					int i2 = std::min(i+1, nSize-1);
					vMerkleTree.push_back(LLC::HASH::SK512(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
											    BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
				}
				j += nSize;
			}
			return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
		}
		
		
		/* Get the current Branch that is being worked on. */
		std::vector<uint512> GetMerkleBranch(int nIndex) const
		{
			if (vMerkleTree.empty())
				BuildMerkleTree();
			std::vector<uint512> vMerkleBranch;
			int j = 0;
			for (int nSize = (int)vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				int i = std::min(nIndex^1, nSize-1);
				vMerkleBranch.push_back(vMerkleTree[j+i]);
				nIndex >>= 1;
				j += nSize;
			}
			return vMerkleBranch;
		}

		
		/* Check that the Merkle branch matches hash tree. */
		static uint512 CheckMerkleBranch(uint512 hash, const std::vector<uint512>& vMerkleBranch, int nIndex)
		{
			if (nIndex == -1)
				return 0;
			for(int i = 0; i < vMerkleBranch.size(); i++)
			{
				if (nIndex & 1)
					hash = LLC::HASH::SK512(BEGIN(vMerkleBranch[i]), END(vMerkleBranch[i]), BEGIN(hash), END(hash));
				else
					hash = LLC::HASH::SK512(BEGIN(hash), END(hash), BEGIN(vMerkleBranch[i]), END(vMerkleBranch[i]));
				nIndex >>= 1;
			}
			return hash;
		}
	
		/* Write Block to Disk File. */
		bool WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet);

		
		/* Read Block from Disk File. */
		bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions=true);
		
		
		/* Read Block from Disk File by Index Object. */
		bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);


		/* Dump the Block data to Console / Debug.log. */
		void print() const;

		
		/* Disconnect all associated inputs from a block. */
		bool DisconnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		
		
		/* Connect all associated inputs from a block. */
		bool ConnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		
		
		/* Verify the Proof of Work satisfies network requirements. */
		bool VerifyWork() const;
		
		
		/* Verify the Proof of Stake satisfies network requirements. */
		bool VerifyStake() const;
		
		/* bdg note: StakeWeight is not implemented. */
		/* Determine the Stake Weight of Given Block. */
		bool StakeWeight();
		
		
		/* Sign the block with the key that found the block. */
		bool SignBlock(const Wallet::CKeyStore& keystore);
		
		
		/* Check that the block signature is a valid signature. */
		bool CheckBlockSignature() const;
		
	};



	/** The block chain is a tree shaped structure starting with the
	 * genesis block at the root, with each block potentially having multiple
	 * candidates to be the next block.  pprev and pnext link a path through the
	 * main/longest chain.  A blockindex may have multiple pprev pointing back
	 * to it, but pnext will only point forward to the longest branch, or will
	 * be null if the block is not part of the longest chain.
	 */
	class CBlockIndex
	{
	public:
		const uint1024* phashBlock;
		CBlockIndex* pprev;
		CBlockIndex* pnext;
		unsigned int nFile;
		unsigned int nBlockPos;
		
		uint64 nChainTrust; // Nexus: trust score of block chain
		int64  nMint;
		int64  nMoneySupply;
		int64  nChannelHeight;
		int64  nReleasedReserve[4];
		int64  nCoinbaseRewards[3];
		
				
		/* Used to store the pending Checkpoint in Blockchain. 
			This is also another proof that this block is descendant 
			of most recent Pending Checkpoint. This helps Nexus
			deal with reorganizations of a Pending Checkpoint **/
		std::pair<unsigned int, uint1024> PendingCheckpoint;
		

		unsigned int nFlags;  // Nexus: block index flags
		uint64 nStakeModifier;

		// block header
		unsigned int nVersion;
		uint512 hashMerkleRoot;
		unsigned int nChannel;
		unsigned int nHeight;
		unsigned int nBits;
		uint64 nNonce;

		unsigned int nTime;

		CBlockIndex()
		{
			phashBlock = NULL;
			pprev = NULL;
			pnext = NULL;
			nFile = 0;
			nBlockPos = 0;
			
			nChainTrust = 0;
			nMint = 0;
			nMoneySupply = 0;
			nFlags = 0;
			nStakeModifier = 0;
			
			nCoinbaseRewards[0] = 0;
			nCoinbaseRewards[1] = 0;
			nCoinbaseRewards[2] = 0;

			nVersion       = 0;
			hashMerkleRoot = 0;
			nHeight        = 0;
			nBits          = 0;
			nNonce         = 0;
			
			nTime          = 0;
		}
		
		CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock& block)
		{
			phashBlock = NULL;
			pprev = NULL;
			pnext = NULL;
			nFile = nFileIn;
			nBlockPos = nBlockPosIn;
			nChainTrust = 0;
			nMint = 0;
			nMoneySupply = 0;
			nStakeModifier = 0;
			nFlags = 0;
			
			nCoinbaseRewards[0] = 0;
			nCoinbaseRewards[1] = 0;
			nCoinbaseRewards[2] = 0;
				
			if (block.IsProofOfWork() && block.nHeight > 0)
			{
				int nSize = (int)block.vtx[0].vout.size();
				int nIter = (nSize > 2) ? nSize - 2 : 1;
				for(int nIndex = 0; nIndex < nIter; nIndex++)
					nCoinbaseRewards[0] += block.vtx[0].vout[nIndex].nValue;

				if (nSize > 2) {
					nCoinbaseRewards[1] = block.vtx[0].vout[nSize - 2].nValue;
					nCoinbaseRewards[2] = block.vtx[0].vout[nSize - 1].nValue;
				}
			}

			nVersion       = block.nVersion;
			hashMerkleRoot = block.hashMerkleRoot;
			nChannel       = block.nChannel;
			nHeight        = block.nHeight;
			nBits          = block.nBits;
			nNonce         = block.nNonce;
			
			nTime          = block.nTime;
		}

		CBlock GetBlockHeader() const
		{
			CBlock block;
			block.nVersion       = nVersion;
			if (pprev)
				block.hashPrevBlock = pprev->GetBlockHash();
			block.hashMerkleRoot = hashMerkleRoot;
			block.nChannel       = nChannel;
			block.nHeight        = nHeight;
			block.nBits          = nBits;
			block.nNonce         = nNonce;
			
			block.nTime          = nTime;
			
			return block;
		}
		
		int GetChannel() const
		{
			return nChannel;
		}

		uint1024 GetBlockHash() const
		{
			return *phashBlock;
		}

		int64 GetBlockTime() const
		{
			return (int64)nTime;
		}

		uint64 GetBlockTrust() const
		{

			/** bdg thought: what if we look back at the past two blocks
			 **                 give 3 if last two are different, 2 if only
			 **                 one of the last two is different.
			 ** Give higher block trust if last block was of different channel
			 **/
			if(pprev && pprev->GetChannel() != GetChannel())
				return 3;
			
			/** Normal Block Trust Increment. **/
			return 1;
		}
		
		bool IsInMainChain() const
		{
			return (pnext || this == pindexBest);
		}

		/* bdg question: why is this not checking the proof of work? */
		bool CheckIndex() const
		{
			return true;//IsProofOfWork() ? CheckProofOfWork(GetBlockHash(), nBits) : true;
		}
		
		bool IsProofOfWork() const
		{
			return (nChannel > 0);
		}

		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}

		/* bdg note: this is never used */
		bool EraseBlockFromDisk();
		
		std::string ToString() const;
		void print() const;
	};
	
	
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

			}

			READWRITE(this->nVersion);
			READWRITE(hashPrev);
			READWRITE(hashMerkleRoot);
			READWRITE(nChannel);
			READWRITE(nHeight);
			READWRITE(nBits);
			READWRITE(nNonce);
			READWRITE(nTime);
			
		)

		/* Get the Block Hash. */
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
		
		/* Output all data into a std::string. */
		std::string ToString() const;
		
		/* Dump the data into console / debug.log. */
		void print() const;
	};




	/** Describes a place in the block chain to another node such that if the
	 * other node doesn't have the same branch, it can find a recent common trunk.
	 * The further back it is, the further before the fork it may be.
	 */
	class CBlockLocator
	{
	protected:
		std::vector<uint1024> vHave;
		
	public:
		
		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);
			READWRITE(vHave);
		)

		CBlockLocator()
		{
		}

		explicit CBlockLocator(const CBlockIndex* pindex)
		{
			Set(pindex);
		}

		explicit CBlockLocator(uint1024 hashBlock)
		{
			std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
			if (mi != mapBlockIndex.end())
				Set((*mi).second);
		}
		
		
		/* Set by List of Vectors. */
		CBlockLocator(const std::vector<uint1024>& vHaveIn)
		{
			vHave = vHaveIn; 
		}
		
		
		/* Set the State of Object to NULL. */
		void SetNull()
		{
			vHave.clear();
		}
		
		
		/* Check the State of Object as NULL. */
		bool IsNull()
		{
			return vHave.empty();
		}
		
		
		/* Set from Block Index Object. */
		void Set(const CBlockIndex* pindex);
		
		
		/* Find the total blocks back locator determined. */
		int GetDistanceBack();
		
		
		/* Get the Index object stored in Locator. */
		CBlockIndex* GetBlockIndex();
		
		
		/* Get the hash of block. */
		uint1024 GetBlockHash();
		

		/* bdg note: GetHeight is never used. */
		/* Get the Height of the Locator. */
		int GetHeight();
		
	};
	
		
		
	
	/* __________________________________________________ (Block Processing Methods) __________________________________________________  */
	
	
	/* Search back for an index given PoW / PoS parameters. */
	const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
	
	
	/* Search back for an index of given Mining Channel. */
	const CBlockIndex* GetLastChannelIndex(const CBlockIndex* pindex, int nChannel);

	
	
	/* Check the disk space for the current partition database is stored in. */
	bool CheckDiskSpace(uint64 nAdditionalBytes = 0);
	
	
	/* Read the block from file and binary position (blk0001.dat), */
	FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode);
	
	
	/* Add a block to the block file and binary position (blk0001.dat). */
	FILE* AppendBlockFile(unsigned int& nFileRet);
	
	
	/* Load the Genesis and other blocks from the BDB/LLD Indexes. */
	bool LoadBlockIndex(bool fAllowNew = true);

	
	/* Create a new block with given parameters and optional coinbase transaction. */
	CBlock* CreateNewBlock(Wallet::CReserveKey& reservekey, Wallet::CWallet* pwallet, unsigned int nChannel, unsigned int nID = 1, LLP::Coinbase* pCoinbase = NULL);
	
	
	/* Add the Transactions to a Block from the Memroy Pool (TODO: Decide whether to put this in transactions.h/transactions.cpp or leave it here). */
	void AddTransactions(std::vector<CTransaction>& vtx, CBlockIndex* pindexPrev);
	
	
	/* Check that the Proof of work is valid for the given Mined block before sending it into the processing queue. */
	bool CheckWork(CBlock* pblock, Wallet::CWallet& wallet, Wallet::CReserveKey& reservekey);
	

	/* TODO: Remove this where not needed. */
	std::string GetChannelName(int nChannel);
	

}

#endif
