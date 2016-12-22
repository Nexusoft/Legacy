/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../main.h"
#include "unifiedtime.h"

#include "../wallet/db.h"
#include "../LLU/ui_interface.h"

#include "../LLD/index.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;

namespace Core
{

	uint1024 GetOrphanRoot(const CBlock* pblock)
	{
		// Work back to the first block in the orphan chain
		while (mapOrphanBlocks.count(pblock->hashPrevBlock))
			pblock = mapOrphanBlocks[pblock->hashPrevBlock];
		return pblock->GetHash();
	}

	uint1024 WantedByOrphan(const CBlock* pblockOrphan)
	{
		// Work back to the first block in the orphan chain
		while (mapOrphanBlocks.count(pblockOrphan->hashPrevBlock))
			pblockOrphan = mapOrphanBlocks[pblockOrphan->hashPrevBlock];
		return pblockOrphan->hashPrevBlock;
	}

	const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
	{
		while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
			pindex = pindex->pprev;
			
		return pindex;
	}
	
	const CBlockIndex* GetLastChannelIndex(const CBlockIndex* pindex, int nChannel)
	{
		while (pindex && pindex->pprev && (pindex->GetChannel() != nChannel))
			pindex = pindex->pprev;
			
		return pindex;
	}

	
	int GetNumBlocksOfPeers() { return cPeerBlockCounts.Majority(); }

	
	bool IsInitialBlockDownload()
	{
		if (pindexBest == NULL || nBestHeight < GetNumBlocksOfPeers() - 1440 )
			return true;
			
		static int64 nLastUpdate;
		static CBlockIndex* pindexLastBest;
		if (pindexBest != pindexLastBest)
		{
			pindexLastBest = pindexBest;
			nLastUpdate = GetUnifiedTimestamp();
		}
		return (GetUnifiedTimestamp() - nLastUpdate < 30 &&
				pindexBest->GetBlockTime() < GetUnifiedTimestamp() - 24 * 60 * 60);
	}


	bool CBlock::ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions)
	{
		if (!fReadTransactions)
		{
			*this = pindex->GetBlockHeader();
			return true;
		}
		if (!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions))
			return false;
		if (GetHash() != pindex->GetBlockHash())
			return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
		return true;
	}


	void CBlock::UpdateTime() { nTime = std::max(mapBlockIndex[hashPrevBlock]->GetBlockTime() + 1, GetUnifiedTimestamp()); }

	
	bool CBlock::DisconnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex)
	{
		// Disconnect in reverse order
		for (int i = vtx.size() - 1; i >= 0; i--)
			if (!vtx[i].DisconnectInputs(indexdb))
				return false;

		// Update block index on disk without changing it in memory.
		// The memory index structure will be changed after the db commits.
		if (pindex->pprev)
		{
			CDiskBlockIndex blockindexPrev(pindex->pprev);
			blockindexPrev.hashNext = 0;
			if (!indexdb.WriteBlockIndex(blockindexPrev))
				return error("DisconnectBlock() : WriteBlockIndex failed");
		}

		// Nexus: clean up wallet after disconnecting coinstake
		BOOST_FOREACH(CTransaction& tx, vtx)
			SyncWithWallets(tx, this, false, false);

		return true;
	}

	
	bool CBlock::ConnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex)
	{

		// Do not allow blocks that contain transactions which 'overwrite' older transactions,
		// unless those are already completely spent.
		// If such overwrites are allowed, coinbases and transactions depending upon those
		// can be duplicated to remove the ability to spend the first instance -- even after
		// being sent to another address.
		BOOST_FOREACH(CTransaction& tx, vtx)
		{
			CTxIndex txindexOld;
			if (indexdb.ReadTxIndex(tx.GetHash(), txindexOld))
			{
				BOOST_FOREACH(CDiskTxPos &pos, txindexOld.vSpent)
					if (pos.IsNull()){
						
						return error("ConnectBlock() : Transaction Disk Index is Null %s", tx.GetHash().ToString().c_str());
					}
						
			}
		}

		//// issue here: it doesn't know the version
		unsigned int nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, DATABASE_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(vtx.size());

		map<uint512, CTxIndex> mapQueuedChanges;
		int64 nFees = 0;
		int64 nValueIn = 0;
		int64 nValueOut = 0;
		unsigned int nSigOps = 0;
		unsigned int nIterator = 0;
		BOOST_FOREACH(CTransaction& tx, vtx)
		{
			nSigOps += tx.GetLegacySigOpCount();
			if (nSigOps > MAX_BLOCK_SIGOPS)
				return DoS(100, error("ConnectBlock() : too many sigops"));

			CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);
			nTxPos += ::GetSerializeSize(tx, SER_DISK, DATABASE_VERSION);

			MapPrevTx mapInputs;
			if (tx.IsCoinBase())
				nValueOut += tx.GetValueOut();
			else
			{
				bool fInvalid;
				if (!tx.FetchInputs(indexdb, mapQueuedChanges, true, false, mapInputs, fInvalid))
					return error("ConnectBlock() : Failed to Connect Inputs.");


				// Add in sigops done by pay-to-script-hash inputs;
				// this is to prevent a "rogue miner" from creating
				// an incredibly-expensive-to-validate block.
				nSigOps += tx.TotalSigOps(mapInputs);
				if (nSigOps > MAX_BLOCK_SIGOPS)
					return DoS(100, error("ConnectBlock() : too many sigops"));

				int64 nTxValueIn = tx.GetValueIn(mapInputs);
				int64 nTxValueOut = tx.GetValueOut();
				
				nValueIn += nTxValueIn;
				nValueOut += nTxValueOut;
				
				if (!tx.ConnectInputs(indexdb, mapInputs, mapQueuedChanges, posThisTx, pindex, true, false))
					return error("ConnectBlock() : Failed to Connect Inputs...");
			}

			nIterator++;
			mapQueuedChanges[tx.GetHash()] = CTxIndex(posThisTx, tx.vout.size());
		}

		// track money supply and mint amount info
		pindex->nMint = nValueOut - nValueIn;
		pindex->nMoneySupply = (pindex->pprev ? pindex->pprev->nMoneySupply : 0) + nValueOut - nValueIn;
		
		if(GetArg("-verbose", 0) >= 0)
			printf("Generated %f Nexus\n", (double) pindex->nMint / COIN);
		
		if (!indexdb.WriteBlockIndex(CDiskBlockIndex(pindex)))
			return error("Connect() : WriteBlockIndex for pindex failed");

		// Write queued txindex changes
		for (map<uint512, CTxIndex>::iterator mi = mapQueuedChanges.begin(); mi != mapQueuedChanges.end(); ++mi)
		{
			if (!indexdb.UpdateTxIndex((*mi).first, (*mi).second))
				return error("ConnectBlock() : UpdateTxIndex failed");
		}

		// Nexus: fees are not collected by miners as in bitcoin
		// Nexus: fees are destroyed to compensate the entire network
		if(GetArg("-verbose", 0) >= 1)
			printf("ConnectBlock() : destroy=%s nFees=%"PRI64d"\n", FormatMoney(nFees).c_str(), nFees);

		// Update block index on disk without changing it in memory.
		// The memory index structure will be changed after the db commits.
		if (pindex->pprev)
		{
			CDiskBlockIndex blockindexPrev(pindex->pprev);
			blockindexPrev.hashNext = pindex->GetBlockHash();
			
			if (!indexdb.WriteBlockIndex(blockindexPrev))
				return error("ConnectBlock() : WriteBlockIndex for blockindexPrev failed");
		}

		// Watch for transactions paying to me
		BOOST_FOREACH(CTransaction& tx, vtx)
			SyncWithWallets(tx, this, true);

		return true;
	}

	
	static void
	runCommand(std::string strCommand)
	{
		int nErr = ::system(strCommand.c_str());
		if (nErr)
			printf("runCommand error: system(%s) returned %d\n", strCommand.c_str(), nErr);
	}

	
	bool CBlock::SetBestChain(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew)
	{
		uint1024 hash = GetHash();
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

			
			/** List of what to Disconnect. **/
			vector<CBlockIndex*> vDisconnect;
			for (CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
				vDisconnect.push_back(pindex);

			
			/** List of what to Connect. **/
			vector<CBlockIndex*> vConnect;
			for (CBlockIndex* pindex = pindexNew; pindex != pfork; pindex = pindex->pprev)
				vConnect.push_back(pindex);
			reverse(vConnect.begin(), vConnect.end());

			
			/** Debug output if there is a fork. **/
			if(vDisconnect.size() > 0 && GetArg("-verbose", 0) >= 1)
			{
				printf("REORGANIZE: Disconnect %i blocks; %s..%s\n", vDisconnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexBest->GetBlockHash().ToString().substr(0,20).c_str());
				printf("REORGANIZE: Connect %i blocks; %s..%s\n", vConnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->GetBlockHash().ToString().substr(0,20).c_str());
			}
			
			/** Disconnect the Shorter Branch. **/
			vector<CTransaction> vResurrect;
			BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
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
				BOOST_FOREACH(const CTransaction& tx, block.vtx)
					if (!(tx.IsCoinBase() || tx.IsCoinStake()))
						vResurrect.push_back(tx);
			}


			
			/* Connect the Longer Branch. */
			vector<CTransaction> vDelete;
			for (unsigned int i = 0; i < vConnect.size(); i++)
			{
				CBlockIndex* pindex = vConnect[i];
				CBlock block;
				if (!block.ReadFromDisk(pindex))
					return error("CBlock::SetBestChain() : ReadFromDisk for connect failed");
				
				if (!block.ConnectBlock(indexdb, pindex))
				{
					indexdb.TxnAbort();
					return error("CBlock::SetBestChain() : ConnectBlock %s Height %u failed", pindex->GetBlockHash().ToString().substr(0,20).c_str(), pindex->nHeight);
				}
				
				/* Harden a pending checkpoint if this is the case. */
				if(pindex->pprev && IsNewTimespan(pindex))
					HardenCheckpoint(pindex);
				
				/** Add Transaction to Current Trust Keys **/
				if(block.IsProofOfStake() && !cTrustPool.Accept(block))
					return error("CBlock::SetBestChain() : Failed To Accept Trust Key Block.");

				/* Delete Memory Pool Transactions contained already. **/
				BOOST_FOREACH(const CTransaction& tx, block.vtx)
					if (!(tx.IsCoinBase() || tx.IsCoinStake()))
						vDelete.push_back(tx);
			}
			
			/** Write the Best Chain to the Index Database LLD. **/
			if (!indexdb.WriteHashBestChain(pindexNew->GetBlockHash()))
				return error("CBlock::SetBestChain() : WriteHashBestChain failed");

			
			/** Disconnect Shorter Branch in Memory. **/
			BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
				if (pindex->pprev)
					pindex->pprev->pnext = NULL;


			/** Conenct the Longer Branch in Memory. **/
			BOOST_FOREACH(CBlockIndex* pindex, vConnect)
				if (pindex->pprev)
					pindex->pprev->pnext = pindex;


			BOOST_FOREACH(CTransaction& tx, vResurrect)
				tx.AcceptToMemoryPool(indexdb, false);
				

			BOOST_FOREACH(CTransaction& tx, vDelete)
				mempool.remove(tx);
				
		}
		
		
		/** Update the Best Block in the Wallet. **/
		bool fIsInitialDownload = IsInitialBlockDownload();
		if (!fIsInitialDownload)
		{
			const CBlockLocator locator(pindexNew);
			
			Core::SetBestChain(locator);
		}

		
		/** Establish the Best Variables for the Height of the Block-chain. **/
		hashBestChain = hash;
		pindexBest = pindexNew;
		nBestHeight = pindexBest->nHeight;
		nBestChainTrust = pindexNew->nChainTrust;
		nTimeBestReceived = GetUnifiedTimestamp();
		
		if(GetArg("-verbose", 0) >= 0)
			printf("SetBestChain: new best=%s  height=%d  trust=%"PRIu64"  moneysupply=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust, FormatMoney(pindexBest->nMoneySupply).c_str());
		
		/** Grab the transactions for the block and set the address balances. **/
		for(int nTx = 0; nTx < vtx.size(); nTx++)
		{
			for(int nOut = 0; nOut < vtx[nTx].vout.size(); nOut++)
			{	
				Wallet::NexusAddress cAddress;
				if(!Wallet::ExtractAddress(vtx[nTx].vout[nOut].scriptPubKey, cAddress))
					continue;
							
				mapAddressTransactions[cAddress.GetHash256()] += (uint64) vtx[nTx].vout[nOut].nValue;
						
				if(GetArg("-verbose", 0) >= 2)
					printf("%s Credited %f Nexus | Balance : %f Nexus\n", cAddress.ToString().c_str(), (double)vtx[nTx].vout[nOut].nValue / COIN, (double)mapAddressTransactions[cAddress.GetHash256()] / COIN);
			}
					
			if(!vtx[nTx].IsCoinBase())
			{
				BOOST_FOREACH(const CTxIn& txin, vtx[nTx].vin)
				{
					if(txin.prevout.IsNull())
						continue;
						
					CTransaction tx;
					CTxIndex txind;
							
					if(!indexdb.ReadTxIndex(txin.prevout.hash, txind))
						continue;
								
					if(!tx.ReadFromDisk(txind.pos))
						continue;
							
					Wallet::NexusAddress cAddress;
					if(!Wallet::ExtractAddress(tx.vout[txin.prevout.n].scriptPubKey, cAddress))
						continue;
							
					if(tx.vout[txin.prevout.n].nValue > mapAddressTransactions[cAddress.GetHash256()])
						mapAddressTransactions[cAddress.GetHash256()] = 0;
					else
						mapAddressTransactions[cAddress.GetHash256()] -= (uint64) tx.vout[txin.prevout.n].nValue;
					
					if(GetArg("-verbose", 0) >= 2)
						printf("%s Debited %f Nexus | Balance : %f Nexus\n", cAddress.ToString().c_str(), (double)tx.vout[txin.prevout.n].nValue / COIN, (double)mapAddressTransactions[cAddress.GetHash256()] / COIN);
				}
			}
		}

		
		std::string strCmd = GetArg("-blocknotify", "");
		if (!fIsInitialDownload && !strCmd.empty())
		{
			boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
			boost::thread t(runCommand, strCmd);
		}

		return true;
	}


	/** AddToBlockIndex: Adds a new Block into the Block Index. 
		This is where it is categorized and dealt with in the Blockchain. **/
	bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos)
	{
		/** Check for Duplicate. **/
		uint1024 hash = GetHash();
		if (mapBlockIndex.count(hash))
			return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());

			
		/** Build new Block Index Object. **/
		CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, *this);
		if (!pindexNew)
			return error("AddToBlockIndex() : new CBlockIndex failed");

			
		/** Find Previous Block. **/
		pindexNew->phashBlock = &hash;
		map<uint1024, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);
		if (miPrev != mapBlockIndex.end())
			pindexNew->pprev = (*miPrev).second;
		
		
		/** Compute the Chain Trust **/
		pindexNew->nChainTrust = (pindexNew->pprev ? pindexNew->pprev->nChainTrust : 0) + pindexNew->GetBlockTrust();
		
		
		/** Compute the Channel Height. **/
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
			pindexNew->PendingCheckpoint = make_pair(pindexNew->nHeight, pindexNew->GetBlockHash());
			
			if(GetArg("-verbose", 0) >= 2)
				printf("===== New Pending Checkpoint Hash = %s Height = %u\n", pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->nHeight);
		}
		else
		{
			pindexNew->PendingCheckpoint = pindexNew->pprev->PendingCheckpoint;
			
			unsigned int nAge = pindexNew->pprev->GetBlockTime() - mapBlockIndex[pindexNew->PendingCheckpoint.second]->GetBlockTime();
			
			if(GetArg("-verbose", 0) >= 2)
				printg("===== Pending Checkpoint Age = %u Hash = %s Height = %u\n", nAge, pindexNew->PendingCheckpoint.second.ToString().substr(0, 15).c_str(), pindexNew->PendingCheckpoint.first);
		}								 

		/** Add to the MapBlockIndex **/
		map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
		pindexNew->phashBlock = &((*mi).first);


		/** Write the new Block to Disk. **/
		LLD::CIndexDB indexdb("r+");
		indexdb.TxnBegin();
		indexdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));

		/** Set the Best chain if Highest Trust. **/
		if (pindexNew->nChainTrust > nBestChainTrust)
			if (!SetBestChain(indexdb, pindexNew))
				return false;
			
		/** Commit the Transaction to the Database. **/
		if(!indexdb.TxnCommit())
			return error("CBlock::AddToBlockIndex() : Failed to Commit Transaction to the Database.");
		
		if (pindexNew == pindexBest)
		{
			// Notify UI to display prev block's coinbase if it was ours
			static uint512 hashPrevBestCoinBase;
			UpdatedTransaction(hashPrevBestCoinBase);
			hashPrevBestCoinBase = vtx[0].GetHash();
		}

		MainFrameRepaint();
		return true;
	}
	
	/** Verify Work: Verify the Claimed Proof of Work amount for the Two Mining Channels. **/
	bool CBlock::VerifyWork() const
	{
		/** Check the Prime Number Proof of Work for the Prime Channel. **/
		if(GetChannel() == 1)
		{
			unsigned int nPrimeBits = GetPrimeBits(GetPrime());
			if (nPrimeBits < bnProofOfWorkLimit[1])
				return error("VerifyWork() : prime below minimum work");
			
			if(nBits > nPrimeBits)
				return error("VerifyWork() : prime cluster below target");
				
			return true;
		}
		
		CBigNum bnTarget;
		bnTarget.SetCompact(nBits);

		/** Check that the Hash is Within Range. **/
		if (bnTarget <= 0 || bnTarget > bnProofOfWorkLimit[2])
			return error("VerifyWork() : proof-of-work hash not in range");

		
		/** Check that the Hash is within Proof of Work Amount. **/
		if (GetHash() > bnTarget.getuint1024())
			return error("VerifyWork() : proof-of-work hash below target");

		return true;
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
	bool CBlock::CheckBlock() const
	{
		/** Check the Size limits of the Current Block. **/
		if (vtx.empty() || vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
			return DoS(100, error("CheckBlock() : size limits failed"));
			
			
		/** Make sure the Block was Created within Active Channel. **/
		if (GetChannel() > 2)
			return DoS(50, error("CheckBlock() : Channel out of Range."));
	
	
		/** Do not allow blocks to be accepted above the Current Block Version. **/
		if(nVersion > (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION))
			return DoS(50, error("CheckBlock() : Invalid Block Version."));
	
	
		/** Only allow POS blocks in Version 4. **/
		if(IsProofOfStake() && nVersion < 4)
			return DoS(50, error("CheckBlock() : Proof of Stake Blocks Rejected until Version 4."));
			
			
		/** Check the Proof of Work Claims. **/
		if (!IsInitialBlockDownload() && IsProofOfWork() && !VerifyWork())
			return DoS(50, error("CheckBlock() : Invalid Proof of Work"));

			
		/** Check the Network Launch Time-Lock. **/
		if (nHeight > 0 && GetBlockTime() <= (fTestNet ? NEXUS_TESTNET_TIMELOCK : NEXUS_NETWORK_TIMELOCK))
			return error("CheckBlock() : Block Created before Network Time-Lock");
			
			
		/** Check the Current Channel Time-Lock. **/
		if (nHeight > 0 && GetBlockTime() < (fTestNet ? CHANNEL_TESTNET_TIMELOCK[GetChannel()] : CHANNEL_NETWORK_TIMELOCK[GetChannel()]))
			return error("CheckBlock() : Block Created before Channel Time-Lock. Channel Opens in %"PRId64" Seconds", (fTestNet ? CHANNEL_TESTNET_TIMELOCK[GetChannel()] : CHANNEL_NETWORK_TIMELOCK[GetChannel()]) - GetUnifiedTimestamp());
			
			
		/** Check the Version 1 Block Time-Lock. **/	
		if (nHeight > 0 && nVersion == 1 && GetBlockTime() > (fTestNet ? TESTNET_VERSION2_TIMELOCK : NETWORK_VERSION2_TIMELOCK))
			return error("CheckBlock() : Version 1 Blocks have been Obsolete for %"PRId64" Seconds", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION2_TIMELOCK : NETWORK_VERSION2_TIMELOCK)));
			
			
		/** Check the Version 2 Block Time-Lock. **/
		if (nHeight > 0 && nVersion >= 2 && GetBlockTime() <= (fTestNet ? TESTNET_VERSION2_TIMELOCK : NETWORK_VERSION2_TIMELOCK))
			return error("CheckBlock() : Version 2 Blocks are not Accepted for %"PRId64" Seconds\n", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION2_TIMELOCK : NETWORK_VERSION2_TIMELOCK)));

			
		/** Check the Version 3 Block Time-Lock. **/
		if (nHeight > 0 && nVersion == 2 && GetBlockTime() > (fTestNet ? TESTNET_VERSION3_TIMELOCK : NETWORK_VERSION3_TIMELOCK))
			return error("CheckBlock() : Version 2 Blocks have been Obsolete for %"PRId64" Seconds\n", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION3_TIMELOCK : NETWORK_VERSION3_TIMELOCK)));	
			
			
		/** Check the Version 3 Block Time-Lock. **/
		if (nHeight > 0 && nVersion >= 3 && GetBlockTime() <= (fTestNet ? TESTNET_VERSION3_TIMELOCK : NETWORK_VERSION3_TIMELOCK))
			return error("CheckBlock() : Version 3 Blocks are not Accepted for %"PRId64" Seconds\n", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION3_TIMELOCK : NETWORK_VERSION3_TIMELOCK)));	
			
			
		/** Check the Version 4 Block Time-Lock. Allow Version 3 Blocks for 1 Hour after Time Lock. **/
		if (nHeight > 0 && nVersion == 3 && (GetBlockTime() - 3600) > (fTestNet ? TESTNET_VERSION4_TIMELOCK : NETWORK_VERSION4_TIMELOCK))
			return error("CheckBlock() : Version 3 Blocks have been Obsolete for %"PRId64" Seconds\n", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION4_TIMELOCK : NETWORK_VERSION4_TIMELOCK)));	
			
			
		/** Check the Version 4 Block Time-Lock. **/
		if (nHeight > 0 && nVersion >= 4 && GetBlockTime() <= (fTestNet ? TESTNET_VERSION4_TIMELOCK : NETWORK_VERSION4_TIMELOCK))
			return error("CheckBlock() : Version 4 Blocks are not Accepted for %"PRId64" Seconds\n", (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION4_TIMELOCK : NETWORK_VERSION4_TIMELOCK)));	
			
			
		/** Check the Required Mining Outputs. **/
		if (IsProofOfWork() && nHeight > 0 && nVersion >= 3)
		{
			unsigned int nSize = vtx[0].vout.size();

			/** Check the Coinbase Tx Size. **/
			if(nSize < 3)
				return error("CheckBlock() : Coinbase Too Small.");
				
			if(!fTestNet)
			{
				if (!VerifyAddressList(vtx[0].vout[nSize - 2].scriptPubKey, CHANNEL_SIGNATURES))
					return error("CheckBlock() : Block %u Channel Signature Not Verified.\n", nHeight);

				if (!VerifyAddressList(vtx[0].vout[nSize - 1].scriptPubKey, DEVELOPER_SIGNATURES))
					return error("CheckBlock() :  Block %u Developer Signature Not Verified.\n", nHeight);
			}
			
			else
			{
				if (!VerifyAddress(vtx[0].vout[nSize - 2].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CheckBlock() :  Block %u Channel Signature Not Verified.\n", nHeight);

				if (!VerifyAddress(vtx[0].vout[nSize - 1].scriptPubKey, TESTNET_DUMMY_SIGNATURE))
					return error("CheckBlock() :  Block %u Developer Signature Not Verified.\n", nHeight);
			}
		}

			
		/** Check the Coinbase Transaction is First, with no repetitions. **/
		if (vtx.empty() || (!vtx[0].IsCoinBase() && nChannel > 0))
			return DoS(100, error("CheckBlock() : first tx is not coinbase for Proof of Work Block"));
			
			
		/** Check the Coinstake Transaction is First, with no repetitions. **/
		if (vtx.empty() || (!vtx[0].IsCoinStake() && nChannel == 0))
			return DoS(100, error("CheckBlock() : first tx is not coinstake for Proof of Stake Block"));
			
			
		/** Check for duplicate Coinbase / Coinstake Transactions. **/
		for (unsigned int i = 1; i < vtx.size(); i++)
			if (vtx[i].IsCoinBase() || vtx[i].IsCoinStake())
				return DoS(100, error("CheckBlock() : more than one coinbase / coinstake"));

				
		/** Check coinbase/coinstake timestamp is at least 20 minutes before block time **/
		if (GetBlockTime() > (int64)vtx[0].nTime + ((nVersion < 4) ? 1200 : 3600))
			return DoS(50, error("CheckBlock() : coinbase/coinstake timestamp is too early"));
		
		
		/** Check the Transactions in the Block. **/
		BOOST_FOREACH(const CTransaction& tx, vtx)
		{
			if (!tx.CheckTransaction())
				return DoS(tx.nDoS, error("CheckBlock() : CheckTransaction failed"));
				
			// Nexus: check transaction timestamp
			if (GetBlockTime() < (int64)tx.nTime)
				return DoS(50, error("CheckBlock() : block timestamp earlier than transaction timestamp"));
		}

		
		// Check for duplicate txids. This is caught by ConnectInputs(),
		// but catching it earlier avoids a potential DoS attack:
		set<uint512> uniqueTx;
		BOOST_FOREACH(const CTransaction& tx, vtx)
		{
			uniqueTx.insert(tx.GetHash());
		}
		if (uniqueTx.size() != vtx.size())
			return DoS(100, error("CheckBlock() : duplicate transaction"));

		unsigned int nSigOps = 0;
		BOOST_FOREACH(const CTransaction& tx, vtx)
		{
			nSigOps += tx.GetLegacySigOpCount();
		}
		
		if (nSigOps > MAX_BLOCK_SIGOPS)
			return DoS(100, error("CheckBlock() : out-of-bounds SigOpCount"));

		// Check merkleroot
		if (hashMerkleRoot != BuildMerkleTree())
			return DoS(100, error("CheckBlock() : hashMerkleRoot mismatch"));

		// Nexus: check block signature
		if (!CheckBlockSignature())
			return DoS(100, error("CheckBlock() : bad block signature"));

		return true;
	}
	
	
	bool CBlock::AcceptBlock()
	{
		/** Check for Duplicate Block. **/
		uint1024 hash = GetHash();
		if (mapBlockIndex.count(hash))
			return error("AcceptBlock() : block already in mapBlockIndex");

			
		/** Find the Previous block from hashPrevBlock. **/
		map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
		if (mi == mapBlockIndex.end())
			return DoS(10, error("AcceptBlock() : prev block not found"));
		CBlockIndex* pindexPrev = (*mi).second;
		int nPrevHeight = pindexPrev->nHeight + 1;
		
		
		/** Check the Height of Block to Previous Block. **/
		if(nPrevHeight != nHeight)
			return DoS(100, error("AcceptBlock() : incorrect block height."));
		

		/** Check that the nBits match the current Difficulty. **/
		if (nBits != GetNextTargetRequired(pindexPrev, GetChannel(), !IsInitialBlockDownload()))
			return DoS(100, error("AcceptBlock() : incorrect proof-of-work/proof-of-stake"));
			
			
		/** Check that Block is Descendant of Hardened Checkpoints. **/
		if(pindexPrev && !IsDescendant(pindexPrev))
			return error("AcceptBlock() : Not a descendant of Last Checkpoint");

			
		/** Check That Block Timestamp is not before previous block. **/
		if (GetBlockTime() <= pindexPrev->GetBlockTime())
			return error("AcceptBlock() : block's timestamp too early Block: %"PRId64" Prev: %"PRId64"", GetBlockTime(), pindexPrev->GetBlockTime());
			
		
		if (GetBlockTime() > GetUnifiedTimestamp() + MAX_UNIFIED_DRIFT)
			return error("AcceptBlock() : block timestamp too far in the future");
			
			
		/** Check the Coinbase Transactions in Block Version 3. **/
		if(IsProofOfWork() && nHeight > 0 && nVersion >= 3)
		{
			unsigned int nSize = vtx[0].vout.size();

			/** Add up the Miner Rewards from Coinbase Tx Outputs. **/
			int64 nMiningReward = 0;
			for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
				nMiningReward += vtx[0].vout[nIndex].nValue;
					
			/** Check that the Mining Reward Matches the Coinbase Calculations. **/
			if (nMiningReward != GetCoinbaseReward(pindexPrev, GetChannel(), 0))
				return error("AcceptBlock() : miner reward mismatch %"PRId64" : %"PRId64"", nMiningReward, GetCoinbaseReward(pindexPrev, GetChannel(), 0));
					
			/** Check that the Exchange Reward Matches the Coinbase Calculations. **/
			if (vtx[0].vout[nSize - 2].nValue != GetCoinbaseReward(pindexPrev, GetChannel(), 1))
				return error("AcceptBlock() : exchange reward mismatch %"PRId64" : %"PRId64"\n", vtx[0].vout[1].nValue, GetCoinbaseReward(pindexPrev, GetChannel(), 1));
						
			/** Check that the Developer Reward Matches the Coinbase Calculations. **/
			if (vtx[0].vout[nSize - 1].nValue != GetCoinbaseReward(pindexPrev, GetChannel(), 2))
				return error("AcceptBlock() : developer reward mismatch %"PRId64" : %"PRId64"\n", vtx[0].vout[2].nValue, GetCoinbaseReward(pindexPrev, GetChannel(), 2));
					
		}
		
		/** Check the Proof of Stake Claims. **/
		else if (IsProofOfStake())
		{
			if(!cTrustPool.Check(*this))
				return DoS(50, error("AcceptBlock() : Invalid Trust Key"));
			
			/** Verify the Stake Kernel. **/
			if(!VerifyStake())
				return DoS(50, error("AcceptBlock() : Invalid Proof of Stake"));
		}
		
			
		/** Check that Transactions are Finalized. **/
		BOOST_FOREACH(const CTransaction& tx, vtx)
			if (!tx.IsFinal(nHeight, GetBlockTime()))
				return DoS(10, error("AcceptBlock() : contains a non-final transaction"));

				
		/** Write new Block to Disk. **/
		if (!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK, DATABASE_VERSION)))
			return error("AcceptBlock() : out of disk space");
			
			
		unsigned int nFile = -1;
		unsigned int nBlockPos = 0;
		if (!WriteToDisk(nFile, nBlockPos))
			return error("AcceptBlock() : WriteToDisk failed");
		if (!AddToBlockIndex(nFile, nBlockPos))
			return error("AcceptBlock() : AddToBlockIndex failed");

			
		/** Relay the Block to Nexus Network. **/
		if (hashBestChain == hash && !IsInitialBlockDownload())
		{
			LOCK(Net::cs_vNodes);
			BOOST_FOREACH(Net::CNode* pnode, Net::vNodes)
				pnode->PushInventory(Net::CInv(Net::MSG_BLOCK, hash));
		}

		return true;
	}

	
	bool ProcessBlock(Net::CNode* pfrom, CBlock* pblock)
	{
		// Check for duplicate
		uint1024 hash = pblock->GetHash();
		if (mapBlockIndex.count(hash))
			return error("ProcessBlock() : already have block %d %s", mapBlockIndex[hash]->nHeight, hash.ToString().substr(0,20).c_str());
		if (mapOrphanBlocks.count(hash))
			return error("ProcessBlock() : already have block (orphan) %s", hash.ToString().substr(0,20).c_str());

		// Preliminary checks
		if (!pblock->CheckBlock())
			return error("ProcessBlock() : CheckBlock FAILED");

		// If don't already have its previous block, shunt it off to holding area until we get it
		if (!mapBlockIndex.count(pblock->hashPrevBlock))
		{
			if(GetArg("-verbose", 0) >= 0)
				printf("ProcessBlock: ORPHAN BLOCK, prev=%s\n", pblock->hashPrevBlock.ToString().substr(0,20).c_str());
			
			CBlock* pblock2 = new CBlock(*pblock);
			mapOrphanBlocks.insert(make_pair(hash, pblock2));
			mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrevBlock, pblock2));
			
			/** Simple Catch until I finish Checkpoint Syncing. **/
            if(pfrom)
                pfrom->PushGetBlocks(pindexBest, 0);
			
			return true;
		}

		// Store to disk
		if (!pblock->AcceptBlock())
			return error("ProcessBlock() : AcceptBlock FAILED");


		// Recursively process any orphan blocks that depended on this one
		vector<uint1024> vWorkQueue;
		vWorkQueue.push_back(hash);
		for (unsigned int i = 0; i < vWorkQueue.size(); i++)
		{
			uint1024 hashPrev = vWorkQueue[i];
			for (multimap<uint1024, CBlock*>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev);
				 mi != mapOrphanBlocksByPrev.upper_bound(hashPrev);
				 ++mi)
			{
				CBlock* pblockOrphan = (*mi).second;
				if (pblockOrphan->AcceptBlock())
					vWorkQueue.push_back(pblockOrphan->GetHash());
					
				mapOrphanBlocks.erase(pblockOrphan->GetHash());
				delete pblockOrphan;
			}
			mapOrphanBlocksByPrev.erase(hashPrev);
		}

		printg("ProcessBlock: ACCEPTED %s\n", pblock->GetHash().ToString().substr(0, 10).c_str());

		return true;
	}

	
	// Nexus: sign block
	bool CBlock::SignBlock(const Wallet::CKeyStore& keystore)
	{
		vector<std::vector<unsigned char> > vSolutions;
		Wallet::TransactionType whichType;
		const CTxOut& txout = vtx[0].vout[0];

		if (!Solver(txout.scriptPubKey, whichType, vSolutions))
			return false;
		if (whichType == Wallet::TX_PUBKEY)
		{
			// Sign
			const std::vector<unsigned char>& vchPubKey = vSolutions[0];
			Wallet::CKey key;
			if (!keystore.GetKey(SK256(vchPubKey), key))
				return false;
			if (key.GetPubKey() != vchPubKey)
				return false;
			return key.Sign((nVersion >= 4) ? SignatureHash() : GetHash(), vchBlockSig, 1024);
		}
		return false;
	}

	
	// Nexus: check block signature
	bool CBlock::CheckBlockSignature() const
	{
		if (GetHash() == hashGenesisBlock)
			return vchBlockSig.empty();

		vector<std::vector<unsigned char> > vSolutions;
		Wallet::TransactionType whichType;
		const CTxOut& txout = vtx[0].vout[0];

		if (!Solver(txout.scriptPubKey, whichType, vSolutions))
			return false;
		if (whichType == Wallet::TX_PUBKEY)
		{
			const std::vector<unsigned char>& vchPubKey = vSolutions[0];
			Wallet::CKey key;
			if (!key.SetPubKey(vchPubKey))
				return false;
			if (vchBlockSig.empty())
				return false;
			return key.Verify((nVersion >= 4) ? SignatureHash() : GetHash(), vchBlockSig, 1024);
		}
		return false;
	}


	bool CheckDiskSpace(uint64 nAdditionalBytes)
	{
		uint64 nFreeBytesAvailable = filesystem::space(GetDataDir()).available;

		// Check for 15MB because database could create another 10MB log file at any time
		if (nFreeBytesAvailable < (uint64)15000000 + nAdditionalBytes)
		{
			fShutdown = true;
			string strMessage = _("Warning: Disk space is low");
			strMiscWarning = strMessage;
			printf("*** %s\n", strMessage.c_str());
			ThreadSafeMessageBox(strMessage, "Nexus", wxOK | wxICON_EXCLAMATION | wxMODAL);
			StartShutdown();
			return false;
		}
		return true;
	}

	
	FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode)
	{
		if (nFile == -1)
			return NULL;
		FILE* file = fopen((GetDataDir() / strprintf("blk%04d.dat", nFile)).string().c_str(), pszMode);
		if (!file)
			return NULL;
		if (nBlockPos != 0 && !strchr(pszMode, 'a') && !strchr(pszMode, 'w'))
		{
			if (fseek(file, nBlockPos, SEEK_SET) != 0)
			{
				fclose(file);
				return NULL;
			}
		}
		return file;
	}

	
	unsigned int nCurrentBlockFile = 1;
	FILE* AppendBlockFile(unsigned int& nFileRet)
	{
		nFileRet = 0;
		loop
		{
			FILE* file = OpenBlockFile(nCurrentBlockFile, 0, "ab");
			if (!file)
				return NULL;
			if (fseek(file, 0, SEEK_END) != 0)
				return NULL;
			// FAT32 filesize max 4GB, fseek and ftell max 2GB, so we must stay under 2GB
			if (ftell(file) < 0x7F000000 - MAX_SIZE)
			{
				nFileRet = nCurrentBlockFile;
				return file;
			}
			fclose(file);
			nCurrentBlockFile++;
		}
	}

	
	bool LoadBlockIndex(bool fAllowNew)
	{
		if (fTestNet)
		{
			hashGenesisBlock = hashGenesisBlockTestNet;
			nCoinbaseMaturity = 10;
			
			TRUST_KEY_EXPIRE = 60 * 60 * 12;
			TRUST_KEY_MIN_INTERVAL = 5;
		}

		if(GetArg("-verbose", 0) >= 0)
			printf("%s Network: genesis=0x%s nBitsLimit=0x%08x nBitsInitial=0x%08x nCoinbaseMaturity=%d\n",
			   fTestNet? "Test" : "Nexus", hashGenesisBlock.ToString().substr(0, 20).c_str(), bnProofOfWorkLimit[0].GetCompact(), bnProofOfWorkStart[0].GetCompact(), nCoinbaseMaturity);

		/** Initialize Block Index Database. **/
		LLD::CIndexDB indexdb("cr");
		if (!indexdb.LoadBlockIndex() || mapBlockIndex.empty())
		{
			if (!fAllowNew)
				return false;


			const char* pszTimestamp = "Silver Doctors [2-19-2014] BANKER CLEAN-UP: WE ARE AT THE PRECIPICE OF SOMETHING BIG";
			CTransaction txNew;
			txNew.nTime = 1409456199;
			txNew.vin.resize(1);
			txNew.vout.resize(1);
			txNew.vin[0].scriptSig = Wallet::CScript() << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
			txNew.vout[0].SetEmpty();
			
			CBlock block;
			block.vtx.push_back(txNew);
			block.hashPrevBlock = 0;
			block.hashMerkleRoot = block.BuildMerkleTree();
			block.nVersion = 1;
			block.nHeight  = 0;
			block.nChannel = 2;
			block.nTime    = 1409456199;
			block.nBits    = bnProofOfWorkLimit[2].GetCompact();
			block.nNonce   = fTestNet ? 122999499 : 2196828850;

			assert(block.hashMerkleRoot == uint512("0x8a971e1cec5455809241a3f345618a32dc8cb3583e03de27e6fe1bb4dfa210c413b7e6e15f233e938674a309df5a49db362feedbf96f93fb1c6bfeaa93bd1986"));
			
			CBigNum target;
			target.SetCompact(block.nBits);
			block.print();
			
			if(block.GetHash() != hashGenesisBlock)
				return error("LoadBlockIndex() : genesis hash does not match");
			
			if(!block.CheckBlock())
				return error("LoadBlockIndex() : genesis block check failed");

			
			/** Write the New Genesis to Disk. **/
			unsigned int nFile;
			unsigned int nBlockPos;
			if (!block.WriteToDisk(nFile, nBlockPos))
				return error("LoadBlockIndex() : writing genesis block to disk failed");
			if (!block.AddToBlockIndex(nFile, nBlockPos))
				return error("LoadBlockIndex() : genesis block not accepted");
		}

		return true;
	}
	
	/** Useful to let the Map Block Index not need to be loaded fully.
		If a block index is ever needed it can be read from the LLD. 
		This lets us have load times of a few seconds 
		
		TODO:: Complete this **/
	bool CheckBlockIndex(uint1024 hashBlock)
	{
		
		return true;
	}
}


