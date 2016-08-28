#include "index.h"
#include "core.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
	using namespace std;
	
	bool CIndexDB::ReadTxIndex(uint512 hash, Core::CTxIndex& txindex)
	{
		txindex.SetNull();
		return Read(make_pair(string("tx"), hash), txindex);
	}

	bool CIndexDB::UpdateTxIndex(uint512 hash, const Core::CTxIndex& txindex)
	{
		return Write(make_pair(string("tx"), hash), txindex);
	}
	
	bool CIndexDB::EraseTxIndex(const Core::CTransaction& tx)
	{
		assert(!Net::fClient);
		uint512 hash = tx.GetHash();

		return Erase(make_pair(string("tx"), hash));
	}

	bool CIndexDB::AddTxIndex(const Core::CTransaction& tx, const Core::CDiskTxPos& pos, int nHeight)
	{
		// Add to tx index
		uint512 hash = tx.GetHash();
		Core::CTxIndex txindex(pos, tx.vout.size());
		return Write(make_pair(string("tx"), hash), txindex);
	}

	bool CIndexDB::ContainsTx(uint512 hash)
	{
		assert(!Net::fClient);
		return Exists(make_pair(string("tx"), hash));
	}

	bool CIndexDB::ReadDiskTx(uint512 hash, Core::CTransaction& tx, Core::CTxIndex& txindex)
	{
		assert(!Net::fClient);
		tx.SetNull();
		if (!ReadTxIndex(hash, txindex))
			return false;
		return (tx.ReadFromDisk(txindex.pos));
	}

	bool CIndexDB::ReadDiskTx(uint512 hash, Core::CTransaction& tx)
	{
		Core::CTxIndex txindex;
		return ReadDiskTx(hash, tx, txindex);
	}

	bool CIndexDB::ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx, Core::CTxIndex& txindex)
	{
		return ReadDiskTx(outpoint.hash, tx, txindex);
	}

	bool CIndexDB::ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx)
	{
		Core::CTxIndex txindex;
		return ReadDiskTx(outpoint.hash, tx, txindex);
	}

	bool CIndexDB::WriteBlockIndex(const Core::CDiskBlockIndex& blockindex)
	{
		return Write(make_pair(string("blockindex"), blockindex.GetBlockHash()), blockindex);
	}

	bool CIndexDB::ReadHashBestChain(uint1024& hashBestChain)
	{
		return Read(string("hashBestChain"), hashBestChain);
	}

	bool CIndexDB::WriteHashBestChain(uint1024 hashBestChain)
	{
		return Write(string("hashBestChain"), hashBestChain);
	}
	
	bool CIndexDB::WriteTrustKey(uint512 hashTrustKey, Core::CTrustKey cTrustKey)
	{
		return Write(make_pair(string("trustKey"), hashTrustKey), cTrustKey);
	}
	
	bool CIndexDB::ReadTrustKey(uint512 hashTrustKey, Core::CTrustKey& cTrustKey)
	{
		return Read(make_pair(string("trustKey"), hashTrustKey), cTrustKey);
	}
	
	bool CIndexDB::AddTrustBlock(uint512 hashTrustKey, uint1024 hashTrustBlock)
	{
		return Write(make_pair(string("trustBlock"), hashTrustBlock), hashTrustKey);
	}
	
	bool CIndexDB::RemoveTrustBlock(uint1024 hashTrustBlock)
	{
		return Erase(make_pair(string("trustBlock"), hashTrustBlock));
	}

	Core::CBlockIndex static * InsertBlockIndex(uint1024 hash)
	{
		if (hash == 0)
			return NULL;

		// Return existing
		map<uint1024, Core::CBlockIndex*>::iterator mi = Core::mapBlockIndex.find(hash);
		if (mi != Core::mapBlockIndex.end())
			return (*mi).second;

		// Create new
		Core::CBlockIndex* pindexNew = new Core::CBlockIndex();
		if (!pindexNew)
			throw runtime_error("LoadBlockIndex() : new Core::CBlockIndex failed");
		
		mi = Core::mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
		pindexNew->phashBlock = &((*mi).first);

		return pindexNew;
	}

	bool CIndexDB::LoadBlockIndex()
	{
		Core::hashBestChain;
		if(!ReadHashBestChain(Core::hashBestChain))
			return error("No Hash Best Chain in Index Database.");
		
		uint1024 hashBlock = Core::hashGenesisBlock;
		while(!fRequestShutdown)
		{
			Core::CDiskBlockIndex diskindex;
			if(!Read(make_pair(string("blockindex"), hashBlock), diskindex))
			{
				printf("Failed to Read Block %s Height %u\n", hashBlock.ToString().substr(0, 20).c_str(), diskindex.nHeight);
				
				Core::mapBlockIndex.erase(hashBlock);
				break;
			}
			
			
			Core::CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
			pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
			pindexNew->pnext          = InsertBlockIndex(diskindex.hashNext);
			pindexNew->nFile          = diskindex.nFile;
			pindexNew->nBlockPos      = diskindex.nBlockPos;
			pindexNew->nMint          = diskindex.nMint;
			pindexNew->nMoneySupply   = diskindex.nMoneySupply;
			pindexNew->nChannelHeight = diskindex.nChannelHeight;
			pindexNew->nChainTrust    = diskindex.nChainTrust;
			
			/** Handle the Reserves. **/
			pindexNew->nCoinbaseRewards[0] = diskindex.nCoinbaseRewards[0];
			pindexNew->nCoinbaseRewards[1] = diskindex.nCoinbaseRewards[1];
			pindexNew->nCoinbaseRewards[2] = diskindex.nCoinbaseRewards[2];
			pindexNew->nReleasedReserve[0] = diskindex.nReleasedReserve[0];
			pindexNew->nReleasedReserve[1] = diskindex.nReleasedReserve[1];
			pindexNew->nReleasedReserve[2] = diskindex.nReleasedReserve[2];
				
			/** Handle the Block Headers. **/
			pindexNew->nVersion       = diskindex.nVersion;
			pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
			pindexNew->nChannel       = diskindex.nChannel;
			pindexNew->nHeight        = diskindex.nHeight;
			pindexNew->nBits          = diskindex.nBits;
			pindexNew->nNonce         = diskindex.nNonce;
			pindexNew->nTime          = diskindex.nTime;
			
			/** Add the Pending Checkpoint into the Blockchain. **/
			if(!pindexNew->pprev || Core::HardenCheckpoint(pindexNew, true))
				pindexNew->PendingCheckpoint = make_pair(pindexNew->nHeight, pindexNew->GetBlockHash());
			else
				pindexNew->PendingCheckpoint = pindexNew->pprev->PendingCheckpoint;
				
			/** Accept the Trust Keys into the Trust Pool. **/
			if(pindexNew->IsProofOfStake()) {
				Core::CBlock block;
				if (!block.ReadFromDisk(pindexNew))
					break;
					
				if(!Core::cTrustPool.Accept(block, true))
					return error("CIndexDB::LoadBlockIndex() : Failed To Accept Trust Key Block.");
			}
			
			if(hashBlock == Core::hashGenesisBlock)
				Core::pindexGenesisBlock = pindexNew;
				
			if(hashBlock == Core::hashBestChain) {
				break;
			}
			
			Core::pindexBest  = pindexNew;
			hashBlock = diskindex.hashNext;
		}
		
		Core::nBestHeight = Core::pindexBest->nHeight;
		Core::nBestChainTrust = Core::pindexBest->nChainTrust;
		Core::hashBestChain   = Core::pindexBest->GetBlockHash();
		
		printf("[DATABASE] Indexes Loaded. Height %u Hash %s\n", Core::nBestHeight, Core::pindexBest->GetBlockHash().ToString().substr(0, 20).c_str());
		
		/** Double Check for Future Forked Blocks. **/
		while(Core::pindexBest->pnext) {
			Core::mapBlockIndex.erase(Core::pindexBest->pnext->GetBlockHash());
			//Erase(make_pair(string("blockindex"), Core::pindexBest->pnext->GetBlockHash()));
			
			printf("[DATABASE] Destorying Pointer for %s\n", Core::pindexBest->pnext->GetBlockHash().ToString().substr(0, 20).c_str());
			Core::pindexBest = Core::pindexBest->pnext;
		}
		
		Core::pindexBest = Core::mapBlockIndex[Core::hashBestChain];
		Core::pindexBest->pnext = NULL;

		/** Verify the Blocks in the Best Chain To Last Checkpoint. **/
		int nCheckLevel = GetArg("-checklevel", 5);
		int nCheckDepth = GetArg( "-checkblocks", 100);
		//if (nCheckDepth == 0)
		//	nCheckDepth = 1000000000;
			
		if (nCheckDepth > Core::nBestHeight)
			nCheckDepth = Core::nBestHeight;
		printf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
		Core::CBlockIndex* pindexFork = NULL;
		
		
		map<pair<unsigned int, unsigned int>, Core::CBlockIndex*> mapBlockPos;
		for (Core::CBlockIndex* pindex = Core::pindexBest; pindex && pindex->pprev && nCheckDepth > 0; pindex = pindex->pprev)
		{
			if (pindex->nHeight < Core::nBestHeight - nCheckDepth)
				break;
				
			Core::CBlock block;
			if (!block.ReadFromDisk(pindex))
				return error("LoadBlockIndex() : block.ReadFromDisk failed");
				
			if (nCheckLevel > 0 && !block.CheckBlock())
			{
				printf("LoadBlockIndex() : *** found bad block at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str());
				pindexFork = pindex->pprev;
			}
			
			// check level 2: verify transaction index validity
			if (nCheckLevel>1)
			{
				pair<unsigned int, unsigned int> pos = make_pair(pindex->nFile, pindex->nBlockPos);
				mapBlockPos[pos] = pindex;
				BOOST_FOREACH(const Core::CTransaction &tx, block.vtx)
				{
					uint512 hashTx = tx.GetHash(); 
					Core::CTxIndex txindex;
					if (ReadTxIndex(hashTx, txindex))
					{
					
						// check level 3: checker transaction hashes
						if (nCheckLevel>2 || pindex->nFile != txindex.pos.nFile || pindex->nBlockPos != txindex.pos.nBlockPos)
						{
							// either an error or a duplicate transaction
							Core::CTransaction txFound;
							if (!txFound.ReadFromDisk(txindex.pos))
							{
								printf("LoadBlockIndex() : *** cannot read mislocated transaction %s\n", hashTx.ToString().c_str());
								pindexFork = pindex->pprev;
							}
							else
								if (txFound.GetHash() != hashTx) // not a duplicate tx
								{
									printf("LoadBlockIndex(): *** invalid tx position for %s\n", hashTx.ToString().c_str());
									pindexFork = pindex->pprev;
								}
						}
						// check level 4: check whether spent txouts were spent within the main chain
						unsigned int nOutput = 0;
						if (nCheckLevel>3)
						{
							BOOST_FOREACH(const Core::CDiskTxPos &txpos, txindex.vSpent)
							{
								if (!txpos.IsNull())
								{
									pair<unsigned int, unsigned int> posFind = make_pair(txpos.nFile, txpos.nBlockPos);
									if (!mapBlockPos.count(posFind))
									{
										printf("LoadBlockIndex(): *** found bad spend at %d, hashBlock=%s, hashTx=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str(), hashTx.ToString().c_str());
										pindexFork = pindex->pprev;
									}
									// check level 6: check whether spent txouts were spent by a valid transaction that consume them
									if (nCheckLevel>5)
									{
										Core::CTransaction txSpend;
										if (!txSpend.ReadFromDisk(txpos))
										{
											printf("LoadBlockIndex(): *** cannot read spending transaction of %s:%i from disk\n", hashTx.ToString().c_str(), nOutput);
											pindexFork = pindex->pprev;
										}
										else if (!txSpend.CheckTransaction())
										{
											printf("LoadBlockIndex(): *** spending transaction of %s:%i is invalid\n", hashTx.ToString().c_str(), nOutput);
											pindexFork = pindex->pprev;
										}
										else
										{
											bool fFound = false;
											BOOST_FOREACH(const Core::CTxIn &txin, txSpend.vin)
												if (txin.prevout.hash == hashTx && txin.prevout.n == nOutput)
													fFound = true;
											if (!fFound)
											{
												printf("LoadBlockIndex(): *** spending transaction of %s:%i does not spend it\n", hashTx.ToString().c_str(), nOutput);
												pindexFork = pindex->pprev;
											}
										}
									}
								}
								nOutput++;
							}
						}
					}
					// check level 5: check whether all prevouts are marked spent
					if (nCheckLevel>4)
					{
						 BOOST_FOREACH(const Core::CTxIn &txin, tx.vin)
						 {
							  Core::CTxIndex txindex;
							  if (ReadTxIndex(txin.prevout.hash, txindex))
								  if (txindex.vSpent.size()-1 < txin.prevout.n || txindex.vSpent[txin.prevout.n].IsNull())
								  {
									  printf("LoadBlockIndex(): *** found unspent prevout %s:%i in %s\n", txin.prevout.hash.ToString().c_str(), txin.prevout.n, hashTx.ToString().c_str());
									  pindexFork = pindex->pprev;
								  }
						 }
					}
				}
			}
		}
		if (pindexFork)
		{
			// Reorg back to the fork
			printf("LoadBlockIndex() : *** moving best chain pointer back to block %d\n", pindexFork->nHeight);
			Core::CBlock block;
			if (!block.ReadFromDisk(pindexFork))
				return error("LoadBlockIndex() : block.ReadFromDisk failed");
			CIndexDB txdb;
			block.SetBestChain(txdb, pindexFork);
		}

		return true;
	}
}