/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#include "include/blkpool.h"
#include "../include/manager.h"
#include "../include/checkpoints.h"
#include "../include/difficulty.h"
#include "../include/supply.h"

#include "../../LLD/include/index.h"

#include "../../LLP/include/legacy.h"

#include "../../Util/include/parse.h"

namespace Core
{		
	
	static void runCommand(std::string strCommand)
	{
		int nErr = ::system(strCommand.c_str());
		if (nErr)
			printf("runCommand error: system(%s) returned %d\n", strCommand.c_str(), nErr);
	}
	
	
	/** Verify the Signature is Valid for Last 2 Coinbase Tx Outputs. **/
	bool VerifyAddress(const std::vector<unsigned char> cScript, const unsigned char nSignature[])
	{
		if(cScript.size() != 37)
			return false;
				
		for(int nIndex = 0; nIndex < 37; nIndex++)
			if(cScript[nIndex] != nSignature[nIndex] )
				return false;
				
		return true;
	}
		
	/** Compare Two Vectors Element by Element. **/
	bool VerifyAddressList(const std::vector<unsigned char> cScript, const unsigned char SIGNATURES[][37])
	{
		for(int nSig = 0; nSig < 13; nSig++)
			if(VerifyAddress(cScript, SIGNATURES[nSig]))
				return true;
				
		return false;
	}
    
    
    /* Get Last block in a specific Channel. */
    CBlockState CBlkPool::GetLastBlock(uint1024 hashBlock, unsigned int nChannel) const
    {
        CBlockState blk;
        while(GetBlockState(hashBlock, blk) && blk.GetChannel() != nChannel)
            hashBlock = blk.hashPrevBlock;
        
        return blk;
    }
    
    
    /* Read a previous block from the LLD instance. */
    bool CBlkPool::GetBlockState(uint1024 hashBlock, CBlockState& blk)
    {
        /* Check for the Block from the Memory Pool. */
        if(Get(hashBlock, blk))
            return true;
        
        /* Check for the Block from the LLD. */
        if(blockdb.ReadBlockState(hashBlock, blk))
            return true;
        
        /* Block hasn't been seen if not in memory or on disk. */
        return false;
    }
    
	
	/* This function is responsible for running the basic process checks for (check and accept)
	 * In order to determine if the block is a valid block, but allow multiprocessing of the data.
	 */
	bool CBlkPool::Process(CBlock blk)
	{
		/* Timer object for runtime calculations. */
		Timer cTimer;
		cTimer.Reset();
		
		/* Get the block Hash. */
		uint1024 hash = blk.GetHash();
			
		/* Check the Block validity. */
		if(Check(blk))
		{

			/* Set the state to verified. */
			Update(hash, blk, CHECKED);
			
			
			if(GetArg("-verbose", 0) >= 3)
				printf("PASSED %s checks in %" PRIu64 " us\n", hash.ToString().substr(0, 20).c_str(), cTimer.ElapsedMicroseconds());
			
			Core::pManager->nProcessed ++;
		}
		else
		{
			if(GetArg("-verbose", 0) >= 3)
				return error("INVALID checks in %" PRIu64 " us\n", cTimer.ElapsedMicroseconds());
			
			/* Set the proper state for the new block. */
			Add(hash, blk, ERROR_CHECK);
				
			return false;
		}

		return true;
	}
	
	
	/** Check Block: These are Checks done before the Block is sunken in the Blockchain.
		These are done before a block is orphaned to ensure it is valid before trying to obtain its chain. **/
	bool CBlkPool::Check(CBlock blk)
	{
		
		/* Check the Size limits of the Current Block. */
		if (blk.vtx.empty() || blk.vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(blk, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
			return error("CBlkPool::Check() : size limits failed");
			
		
		/* Make sure the Block was Created within Active Channel. */
		if (blk.GetChannel() > 2)
			return error("CBlkPool::Check() : Channel out of Range.");
		
		
		/* Check that the Timestamp isn't made in the future. */
		if (blk.GetBlockTime() > UnifiedTimestamp() + MAX_UNIFIED_DRIFT)
			return error("CBlkPool::Check() : block timestamp too far in the future");
	
		
		/* Do not allow blocks to be accepted above the Current Block Version. */
		if(blk.nVersion > (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION))
			return error("CBlkPool::Check() : Invalid Block Version.");
	
		
		/* Only allow POS blocks in Version 4. */
		if(blk.IsProofOfStake() && blk.nVersion < 4)
			return error("CBlkPool::Check() : Proof of Stake Blocks Rejected until Version 4.");
			
		
		/* Check the Proof of Work Claims. */
		if (blk.IsProofOfWork() && !blk.VerifyWork())
			return error("CBlkPool::Check() : Invalid Proof of Work");

		
		/* Check the Network Launch Time-Lock. */
		if (blk.nHeight > 0 && blk.GetBlockTime() <= (fTestNet ? NEXUS_TESTNET_TIMELOCK : NEXUS_NETWORK_TIMELOCK))
			return error("CBlkPool::Check() : Block Created before Network Time-Lock");
			
		
		/* Check the Current Channel Time-Lock. */
		if (blk.nHeight > 0 && blk.GetBlockTime() < (fTestNet ? CHANNEL_TESTNET_TIMELOCK[blk.GetChannel()] : CHANNEL_NETWORK_TIMELOCK[blk.GetChannel()]))
			return error("CBlkPool::Check() : Block Created before Channel Time-Lock. Channel Opens in %" PRId64 " Seconds", (fTestNet ? CHANNEL_TESTNET_TIMELOCK[blk.GetChannel()] : CHANNEL_NETWORK_TIMELOCK[blk.GetChannel()]) - UnifiedTimestamp());
			
		
		/* Check the Previous Version to Block Time-Lock. Allow Version (Current -1) Blocks for 1 Hour after Time Lock. */
		if (blk.nVersion > 1 && blk.nVersion == (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION - 1 : NETWORK_BLOCK_CURRENT_VERSION - 1) && (blk.GetBlockTime() - 3600) > (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
			return error("CBlkPool::Check() : Version %u Blocks have been Obsolete for %" PRId64 " Seconds", blk.nVersion, (UnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2])));	
			
		
		/* Check the Current Version Block Time-Lock. */
		if (blk.nVersion >= (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION) && blk.GetBlockTime() <= (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
			return error("CBlkPool::Check() : Version %u Blocks are not Accepted for %" PRId64 " Seconds", blk.nVersion, (UnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2])));	
			
		
		/* Check the Required Mining Outputs (blocks version 3 and 4 only). */
		if (blk.nHeight > 0 && blk.IsProofOfWork() && blk.nVersion >= 3 && blk.nVersion < 5) {
			unsigned int nSize = blk.vtx[0].vout.size();

			
			/* Check the Coinbase Tx Size. */
			if(nSize < 3)
				return error("CBlkPool::Check() : Coinbase Too Small.");
				
			if(!fTestNet)
			{
				if (!VerifyAddressList(blk.vtx[0].vout[nSize - 2].scriptPubKey, AMBASSADOR_SCRIPT_SIGNATURES))
					return error("CBlkPool::Check() : Block %u Channel Signature Not Verified.", blk.nHeight);

				if (!VerifyAddressList(blk.vtx[0].vout[nSize - 1].scriptPubKey, DEVELOPER_SCRIPT_SIGNATURES))
					return error("CBlkPool::Check() :  Block %u Developer Signature Not Verified.", blk.nHeight);
			}
			
			else
			{
				if (!VerifyAddress(blk.vtx[0].vout[nSize - 2].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CBlkPool::Check() :  Block %u Channel Signature Not Verified.", blk.nHeight);

				if (!VerifyAddress(blk.vtx[0].vout[nSize - 1].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CBlkPool::Check() :  Block %u Developer Signature Not Verified.", blk.nHeight);
			}
		}

		
		/* Check the Coinbase Transaction is First, with no repetitions. */
		if (blk.vtx.empty() || (!blk.vtx[0].IsCoinBase() && blk.nChannel > 0))
			return error("CBlkPool::Check() : first tx is not coinbase for Proof of Work Block");
			
		
		/* Check the Coinstake Transaction is First, with no repetitions. */
		if (blk.vtx.empty() || (!blk.vtx[0].IsCoinStake() && blk.nChannel == 0))
			return error("CBlkPool::Check() : first tx is not coinstake for Proof of Stake Block");
			
			
		/* Check coinbase/coinstake timestamp is at most 20 minutes before block time */
		if (blk.GetBlockTime() > (int64)blk.vtx[0].nTime + ((blk.nVersion < 4) ? 1200 : 3600))
			return error("CBlkPool::Check() : coinbase/coinstake timestamp is too early");
		
		
		/* Check the Transactions in the Block. */
		std::set<uint512> uniqueTx; unsigned int nSigOps = 0;
		for (unsigned int i = 0; i < blk.vtx.size(); i++)
		{
			
			/* Check for duplicate Coinbase / Coinstake Transactions. */
			if (i > 0 && (blk.vtx[i].IsCoinBase() || blk.vtx[i].IsCoinStake()))
				return error("CBlkPool::Check() : more than one coinbase / coinstake");
				
			
			/* Verify Transaction Validity. */
			if (!blk.vtx[i].CheckTransaction())
				return error("CBlkPool::Check() : CheckTransaction failed");
				
			
			/* Transaction timestamp must be less than block timestamp. */
			if (blk.GetBlockTime() < (int64)blk.vtx[i].nTime)
				return error("CBlkPool::Check() : block timestamp earlier than transaction timestamp");
			
			
			/* Check for Duplicate txid's. */
			uniqueTx.insert(blk.vtx[i].GetHash());
			
			
			/* Calculate Signature Ops. */
			nSigOps += blk.vtx[i].GetLegacySigOpCount();
		}

		
		/* Reject Block if there are duplicate txid's. */
		if (uniqueTx.size() != blk.vtx.size())
			return error("CBlkPool::Check() : duplicate transaction");

		
		/* Check the signature operations are within bound. */
		if (nSigOps > MAX_BLOCK_SIGOPS)
			return error("CBlkPool::Check() : out-of-bounds SigOpCount");

		
		/* Check the Merkle Root builds as Advertised. */
		if (blk.hashMerkleRoot != blk.BuildMerkleTree())
			return error("CBlkPool::Check() : hashMerkleRoot mismatch");
		
		
		/* Check the Block Signature. */
		if (!blk.CheckBlockSignature())
			return error("CBlkPool::Check() : bad block signature");

		return true;
	}
	
	
	bool CBlkPool::Accept(CBlock blk)
	{
		
		/* Get the Previous block from the Pool. */
		CBlockState blkPrev;
		if(!GetBlockState(blk.hashPrevBlock, blkPrev))
			return error("CBlkPool::Accept() : prev block not found");
		
		/** Check the Height of Block to Previous Block. **/
		if(blkPrev.nHeight != blk.nHeight)
			return error("CBlkPool::Accept() : incorrect block height.");
		
		/* Check that the nBits match the current Difficulty. */
		if (blk.nBits != GetNextTargetRequired(blkPrev, blk.GetChannel()))
			return error("CBlkPool::Accept() : incorrect proof-of-work/proof-of-stake %s", blk.GetHash().ToString().substr(0, 20).c_str());
			
			
		/* TODO: CBlockState - Check that Block is Descendant of Hardened Checkpoints. **/
		//if(pindexPrev && !IsDescendant(pindexPrev))
		//	return error("CBlkPool::Accept() : Not a descendant of Last Checkpoint");

			
		/** Check That Block Timestamp is not before previous block. **/
		if (blk.GetBlockTime() <= blkPrev.GetBlockTime())
			return error("CBlkPool::Accept() : block's timestamp too early Block: % " PRId64 " Prev: %" PRId64 "", blk.GetBlockTime(), blkPool.GetBlockTime());
			
			
		/** Check the Coinbase Transactions in Block Version 3. **/
		if(blk.IsProofOfWork() && blk.nHeight > 0 && blk.nVersion >= 3)
		{
			unsigned int nSize = blk.vtx[0].vout.size();

			/** Add up the Miner Rewards from Coinbase Tx Outputs. **/
			int64 nMiningReward = 0;
			for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
				nMiningReward += blk.vtx[0].vout[nIndex].nValue;
					
			/** Check that the Mining Reward Matches the Coinbase Calculations. **/
			if (nMiningReward != GetCoinbaseReward(blkPrev, blk.GetChannel(), 0))
				return error("CBlkPool::Accept() : miner reward mismatch %" PRId64 " : %" PRId64 "", nMiningReward, GetCoinbaseReward(blkPrev, blk.GetChannel(), 0));
					
			/** Check that the Exchange Reward Matches the Coinbase Calculations. **/
			if (blk.vtx[0].vout[nSize - 2].nValue != GetCoinbaseReward(blkPrev, blk.GetChannel(), 1))
				return error("CBlkPool::Accept() : exchange reward mismatch %" PRId64 " : %" PRId64 "\n", blk.vtx[0].vout[1].nValue, GetCoinbaseReward(blkPrev, blk.GetChannel(), 1));
						
			/** Check that the Developer Reward Matches the Coinbase Calculations. **/
			if (blk.vtx[0].vout[nSize - 1].nValue != GetCoinbaseReward(blkPrev, blk.GetChannel(), 2))
				return error("CBlkPool::Accept() : developer reward mismatch %" PRId64 " : %" PRId64 "\n", blk.vtx[0].vout[2].nValue, GetCoinbaseReward(pindexPrev, blk.GetChannel(), 2));
					
		}
		
		
		/** Check the Proof of Stake Claims. **/
		else if (blk.IsProofOfStake())
		{
			if(!cTrustPool.Check(blk))
				return error("CBlkPool::Accept() : Invalid Trust Key");
			
			/** Verify the Stake Kernel. **/
			if(!blk.VerifyStake())
				return error("CBlkPool::Accept() : Invalid Proof of Stake");
		}
		
			
		/** Check that Transactions are Finalized. **/
		for(auto tx : blk.vtx)
			if (!tx.IsFinal(blk.nHeight, blk.GetBlockTime()))
				return error("CBlkPool::Accept() : contains a non-final transaction");
        
        /* Create the State object. */
        CBlockState blkState(blk);
        
        /* Compute the Chain Trust */
        blkState.nChainTrust = (blkState.hashPrevBlock == 0 ? blkPrev.nChainTrust : 0) + blkPrev.GetBlockTrust();
        
        
        /* Compute the Channel Height. */
        blkPrev = GetLastBlock(blkPrev, blkPrev.GetChannel();
        blkState.nChannelHeight = (blkPrev.IsNull() ? blkPrev.nChannelHeight : 0) + 1;
        
        
        /** Compute the Released Reserves. **/
        for(int nType = 0; nType < 3; nType++)
        {
            if(blkState.IsProofOfWork() && !blkPrev.IsNull())
            {
                /** Calculate the Reserves from the Previous Block in Channel's reserve and new Release. **/
                int64 nReserve  = blkPrev.nReleasedReserve[nType] + GetReleasedReserve(blkState, blkState.GetChannel(), nType);
                
                /** Block Version 3 Check. Disable Reserves from going below 0. **/
                if(blkState.nVersion >= 3 && blkState.nCoinbaseRewards[nType] >= nReserve)
                    return error("CBlkPool::Accept() : Coinbase Transaction too Large. Out of Reserve Limits");
                
                blkState.nReleasedReserve[nType] =  nReserve - pindexNew->nCoinbaseRewards[nType];
                
                if(GetArg("-verbose", 0) >= 3)
                    printf("Reserve Balance %i | %f Nexus | Released %f\n", nType, blkState.nReleasedReserve[nType] / 1000000.0, (nReserve - blkPrev.nReleasedReserve[nType]) / 1000000.0 );
            }
            else
                blkState.nReleasedReserve[nType] = 0;
            
        }
        
        /** TODO:: New checkpointing system
            Add the Pending Checkpoint into the Blockchain.
        if(!pindexNew->pprev || HardenCheckpoint(pindexNew))
        {
            pindexNew->PendingCheckpoint = std::make_pair(pindexNew->nHeight, pindexNew->GetBlockHash());
            
            if(GetArg("-verbose", 0) >= 3)
                printf("===== New Pending Checkpoint Hash = %s Height = %u\n", pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->nHeight);
        }
        else
        {
            pindexNew->PendingCheckpoint = pindexNew->pprev->PendingCheckpoint;
            
            unsigned int nAge = pindexNew->pprev->GetBlockTime() - mapBlockIndex[pindexNew->PendingCheckpoint.second]->GetBlockTime();
            
            if(GetArg("-verbose", 0) >= 3)
                printf("===== Pending Checkpoint Age = %u Hash = %s Height = %u\n", nAge, pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->PendingCheckpoint.first);
        }
         **/
				
		
		/* Set the best chain if is best block. */
		if (blkState.nChainTrust > nBestChainTrust)
		{

		}
                               
        /* Update the new state object into pool to be written. */
		Update(blkState.GetHash(), blkState, ACCEPTED);

                               
		return true;
	}
	

	bool CBlkPool::Connect(CBlockIndex* pindexNew)
	{
		uint1024 hash = pindexNew->GetBlockHash();
		if (pindexGenesisBlock == NULL && hash == hashGenesisBlock)
		{
			indexdb.WriteHashBestChain(hash);
			pindexGenesisBlock = pindexNew;
		}
		else
		{
			CBlockIndex* pfork = pindexBest;
			CBlockIndex* plonger = pindexNew;
			while (pfork != plonger)
			{
				while (plonger->nHeight > pfork->nHeight)
					if (!(plonger = plonger->pprev))
						return error("CBlkPool::Connect() : plonger->pprev is null");
				if (pfork == plonger)
					break;
				if (!(pfork = pfork->pprev))
					return error("CBlkPool::Connect : pfork->pprev is null");
			}

			
			/* List of what to Disconnect. */
			std::vector<CBlockIndex*> vDisconnect;
			for (CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
				vDisconnect.push_back(pindex);

			
			/* List of what to Connect. */
			std::vector<CBlockIndex*> vConnect;
			for (CBlockIndex* pindex = pindexNew; pindex != pfork; pindex = pindex->pprev)
				vConnect.push_back(pindex);
			reverse(vConnect.begin(), vConnect.end());

			
			/* Debug output if there is a fork. */
			if(vDisconnect.size() > 0 && GetArg("-verbose", 0) >= 1)
			{
				printf("REORGANIZE: Disconnect %i blocks; %s..%s\n", vDisconnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexBest->GetBlockHash().ToString().substr(0,20).c_str());
				printf("REORGANIZE: Connect %i blocks; %s..%s\n", vConnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->GetBlockHash().ToString().substr(0,20).c_str());
			}
			
			
			/* Disconnect the Shorter Branch. */
			std::vector<CTransaction> vResurrect;
			for(auto pindex : vDisconnect)
			{
				CBlock block;
				if (!block.ReadFromDisk(pindex))
					return error("CBlkPool::Connect() : ReadFromDisk for disconnect failed");
				if (!block.DisconnectBlock(indexdb, pindex))
					return error("CBlkPool::Connect() : DisconnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());
					
				
				/* Remove Transactions from Current Trust Keys */
				if(block.IsProofOfStake() && !cTrustPool.Remove(block))
					return error("CBlkPool::Connect() : Disconnect Failed to Remove Trust Key at Block %s", pindex->GetBlockHash().ToString().substr(0,20).c_str());
				
				
				/* Set the block's state in blkPool. */
				pManager->blkPool.SetState(pindex->GetBlockHash(), pManager->blkPool.INDEXED);
				
				
				/* Resurrect Memory Pool Transactions. */
				for(auto tx : block.vtx)
					if (!(tx.IsCoinBase() || tx.IsCoinStake()))
						vResurrect.push_back(tx);
			}


			
			/* Connect the Longer Branch. */
			std::vector<CTransaction> vDelete;
			for (unsigned int i = 0; i < vConnect.size(); i++)
			{
				CBlockIndex* pindex = vConnect[i];
				CBlock block;
				if (!block.ReadFromDisk(pindex))
					return error("CBlkPool::Connect() : ReadFromDisk for connect failed");
				
				
				if (!block.ConnectBlock(indexdb, pindex))
					return error("CBlkPool::Connect() : ConnectBlock %s Height %u failed", pindex->GetBlockHash().ToString().substr(0,20).c_str(), pindex->nHeight);
				
				
				/* Harden a pending checkpoint if this is the case. */
				if(pindex->pprev && IsNewTimespan(pindex))
					HardenCheckpoint(pindex);
				
				
				/* Add Transaction to Current Trust Keys */
				if(block.IsProofOfStake() && !cTrustPool.Accept(block))
					return error("CBlkPool::Connect() : Failed To Accept Trust Key Block.");
				
				
				/* Set the block's state in blkPool. */
				pManager->blkPool.SetState(pindex->GetBlockHash(), pManager->blkPool.CONNECTED);

				
				/* Delete Memory Pool Transactions contained already. **/
				for(auto tx : block.vtx)
					if (!(tx.IsCoinBase() || tx.IsCoinStake()))
						vDelete.push_back(tx);
			}
			
			
			/* Write the Best Chain to the Index Database LLD. */
			if (!indexdb.WriteHashBestChain(pindexNew->GetBlockHash()))
				return error("CBlkPool::Connect() : WriteHashBestChain failed");
			
			
			/* Disconnect Shorter Branch in Memory. */
			for(auto pindex : vDisconnect)
				if (pindex->pprev)
					pindex->pprev->pnext = NULL;


			/* Conenct the Longer Branch in Memory. */
			for(auto pindex : vConnect)
				if (pindex->pprev)
					pindex->pprev->pnext = pindex;


			for(auto tx : vResurrect)
				pManager->txPool.SetState(tx.GetHash(), pManager->txPool.ACCEPTED);

			for(auto tx : vDelete)
				pManager->txPool.SetState(tx.GetHash(), pManager->txPool.CONNECTED);
			
		}

		
		/* Establish the Best Variables for the Height of the Block-chain. */
		hashBestChain = hash;
		pindexBest = pindexNew;
		nBestHeight = pindexBest->nHeight;
		nBestChainTrust = pindexNew->nChainTrust;
		nTimeBestReceived = UnifiedTimestamp();
		
		
		if(GetArg("-verbose", 0) >= 2)
			printf("CBlkPool::Connect() : best=%s  height=%d  trust=%" PRIu64 "  moneysupply=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust, FormatMoney(pindexBest->nMoneySupply).c_str());

		
		/*
		std::string strCmd = GetArg("-blocknotify", "");
		if (!IsInitialBlockDownload() && !strCmd.empty())
		{
			boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
			boost::thread t(runCommand, strCmd);
		}
		*/

		return true;
	}
	

	
}
