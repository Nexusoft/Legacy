/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_TEMPLATES_BLOCK_H
#define NEXUS_CORE_TEMPLATES_BLOCK_H

namespace Core
{

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
		
		/* Block Signature Variables. */
		unsigned int nTime;

		/* Nexus: block signature
		 * Signed by coinbase / coinstake txout[0]'s owner
		 * Seals the nTime and nNonce into Block
		 * References Previous Block's Signature for a Signature Chain.
		 */
		std::vector<unsigned char> vchBlockSig;
		
		// network and disk
		std::vector<CTransaction> vtx;

		// memory only
		mutable std::vector<uint512> vMerkleTree;
		uint512 hashPrevChecksum;

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

		void SetNull()
		{
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
			nDoS = 0;
		}
		
		void SetChannel(unsigned int nNewChannel)
		{
			nChannel = nNewChannel;
		}
		
		int GetChannel() const
		{
			return nChannel;
		}

		bool IsNull() const
		{
			return (nBits == 0);
		}
		
		CBigNum GetPrime() const
		{
			return CBigNum(GetHash() + nNonce);
		}

		/** Generate a Hash For the Block from the Header. **/
		uint1024 GetHash() const
		{
			/** Hashing template for CPU miners uses nVersion to nBits **/
			if(GetChannel() == 1)
				return SK1024(BEGIN(nVersion), END(nBits));
				
			/** Hashing template for GPU uses nVersion to nNonce **/
			return SK1024(BEGIN(nVersion), END(nNonce));
		}
		
		
		/** Generate the Signature Hash Required After Block completes Proof of Work / Stake.
			This is to seal the Block Timestamp / nNonce [For CPU Channel] into the Block Signature while allowing it
			to be set independent of proof of work searching. This prevents this data from being tampered with after broadcast. **/
		uint1024 SignatureHash() const { 
			if(nVersion < 5)
				return SK1024(BEGIN(nVersion), END(nTime));
			else
				return SK1024(BEGIN(nVersion), END(hashPrevChecksum));
		}

		
		/** Return the Block's current Timestamp. **/
		int64 GetBlockTime() const
		{
			return (int64)nTime;
		}

		void UpdateTime();


		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}

		bool IsProofOfWork() const
		{
			return (nChannel == 1 || nChannel == 2);
		}


		uint512 BuildMerkleTree() const
		{
			vMerkleTree.clear();
			BOOST_FOREACH(const CTransaction& tx, vtx)
				vMerkleTree.push_back(tx.GetHash());
			int j = 0;
			for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				for (int i = 0; i < nSize; i += 2)
				{
					int i2 = std::min(i+1, nSize-1);
					vMerkleTree.push_back(LLH::SK512(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
											    BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
				}
				j += nSize;
			}
			return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
		}

		std::vector<uint512> GetMerkleBranch(int nIndex) const
		{
			if (vMerkleTree.empty())
				BuildMerkleTree();
			std::vector<uint512> vMerkleBranch;
			int j = 0;
			for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				int i = std::min(nIndex^1, nSize-1);
				vMerkleBranch.push_back(vMerkleTree[j+i]);
				nIndex >>= 1;
				j += nSize;
			}
			return vMerkleBranch;
		}

		static uint512 CheckMerkleBranch(uint512 hash, const std::vector<uint512>& vMerkleBranch, int nIndex)
		{
			if (nIndex == -1)
				return 0;
			BOOST_FOREACH(const uint512& otherside, vMerkleBranch)
			{
				if (nIndex & 1)
					hash = LLH::SK512(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
				else
					hash = LLH::SK512(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
				nIndex >>= 1;
			}
			return hash;
		}


		bool WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet)
		{
			// Open history file to append
			CAutoFile fileout = CAutoFile(AppendBlockFile(nFileRet), SER_DISK, DATABASE_VERSION);
			if (!fileout)
				return error("CBlock::WriteToDisk() : AppendBlockFile failed");

			// Write index header
			unsigned char pchMessageStart[4];
			LLP::GetMessageStart(pchMessageStart);
			unsigned int nSize = fileout.GetSerializeSize(*this);
			fileout << FLATDATA(pchMessageStart) << nSize;

			// Write block
			long fileOutPos = ftell(fileout);
			if (fileOutPos < 0)
				return error("CBlock::WriteToDisk() : ftell failed");
			nBlockPosRet = fileOutPos;
			fileout << *this;

			// Flush stdio buffers and commit to disk before returning
			fflush(fileout);
			if (!IsInitialBlockDownload() || (nBestHeight+1) % 500 == 0)
			{
	#ifdef WIN32
				_commit(_fileno(fileout));
	#else
				fsync(fileno(fileout));
	#endif
			}

			return true;
		}

		bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions=true)
		{
			SetNull();

			// Open history file to read
			CAutoFile filein = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb"), SER_DISK, DATABASE_VERSION);
			if (!filein)
				return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
			if (!fReadTransactions)
				filein.nType |= SER_BLOCKHEADERONLY;

			// Read block
			try {
				filein >> *this;
			}
			catch (std::exception &e) {
				return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
			}

			return true;
		}



		void print() const
		{
			printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nChannel = %u, nHeight = %u, nNonce=%"PRIu64", vtx=%d, vchBlockSig=%s)\n",
				GetHash().ToString().substr(0,20).c_str(),
				nVersion,
				hashPrevBlock.ToString().substr(0,20).c_str(),
				hashMerkleRoot.ToString().substr(0,10).c_str(),
				nTime, nBits, nChannel, nHeight, nNonce,
				vtx.size(),
				HexStr(vchBlockSig.begin(), vchBlockSig.end()).c_str());
			for (unsigned int i = 0; i < vtx.size(); i++)
			{
				printf("  ");
				vtx[i].print();
			}
			printf("  vMerkleTree: ");
			for (unsigned int i = 0; i < vMerkleTree.size(); i++)
				printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
			printf("\n");
		}


		bool DisconnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		bool ConnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);
		bool SetBestChain(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew);
		bool AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos);
		bool CheckBlock() const;
		
		bool VerifyWork() const;
		bool VerifyStake() const;
		
		bool AcceptBlock();
		bool StakeWeight();
		bool SignBlock(const Wallet::CKeyStore& keystore);
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
		
				
		/** Used to store the pending Checkpoint in Blockchain. 
			This is also another proof that this block is descendant 
			of most recent Pending Checkpoint. This helps Nexus
			deal with reorganizations of a Pending Checkpoint **/
		std::pair<unsigned int, uint1024> PendingCheckpoint;
		

		unsigned int nFlags;  // Nexus: block index flags
		enum  
		{
			BLOCK_PROOF_OF_STAKE = (1 << 0), // is proof-of-stake block
			BLOCK_STAKE_ENTROPY  = (1 << 1), // entropy bit for stake modifier
			BLOCK_STAKE_MODIFIER = (1 << 2), // regenerated stake modifier
		};

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
				unsigned int nSize = block.vtx[0].vout.size();
				for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
					nCoinbaseRewards[0] += block.vtx[0].vout[nIndex].nValue;
						
				nCoinbaseRewards[1] = block.vtx[0].vout[nSize - 2].nValue;
				nCoinbaseRewards[2] = block.vtx[0].vout[nSize - 1].nValue;
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
				
			/** Give higher block trust if last block was of different channel **/
			if(pprev && pprev->GetChannel() != GetChannel())
				return 3;
			
			/** Normal Block Trust Increment. **/
			return 1;
		}

		bool IsInMainChain() const
		{
			return (pnext || this == pindexBest);
		}

		bool CheckIndex() const
		{
			return true;//IsProofOfWork() ? CheckProofOfWork(GetBlockHash(), nBits) : true;
		}

		bool EraseBlockFromDisk()
		{
			// Open history file
			CAutoFile fileout = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb+"), SER_DISK, DATABASE_VERSION);
			if (!fileout)
				return false;

			// Overwrite with empty null block
			CBlock block;
			block.SetNull();
			fileout << block;

			return true;
		}

		bool IsProofOfWork() const
		{
			return (nChannel > 0);
		}

		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}

		std::string ToString() const
		{
			return strprintf("CBlockIndex(nprev=%08x, pnext=%08x, nFile=%d, nBlockPos=%-6d nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s), merkle=%s, hashBlock=%s)",
				pprev, pnext, nFile, nBlockPos, nHeight,
				FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
				IsProofOfStake() ? "PoS" : "PoW",
				hashMerkleRoot.ToString().substr(0,10).c_str(),
				GetBlockHash().ToString().substr(0,20).c_str());
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
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

		CBlockLocator(const std::vector<uint1024>& vHaveIn)
		{
			vHave = vHaveIn;
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);
			READWRITE(vHave);
		)

		void SetNull()
		{
			vHave.clear();
		}

		bool IsNull()
		{
			return vHave.empty();
		}

		void Set(const CBlockIndex* pindex)
		{
			vHave.clear();
			int nStep = 1;
			while (pindex)
			{
				vHave.push_back(pindex->GetBlockHash());

				// Exponentially larger steps back
				for (int i = 0; pindex && i < nStep; i++)
					pindex = pindex->pprev;
				if (vHave.size() > 10)
					nStep *= 2;
			}
			vHave.push_back(hashGenesisBlock);
		}

		int GetDistanceBack()
		{
			// Retrace how far back it was in the sender's branch
			int nDistance = 0;
			int nStep = 1;
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return nDistance;
				}
				nDistance += nStep;
				if (nDistance > 10)
					nStep *= 2;
			}
			return nDistance;
		}

		CBlockIndex* GetBlockIndex()
		{
			// Find the first block the caller has in the main chain
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return pindex;
				}
			}
			return pindexGenesisBlock;
		}

		uint1024 GetBlockHash()
		{
			// Find the first block the caller has in the main chain
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return hash;
				}
			}
			return hashGenesisBlock;
		}

		int GetHeight()
		{
			CBlockIndex* pindex = GetBlockIndex();
			if (!pindex)
				return 0;
			return pindex->nHeight;
		}
	};
}
