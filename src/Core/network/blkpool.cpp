/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/blkpool.h"
#include "../include/manager.h"
#include "../include/checkpoints.h"
#include "../include/difficulty.h"
#include "../include/supply.h"

#include "../../LLD/include/index.h"

#include "../../LLP/include/node.h"

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
			return error("Script Size not 37 Bytes");
				
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
	
	/** Check Block: These are Checks done before the Block is sunken in the Blockchain.
		These are done before a block is orphaned to ensure it is valid before trying to obtain its chain. **/
	bool CBlkPool::Check(CBlock blk, LLP::CNode* pfrom)
	{
		
		/* Check the Size limits of the Current Block. */
		if (blk.vtx.empty() || blk.vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(blk, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
			return LLP::DoS(pfrom, 100, error("CheckBlock() : size limits failed"));
			
		
		/* Make sure the Block was Created within Active Channel. */
		if (blk.GetChannel() > 2)
			return LLP::DoS(pfrom, 50, error("CheckBlock() : Channel out of Range."));
		
		
		/* Check that the Timestamp isn't made in the future. */
		if (blk.GetBlockTime() > UnifiedTimestamp() + MAX_UNIFIED_DRIFT)
			return error("AcceptBlock() : block timestamp too far in the future");
	
		
		/* Do not allow blocks to be accepted above the Current Block Version. */
		if(blk.nVersion > (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION))
			return LLP::DoS(pfrom, 50, error("CheckBlock() : Invalid Block Version."));
	
		
		/* Only allow POS blocks in Version 4. */
		if(blk.IsProofOfStake() && blk.nVersion < 4)
			return LLP::DoS(pfrom, 50, error("CheckBlock() : Proof of Stake Blocks Rejected until Version 4."));
			
		
		/* Check the Proof of Work Claims. */
		if (blk.IsProofOfWork() && !blk.VerifyWork())
			return LLP::DoS(pfrom, 50, error("CheckBlock() : Invalid Proof of Work"));

		
		/* Check the Network Launch Time-Lock. */
		if (blk.nHeight > 0 && blk.GetBlockTime() <= (fTestNet ? NEXUS_TESTNET_TIMELOCK : NEXUS_NETWORK_TIMELOCK))
			return LLP::DoS(pfrom, 50, ("CheckBlock() : Block Created before Network Time-Lock"));
			
		
		/* Check the Current Channel Time-Lock. */
		if (blk.nHeight > 0 && blk.GetBlockTime() < (fTestNet ? CHANNEL_TESTNET_TIMELOCK[blk.GetChannel()] : CHANNEL_NETWORK_TIMELOCK[blk.GetChannel()]))
			return error("CheckBlock() : Block Created before Channel Time-Lock. Channel Opens in %" PRId64 " Seconds", (fTestNet ? CHANNEL_TESTNET_TIMELOCK[blk.GetChannel()] : CHANNEL_NETWORK_TIMELOCK[blk.GetChannel()]) - UnifiedTimestamp());
			
		
		/* Check the Previous Version to Block Time-Lock. Allow Version (Current -1) Blocks for 1 Hour after Time Lock. */
		if (blk.nVersion > 1 && blk.nVersion == (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION - 1 : NETWORK_BLOCK_CURRENT_VERSION - 1) && (blk.GetBlockTime() - 3600) > (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
			return error("CheckBlock() : Version %u Blocks have been Obsolete for %" PRId64 " Seconds\n", blk.nVersion, (UnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2])));	
			
		
		/* Check the Current Version Block Time-Lock. */
		if (blk.nVersion >= (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION) && blk.GetBlockTime() <= (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
			return error("CheckBlock() : Version %u Blocks are not Accepted for %" PRId64 " Seconds\n", blk.nVersion, (UnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2])));	
			
		
		/* Check the Required Mining Outputs. */
		if (blk.nHeight > 0 && blk.IsProofOfWork() && blk.nVersion < 5) {
			unsigned int nSize = blk.vtx[0].vout.size();

			/* Check the Coinbase Tx Size. */
			if(nSize < 3)
				return error("CheckBlock() : Coinbase Too Small.");
				
			if(!fTestNet)
			{
				if (!VerifyAddressList(blk.vtx[0].vout[nSize - 2].scriptPubKey, AMBASSADOR_SCRIPT_SIGNATURES))
					return error("CheckBlock() : Block %u Channel Signature Not Verified.\n", blk.nHeight);

				if (!VerifyAddressList(blk.vtx[0].vout[nSize - 1].scriptPubKey, DEVELOPER_SCRIPT_SIGNATURES))
					return error("CheckBlock() :  Block %u Developer Signature Not Verified.\n", blk.nHeight);
			}
			
			else
			{
				if (!VerifyAddress(blk.vtx[0].vout[nSize - 2].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CheckBlock() :  Block %u Channel Signature Not Verified.\n", blk.nHeight);

				if (!VerifyAddress(blk.vtx[0].vout[nSize - 1].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CheckBlock() :  Block %u Developer Signature Not Verified.\n", blk.nHeight);
			}
		}

		
		/* Check the Coinbase Transaction is First, with no repetitions. */
		if (blk.vtx.empty() || (!blk.vtx[0].IsCoinBase() && blk.nChannel > 0))
			return LLP::DoS(pfrom, 100, error("CheckBlock() : first tx is not coinbase for Proof of Work Block"));
			
		
		/* Check the Coinstake Transaction is First, with no repetitions. */
		if (blk.vtx.empty() || (!blk.vtx[0].IsCoinStake() && blk.nChannel == 0))
			return LLP::DoS(pfrom, 100, error("CheckBlock() : first tx is not coinstake for Proof of Stake Block"));
			
			
		/* Check coinbase/coinstake timestamp is at most 20 minutes before block time */
		if (blk.GetBlockTime() > (int64)blk.vtx[0].nTime + ((blk.nVersion < 4) ? 1200 : 3600))
			return LLP::DoS(pfrom, 50, error("CheckBlock() : coinbase/coinstake timestamp is too early"));
		
		
		/* Check the Transactions in the Block. */
		std::set<uint512> uniqueTx; unsigned int nSigOps = 0;
		for (unsigned int i = 0; i < blk.vtx.size(); i++)
		{
			
			/* Check for duplicate Coinbase / Coinstake Transactions. */
			if (i > 0 && (blk.vtx[i].IsCoinBase() || blk.vtx[i].IsCoinStake()))
				return LLP::DoS(pfrom, 100, error("CheckBlock() : more than one coinbase / coinstake"));
				
			
			/* Verify Transaction Validity. */
			if (!blk.vtx[i].CheckTransaction())
				return LLP::DoS(pfrom, 50, error("CheckBlock() : CheckTransaction failed"));
				
			
			/* Transaction timestamp must be less than block timestamp. */
			if (blk.GetBlockTime() < (int64)blk.vtx[i].nTime)
				return LLP::DoS(pfrom, 50, error("CheckBlock() : block timestamp earlier than transaction timestamp"));
			
			
			/* Check for Duplicate txid's. */
			uniqueTx.insert(blk.vtx[i].GetHash());
			
			
			/* Calculate Signature Ops. */
			nSigOps += blk.vtx[i].GetLegacySigOpCount();
		}

		
		/* Reject Block if there are duplicate txid's. */
		if (uniqueTx.size() != blk.vtx.size())
			return LLP::DoS(pfrom, 100, error("CheckBlock() : duplicate transaction"));

		
		/* Check the signature operations are within bound. */
		if (nSigOps > MAX_BLOCK_SIGOPS)
			return LLP::DoS(pfrom, 100, error("CheckBlock() : out-of-bounds SigOpCount"));

		
		/* Check the Merkle Root builds as Advertised. */
		if (blk.hashMerkleRoot != blk.BuildMerkleTree())
			return LLP::DoS(pfrom, 100, error("CheckBlock() : hashMerkleRoot mismatch"));
		
		
		/* Check the Block Signature. */
		if (!blk.CheckBlockSignature())
			return LLP::DoS(pfrom, 100, error("CheckBlock() : bad block signature"));

		return true;
	}
	
	
	bool CBlkPool::Accept(CBlock blk, LLP::CNode* pfrom)
	{
		
		/** Check for Duplicate Block. **/
		uint1024 hash = blk.GetHash();
		if (mapBlockIndex.count(hash))
			return error("AcceptBlock() : block already in mapBlockIndex");

			
		/** Find the Previous block from hashPrevBlock. **/
		std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(blk.hashPrevBlock);
		if (mi == mapBlockIndex.end())
			return LLP::DoS(pfrom, 10, error("AcceptBlock() : prev block not found"));
		
		CBlockIndex* pindexPrev = (*mi).second;
		int nPrevHeight = pindexPrev->nHeight + 1;
		
		
		/** Check the Height of Block to Previous Block. **/
		if(nPrevHeight != blk.nHeight)
			return LLP::DoS(pfrom, 100, error("AcceptBlock() : incorrect block height."));
		

		/** Check that the nBits match the current Difficulty. **/
		if (blk.nBits != GetNextTargetRequired(pindexPrev, blk.GetChannel(), !IsInitialBlockDownload()))
			return LLP::DoS(pfrom, 100, error("AcceptBlock() : incorrect proof-of-work/proof-of-stake"));
			
			
		/** Check that Block is Descendant of Hardened Checkpoints. **/
		if(pindexPrev && !IsDescendant(pindexPrev))
			return error("AcceptBlock() : Not a descendant of Last Checkpoint");

			
		/** Check That Block Timestamp is not before previous block. **/
		if (blk.GetBlockTime() <= pindexPrev->GetBlockTime())
			return error("AcceptBlock() : block's timestamp too early Block: % " PRId64 " Prev: %" PRId64 "", blk.GetBlockTime(), pindexPrev->GetBlockTime());
			
			
		/** Check the Coinbase Transactions in Block Version 3. **/
		if(blk.IsProofOfWork() && blk.nHeight > 0 && blk.nVersion >= 3)
		{
			unsigned int nSize = blk.vtx[0].vout.size();

			/** Add up the Miner Rewards from Coinbase Tx Outputs. **/
			int64 nMiningReward = 0;
			for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
				nMiningReward += blk.vtx[0].vout[nIndex].nValue;
					
			/** Check that the Mining Reward Matches the Coinbase Calculations. **/
			if (nMiningReward != GetCoinbaseReward(pindexPrev, blk.GetChannel(), 0))
				return error("AcceptBlock() : miner reward mismatch %" PRId64 " : %" PRId64 "", nMiningReward, GetCoinbaseReward(pindexPrev, blk.GetChannel(), 0));
					
			/** Check that the Exchange Reward Matches the Coinbase Calculations. **/
			if (blk.vtx[0].vout[nSize - 2].nValue != GetCoinbaseReward(pindexPrev, blk.GetChannel(), 1))
				return error("AcceptBlock() : exchange reward mismatch %" PRId64 " : %" PRId64 "\n", blk.vtx[0].vout[1].nValue, GetCoinbaseReward(pindexPrev, blk.GetChannel(), 1));
						
			/** Check that the Developer Reward Matches the Coinbase Calculations. **/
			if (blk.vtx[0].vout[nSize - 1].nValue != GetCoinbaseReward(pindexPrev, blk.GetChannel(), 2))
				return error("AcceptBlock() : developer reward mismatch %" PRId64 " : %" PRId64 "\n", blk.vtx[0].vout[2].nValue, GetCoinbaseReward(pindexPrev, blk.GetChannel(), 2));
					
		}
		
		
		/** Check the Proof of Stake Claims. **/
		else if (blk.IsProofOfStake())
		{
			if(!cTrustPool.Check(blk))
				return LLP::DoS(pfrom, 50, error("AcceptBlock() : Invalid Trust Key"));
			
			/** Verify the Stake Kernel. **/
			if(!blk.VerifyStake())
				return LLP::DoS(pfrom, 50, error("AcceptBlock() : Invalid Proof of Stake"));
		}
		
			
		/** Check that Transactions are Finalized. **/
		for(auto tx : blk.vtx)
			if (!tx.IsFinal(blk.nHeight, blk.GetBlockTime()))
				return LLP::DoS(pfrom, 10, error("AcceptBlock() : contains a non-final transaction"));

				
		/** Write new Block to Disk. **/
		if (!CheckDiskSpace(::GetSerializeSize(blk, SER_DISK, DATABASE_VERSION)))
			return error("AcceptBlock() : out of disk space");
			
			
		unsigned int nFile = -1;
		unsigned int nBlockPos = 0;
		if (!blk.WriteToDisk(nFile, nBlockPos))
			return error("AcceptBlock() : WriteToDisk failed");
		
		if (!AddToChain(blk, nFile, nBlockPos, pfrom))
			return error("AcceptBlock() : AddToChain failed");
		

		return true;
	}


	/* AddToBlockIndex: Adds a new Block into the Block Index. 
		This is where it is categorized and dealt with in the Blockchain. */
	bool CBlkPool::AddToChain(CBlock blk, unsigned int nFile, unsigned int nBlockPos, LLP::CNode* pfrom)
	{
		/* Check for Duplicate. */
		uint1024 hash = blk.GetHash();
		if (mapBlockIndex.count(hash))
			return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());

			
		/* Build new Block Index Object. */
		CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, blk);
		if (!pindexNew)
			return error("AddToBlockIndex() : new CBlockIndex failed");

			
		/* Find Previous Block. */
		pindexNew->phashBlock = &hash;
		std::map<uint1024, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(blk.hashPrevBlock);
		if (miPrev != mapBlockIndex.end())
			pindexNew->pprev = (*miPrev).second;
		
		
		/* Compute the Chain Trust */
		pindexNew->nChainTrust = (pindexNew->pprev ? pindexNew->pprev->nChainTrust : 0) + pindexNew->GetBlockTrust();
		
		
		/* Compute the Channel Height. */
		const CBlockIndex* pindexPrev = GetLastChannelIndex(pindexNew->pprev, pindexNew->GetChannel());
		pindexNew->nChannelHeight = (pindexPrev ? pindexPrev->nChannelHeight : 0) + 1;
		
		
		/** Compute the Released Reserves. **/
		for(int nType = 0; nType < 3; nType++)
		{
			if(pindexNew->IsProofOfWork() && pindexPrev)
			{
				/** Calculate the Reserves from the Previous Block in Channel's reserve and new Release. **/
				int64 nReserve  = pindexPrev->nReleasedReserve[nType] + GetReleasedReserve(pindexNew, pindexNew->GetChannel(), nType);
				
				/** Block Version 3 Check. Disable Reserves from going below 0. **/
				if(pindexNew->nVersion >= 3 && pindexNew->nCoinbaseRewards[nType] >= nReserve)
					return error("AddToBlockIndex() : Coinbase Transaction too Large. Out of Reserve Limits");
				
				pindexNew->nReleasedReserve[nType] =  nReserve - pindexNew->nCoinbaseRewards[nType];
				
				if(GetArg("-verbose", 0) >= 2)
					printf("Reserve Balance %i | %f Nexus | Released %f\n", nType, pindexNew->nReleasedReserve[nType] / 1000000.0, (nReserve - pindexPrev->nReleasedReserve[nType]) / 1000000.0 );
			}
			else
				pindexNew->nReleasedReserve[nType] = 0;
				
		}
												
		/** Add the Pending Checkpoint into the Blockchain. **/
		if(!pindexNew->pprev || HardenCheckpoint(pindexNew))
		{
			pindexNew->PendingCheckpoint = std::make_pair(pindexNew->nHeight, pindexNew->GetBlockHash());
			
			if(GetArg("-verbose", 0) >= 2)
				printf("===== New Pending Checkpoint Hash = %s Height = %u\n", pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->nHeight);
		}
		else
		{
			pindexNew->PendingCheckpoint = pindexNew->pprev->PendingCheckpoint;
			
			unsigned int nAge = pindexNew->pprev->GetBlockTime() - mapBlockIndex[pindexNew->PendingCheckpoint.second]->GetBlockTime();
			
			if(GetArg("-verbose", 0) >= 2)
				printf("===== Pending Checkpoint Age = %u Hash = %s Height = %u\n", nAge, pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->PendingCheckpoint.first);
		}								 

		/** Add to the MapBlockIndex **/
		std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.insert(std::make_pair(hash, pindexNew)).first;
		pindexNew->phashBlock = &((*mi).first);


		/** Write the new Block to Disk. **/
		LLD::CIndexDB indexdb("r+");
		indexdb.TxnBegin();
		indexdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));
		

		return true;
	}
	
	bool CBlkPool::SetBestBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew, LLP::CNode* pfrom)
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
						return error("CBlock::SetBestChain() : plonger->pprev is null");
				if (pfork == plonger)
					break;
				if (!(pfork = pfork->pprev))
					return error("CBlock::SetBestChain() : pfork->pprev is null");
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
					return error("CBlock::SetBestChain() : ReadFromDisk for disconnect failed");
				if (!block.DisconnectBlock(indexdb, pindex))
					return error("CBlock::SetBestChain() : DisconnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());
					
				/** Remove Transactions from Current Trust Keys **/
				if(block.IsProofOfStake() && !cTrustPool.Remove(block))
					return error("CBlock::SetBestChain() : Disconnect Failed to Remove Trust Key at Block %s", pindex->GetBlockHash().ToString().substr(0,20).c_str());
				
				/** Resurrect Memory Pool Transactions. **/
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
					return error("CBlock::SetBestChain() : ReadFromDisk for connect failed");
				
				
				if (!block.ConnectBlock(indexdb, pindex, pfrom))
				{
					indexdb.TxnAbort();
					return error("CBlock::SetBestChain() : ConnectBlock %s Height %u failed", pindex->GetBlockHash().ToString().substr(0,20).c_str(), pindex->nHeight);
				}
				
				
				/* Harden a pending checkpoint if this is the case. */
				if(pindex->pprev && IsNewTimespan(pindex))
					HardenCheckpoint(pindex);
				
				
				/* Add Transaction to Current Trust Keys */
				if(block.IsProofOfStake() && !cTrustPool.Accept(block))
					return error("CBlock::SetBestChain() : Failed To Accept Trust Key Block.");

				
				/* Delete Memory Pool Transactions contained already. **/
				for(auto tx : block.vtx)
					if (!(tx.IsCoinBase() || tx.IsCoinStake()))
						vDelete.push_back(tx);
			}
			
			
			/* Write the Best Chain to the Index Database LLD. */
			if (!indexdb.WriteHashBestChain(pindexNew->GetBlockHash()))
				return error("CBlock::SetBestChain() : WriteHashBestChain failed");

			
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
				

			/* Reset the txpool states to VERIFIED not ACCEPTED if block disconnected. */
			for(auto tx : vDelete)
				pManager->txPool.SetState(tx.GetHash(), pManager->txPool.VERIFIED);
				
		}

		
		/* Establish the Best Variables for the Height of the Block-chain. */
		hashBestChain = hash;
		pindexBest = pindexNew;
		nBestHeight = pindexBest->nHeight;
		nBestChainTrust = pindexNew->nChainTrust;
		nTimeBestReceived = UnifiedTimestamp();
		
		
		if(GetArg("-verbose", 0) >= 0)
			printf("SetBestChain: new best=%s  height=%d  trust=%" PRIu64 "  moneysupply=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust, FormatMoney(pindexBest->nMoneySupply).c_str());

		
		std::string strCmd = GetArg("-blocknotify", "");
		if (!IsInitialBlockDownload() && !strCmd.empty())
		{
			boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
			boost::thread t(runCommand, strCmd);
		}

		return true;
	}
	

	
}
