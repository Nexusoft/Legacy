/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_BLOCK_H
#define NEXUS_CORE_BLOCK_H

namespace Core
{
	
	/* Find the Nearest Orphan Block down the Chain. */
	uint1024 GetOrphanRoot(const CBlock* pblock);
	
	
	/* Find the blocks that are required to complete an orphan chain. */
	uint1024 WantedByOrphan(const CBlock* pblockOrphan);
	
	
	/* Get Coinbase Reweards for Given Blocks. */
	int64 GetProofOfWorkReward(unsigned int nBits);
	
	
	/* Get the Stake Reward for Given Blocks. */
	int64 GetProofOfStakeReward(int64 nCoinAge);
	
	
	/* Search back for an index given PoW / PoS parameters. */
	const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
	
	
	/* Search back for an index of given Mining Channel. */
	const CBlockIndex* GetLastChannelIndex(const CBlockIndex* pindex, int nChannel);
	
	
	/* Calculate the majority of blocks that other peers have. */
	int GetNumBlocksOfPeers();
	
	
	/* Determine if the node is syncing from scratch. */
	bool IsInitialBlockDownload();
	
	
	/* Accept a block into the block chain without setting it as the leading block. */
	bool AcceptBlock(LLP::CNode* pfrom, CBlock* pblock);
	
	
	/* Add a block into index memory and give it a location in the chain. */
	bool AddToBlockIndex(LLP::CNode* pfrom, CBlock* pblock);
	
	
	/* Set block as the current leading block of the block chain. */
	bool SetBestChain(LLP::CNode* pfrom, CBlock* pblock);
	
	
	/* Check the disk space for the current partition database is stored in. */
	bool CheckDiskSpace(uint64 nAdditionalBytes = 0);
	
	
	/* Read the block from file and binary postiion (blk0001.dat), */
	FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode);
	
	
	/* Add a block to the block file and binary position (blk0001.dat). */
	FILE* AppendBlockFile(unsigned int& nFileRet);
	
	
	/* Load the Genesis and other blocks from the BDB/LLD Indexes. */
	bool LoadBlockIndex(bool fAllowNew = true);
	
	
	/* Ensure that a disk read block index is valid still (prevents needs for checks in data. */
	bool CheckBlockIndex(uint1024 hashBlock);
	
	
	/* Initialize the Mining LLP to start creating work and accepting blocks to broadcast to the network. */
	void StartMiningLLP();
	
	
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
