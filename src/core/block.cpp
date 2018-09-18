/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

[Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "../main.h"
#include "unifiedtime.h"

#include "../wallet/db.h"
#include "../util/ui_interface.h"
#include "../net/net.h"

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


    static unsigned int nLastBlockTime = 0;
    static CBlockIndex* pindexLast = 0;

    bool IsInitialBlockDownload()
    {
        if(!pindexBest)
            return true;

        if(pindexBest != pindexLast)
        {
            pindexLast = pindexBest;
            nLastBlockTime = GetUnifiedTimestamp();
        }

        if(fTestNet || fLispNet)
            return (GetUnifiedTimestamp() - nLastBlockTime < 60);

        return (pindexBest->GetBlockTime() < GetUnifiedTimestamp() - 20 * 60);
    }


    bool CBlock::Reindex(CBlockIndex* pindex)
    {
        // Open history file to append
        CAutoFile fileout = CAutoFile(AppendBlockFile(pindex->nFile), SER_DISK, DATABASE_VERSION);
        if (!fileout)
            return error("CBlock::WriteToDisk() : AppendBlockFile failed");

        // Write index header
        unsigned char pchMessageStart[4];
        Net::GetMessageStart(pchMessageStart);

        unsigned int nSize = fileout.GetSerializeSize(*this);
        fileout << FLATDATA(pchMessageStart) << nSize;

        // Write block
        long fileOutPos = ftell(fileout);
        if (fileOutPos < 0)
            return error("CBlock::WriteToDisk() : ftell failed");
        pindex->nBlockPos = fileOutPos;
        fileout << *this;

        // Flush stdio buffers and commit to disk before returning
        fflush(fileout);
#ifdef WIN32
        _commit(_fileno(fileout));
#else
        fsync(fileno(fileout));
#endif

        //// issue here: it doesn't know the version
        unsigned int nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, DATABASE_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(vtx.size());

        LLD::CIndexDB indexdb("r+");
        for(auto tx : vtx)
        {

            CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);
            nTxPos += ::GetSerializeSize(tx, SER_DISK, DATABASE_VERSION);

            CTxIndex txindex;
            if (indexdb.ReadTxIndex(tx.GetHash(), txindex))
                txindex.pos = posThisTx;
            else
                txindex = CTxIndex(posThisTx, tx.vout.size());

            indexdb.UpdateTxIndex(tx.GetHash(), txindex);
        }

        if (!indexdb.WriteBlockIndex(CDiskBlockIndex(pindex)))
            return error("Connect() : WriteBlockIndex for pindex failed");

        return true;
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
            return error("CBlock::ReadFromDisk() : GetHash() %s doesn't match Index %s", GetHash().ToString().c_str(), pindex->GetBlockHash().ToString().c_str());
        return true;
    }


    void CBlock::UpdateTime() { nTime = std::max(mapBlockIndex[hashPrevBlock]->GetBlockTime() + 1, GetUnifiedTimestamp()); }


    bool CBlock::DisconnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex)
    {
        // Disconnect in reverse order
        for (int i = vtx.size() - 1; i >= 0; i--)
            if (!vtx[i].DisconnectInputs(indexdb))
                return false;

        // handle for trust keys
        if(IsProofOfStake())
        {
            std::vector<unsigned char> vTrustKey;
            if(!TrustKey(vTrustKey))
                return error("DisconnectBlock() : can't extract trust key.");

            /* Set the ckey object. */
            uint576 cKey;
            cKey.SetBytes(vTrustKey);

            /* Erase Genesis on disconnect. */
            if(vtx[0].IsGenesis())
                indexdb.EraseTrustKey(cKey);
            else //handle the indexing of last trust block. Not validation rules so don't throw failures
            {
                CTrustKey trustKey;
                if(indexdb.ReadTrustKey(cKey, trustKey))
                {
                    /* Don't allow Blocks Created Before Minimum Interval. */
                    if(nVersion < 5)
                    {
                        uint1024 hashLastTrust = hashPrevBlock;
                        if(LastTrustBlock(trustKey, hashLastTrust))
                        {
                            trustKey.hashLastBlock = hashLastTrust;

                            /* Write trust key changes to disk. */
                            indexdb.WriteTrustKey(cKey, trustKey);
                        }
                    }
                    else
                    {
                        /* Extract the data from the coinstake input. */
                        uint1024 hashLastTrust;
                        unsigned int nSequence, nTrust;
                        if(ExtractTrust(hashLastTrust, nSequence, nTrust))
                        {
                            trustKey.hashLastBlock = hashLastTrust;

                            /* Write trust key changes to disk. */
                            indexdb.WriteTrustKey(cKey, trustKey);
                        }
                    }
                }
            }
        }

        // Update block index on disk without changing it in memory.
        // The memory index structure will be changed after the db commits.
        if (pindex && pindex->pprev)
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
                    return error("ConnectBlock() : Failed to Fetch Inputs.");


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

        //move from check transaction to verify script size
        if (vtx[0].vin[0].scriptSig.size() < 2 || vtx[0].vin[0].scriptSig.size() > (nVersion < 5 ? 100 : 144))
            return DoS(100, error("CTransaction::CheckTransaction() : coinbase/coinstake script size"));

        // handle for trust keys
        std::vector<unsigned char> vTrustKey;
        if(!TrustKey(vTrustKey))
            return error("ConnectBlock() : can't extract trust key.");

        /* Set the ckey object. */
        uint576 cKey;
        cKey.SetBytes(vTrustKey);

        /* Check the proof of stake claims. */
        if (IsProofOfStake())
        {
            /* Check the trust scores on version 5 blocks. */
            if(nVersion >= 5 && !CheckTrust())
                return DoS(50, error("ConnectBlock() : invalid trust score"));

            /* Verify the stake on version 4 blocks. */
            else if(nVersion == 4 && !VerifyStake())
                return DoS(50, error("ConnectBlock() : invalid proof of stake"));
        }

        /* Handle Genesis Transaction Rules. Genesis is checked after Trust Key Established. */
        if(vtx[0].IsGenesis())
        {
            /* Check that Transaction is not Genesis when Trust Key is Established. */
            CTrustKey trustCheck;
            if(nVersion >= 5 && indexdb.ReadTrustKey(cKey, trustCheck))
            {
                if(trustCheck.vchPubKey != vTrustKey || trustCheck.hashGenesisBlock != GetHash() ||
                   trustCheck.hashGenesisTx != vtx[0].GetHash() || trustCheck.nGenesisTime != nTime)
                   return error("ConnectBlock() : Duplicate Genesis not Allowed");
            }

            /* Create the Trust Key from Genesis Transaction Block. */
            CTrustKey trustKey(vTrustKey, GetHash(), vtx[0].GetHash(), nTime);

            /* Check the genesis transaction. */
            if(!trustKey.CheckGenesis(*this))
                return error("ConnectBlock() : Invalid Genesis Transaction.");

            /* Write the trust key to indexDB */
            indexdb.WriteTrustKey(cKey, trustKey);

            /* Dump the Trust Key to Console if not Initializing. */
            if(GetArg("-verbose", 0) >= 2)
                trustKey.Print();
        }

        /* Handle Adding Trust Transactions. */
        else if(vtx[0].IsTrust())
        {
            /* No Trust Transaction without a Genesis. */
            CTrustKey trustKey;
            if(!indexdb.ReadTrustKey(cKey, trustKey))
            {
                if(!FindGenesis(cKey, trustKey, hashPrevBlock))
                    return DoS(50, error("ConnectBlock() : no trust without genesis"));

                indexdb.WriteTrustKey(cKey, trustKey);
            }

            /* Check that the Trust Key and Current Block match. */
            if(trustKey.vchPubKey != vTrustKey)
                return error("ConnectBlock() : Trust Key and Block Key Mismatch.");

            /* Trust Keys can only exist after the Genesis Transaction. */
            if(!mapBlockIndex.count(trustKey.hashGenesisBlock))
                return error("ConnectBlock() : Genesis Block Not Found.");

            /* Read the Genesis Transaction's Block from Disk. */
            CBlock blockGenesis;
            if(!blockGenesis.ReadFromDisk(mapBlockIndex[trustKey.hashGenesisBlock], true))
                return error("ConnectBlock() : Could not Read Genesis Block.");

            /* Double Check the Genesis Transaction. */
            if(!trustKey.CheckGenesis(blockGenesis))
                return error("ConnectBlock() : Invalid Genesis Transaction.");

            /* Don't allow Expired Trust Keys. Check Expiration from Previous Block Timestamp. */
            if(nVersion < 5 && trustKey.Expired(hashPrevBlock))
                return error("ConnectBlock() : Cannot Create Block for Expired Trust Key.");

            /* Don't allow Blocks Created Before Minimum Interval. */
            if(nVersion < 5)
            {
                uint1024 hashLastTrust = hashPrevBlock;
                if(!LastTrustBlock(trustKey, hashLastTrust))
                    return error("ConnectBlock() : Can't find last trust block");

                if(nHeight - mapBlockIndex[hashLastTrust]->nHeight < TRUST_KEY_MIN_INTERVAL)
                    return error("ConnectBlock() : Trust Block Created Before Minimum Trust Key Interval.");
            }
            else
            {
                /* Extract the data from the coinstake input. */
                uint1024 hashLastTrust;
                unsigned int nSequence, nTrust;
                if(!ExtractTrust(hashLastTrust, nSequence, nTrust))
                    return error("ConnectBlock() : can't extract trust from coinstake inputs");
            }

            /* Write trust key changes to disk. */
            trustKey.hashLastBlock = GetHash();
            indexdb.WriteTrustKey(cKey, trustKey);

            /* Dump the Trust Key to Console if not Initializing. */
            if(GetArg("-verbose", 0) >= 2)
                trustKey.Print();
        }

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
            printf("ConnectBlock() : destroy=%s nFees=%" PRI64d "\n", FormatMoney(nFees).c_str(), nFees);

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
            if(nVersion < 5 && !fTestNet && !IsInitialBlockDownload() && IsProofOfStake())
            {
                uint576 cKey;
                if(TrustKey(cKey))
                {
                    CTrustKey trustKey;
                    if(indexdb.ReadTrustKey(cKey, trustKey) && !trustKey.IsValid(*this))
                    {
                        error("\x1b[31m SOFTBAN: Invalid nPoS %s\x1b[0m", hash.ToString().substr(0, 20).c_str());

                        return true;
                    }
                }
                else
                    error("\x1b[31m SOFTBAN \x1b[0m : couldn't get trust key");
            }

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
            vector<CBlockIndex*> vDisconnect;
            for (CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
                vDisconnect.push_back(pindex);


            /* List of what to Connect. */
            vector<CBlockIndex*> vConnect;
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
            vector<CTransaction> vResurrect;
            BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
            {
                CBlock block;
                if (!block.ReadFromDisk(pindex))
                    return error("CBlock::SetBestChain() : ReadFromDisk for disconnect failed");
                if (!block.DisconnectBlock(indexdb, pindex))
                    return error("CBlock::SetBestChain() : DisconnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());

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

                /* Delete Memory Pool Transactions contained already. **/
                BOOST_FOREACH(const CTransaction& tx, block.vtx)
                    if (!(tx.IsCoinBase() || tx.IsCoinStake()))
                        vDelete.push_back(tx);
            }

            /* Write the Best Chain to the Index Database LLD. */
            if (!indexdb.WriteHashBestChain(pindexNew->GetBlockHash()))
                return error("CBlock::SetBestChain() : WriteHashBestChain failed");


            /* Disconnect Shorter Branch in Memory. */
            BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
                if (pindex->pprev)
                    pindex->pprev->pnext = NULL;


            /* Conenct the Longer Branch in Memory. */
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
            printf("SetBestChain: new best=%s  height=%d  trust=%" PRIu64 "  moneysupply=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, nBestChainTrust, FormatMoney(pindexBest->nMoneySupply).c_str());

        /** Grab the transactions for the block and set the address balances. **/
        if(GetBoolArg("-richlist", false))
        {
            for(int nTx = 0; nTx < vtx.size(); nTx++)
            {
                for(int nOut = 0; nOut < vtx[nTx].vout.size(); nOut++)
                {
                    Wallet::NexusAddress cAddress;
                    if(!Wallet::ExtractAddress(vtx[nTx].vout[nOut].scriptPubKey, cAddress))
                        continue;

                    if(!Core::mapRichList.count(cAddress.GetHash256()))
                        mapRichList[cAddress.GetHash256()] = { std::make_pair(vtx[nTx].IsCoinBase(), vtx[nTx].GetHash()) };
                    else
                        mapRichList[cAddress.GetHash256()].push_back(std::make_pair(vtx[nTx].IsCoinBase(), vtx[nTx].GetHash()));

                    mapAddressTransactions[cAddress.GetHash256()] += vtx[nTx].vout[nOut].nValue;

                    if(GetArg("-verbose", 0) >= 2)
                        printf("%s Credited %f Nexus | Balance : %f Nexus\n", cAddress.ToString().c_str(), (double)vtx[nTx].vout[nOut].nValue / COIN, (double)mapAddressTransactions[cAddress.GetHash256()] / COIN);
                }

                if(!vtx[nTx].IsCoinBase())
                {
                    BOOST_FOREACH(const CTxIn& txin, vtx[nTx].vin)
                    {
                        if(txin.prevout.IsNull())
                            continue;

                        if(txin.IsStakeSig())
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

                        if(!Core::mapRichList.count(cAddress.GetHash256()))
                            mapRichList[cAddress.GetHash256()] = { std::make_pair(vtx[nTx].IsCoinBase(), tx.GetHash()) };
                        else
                            mapRichList[cAddress.GetHash256()].push_back(std::make_pair(vtx[nTx].IsCoinBase(), tx.GetHash()));

                        mapAddressTransactions[cAddress.GetHash256()] = std::max((uint64)0, mapAddressTransactions[cAddress.GetHash256()] - tx.vout[txin.prevout.n].nValue);

                        if(GetArg("-verbose", 0) >= 2)
                            printf("%s Debited %f Nexus | Balance : %f Nexus\n", cAddress.ToString().c_str(), (double)tx.vout[txin.prevout.n].nValue / COIN, (double)mapAddressTransactions[cAddress.GetHash256()] / COIN);
                    }
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


    /* AddToBlockIndex: Adds a new Block into the Block Index.
        This is where it is categorized and dealt with in the Blockchain. */
    bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos)
    {
        /* Check for Duplicate. */
        uint1024 hash = GetHash();
        if (mapBlockIndex.count(hash))
            return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());


        /* Build new Block Index Object. */
        CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, *this);
        if (!pindexNew)
            return error("AddToBlockIndex() : new CBlockIndex failed");


        /* Find Previous Block. */
        pindexNew->phashBlock = &hash;
        map<uint1024, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);
        if (miPrev != mapBlockIndex.end())
            pindexNew->pprev = (*miPrev).second;


        /* Compute the Chain Trust */
        pindexNew->nChainTrust = (pindexNew->pprev ? pindexNew->pprev->nChainTrust : 0) + pindexNew->GetBlockTrust();


        /* Compute the Channel Height. */
        const CBlockIndex* pindexPrev = GetLastChannelIndex(pindexNew->pprev, pindexNew->GetChannel());
        pindexNew->nChannelHeight = (pindexPrev ? pindexPrev->nChannelHeight : 0) + 1;


        /* Compute the Released Reserves. */
        for(int nType = 0; nType < 3; nType++)
        {
            if(pindexNew->IsProofOfWork() && pindexPrev)
            {
                /* Calculate the Reserves from the Previous Block in Channel's reserve and new Release. */
                int64 nReserve  = pindexPrev->nReleasedReserve[nType] + GetReleasedReserve(pindexNew, pindexNew->GetChannel(), nType);

                /* Block Version 3 Check. Disable Reserves from going below 0. */
                if(pindexNew->nVersion >= 3 && pindexNew->nCoinbaseRewards[nType] >= nReserve)
                    return error("AddToBlockIndex() : Coinbase Transaction too Large. Out of Reserve Limits");

                pindexNew->nReleasedReserve[nType] =  nReserve - pindexNew->nCoinbaseRewards[nType];

                if(GetArg("-verbose", 0) >= 2)
                    printf("Reserve Balance %i | %f Nexus | Released %f\n", nType, pindexNew->nReleasedReserve[nType] / 1000000.0, (nReserve - pindexPrev->nReleasedReserve[nType]) / 1000000.0 );
            }
            else
                pindexNew->nReleasedReserve[nType] = 0;

        }

        /* Add the Pending Checkpoint into the Blockchain. */
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

        /* Add to the MapBlockIndex */
        map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        /* Write the new Block to Disk. */
        LLD::CIndexDB indexdb("r+");
        indexdb.TxnBegin();
        indexdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));

        /* Set the Best chain if Highest Trust. */
        if (pindexNew->nChainTrust > nBestChainTrust)
            if (!SetBestChain(indexdb, pindexNew))
                return false;

        /** Commit the Transaction to the Database. **/
        if(!indexdb.TxnCommit())
            return error("CBlock::AddToBlockIndex() : Failed to Commit Transaction to the Database.");

        if (pindexNew == pindexBest)
        {
            /* Relay the Block to Nexus Network. */
            if (!IsInitialBlockDownload())
            {
                LOCK(Net::cs_vNodes);
                for(auto pnode : Net::vNodes)
                    pnode->PushInventory(Net::CInv(Net::MSG_BLOCK, hash));

            }
            else
            {
                LOCK(Net::cs_vNodes);
                for(auto pnode : Net::vNodes)
                    pnode->nLastGetBlocks = GetUnifiedTimestamp();

            }

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
            if(nVersion >= 5 && ProofHash() < bnPrimeMinOrigins.getuint1024())
                return error("VerifyWork() : prime origins below 1016-bits");

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
        if ((nVersion < 5 ? GetHash() : ProofHash()) > bnTarget.getuint1024())
            return error("VerifyWork() : proof-of-work hash below target");

        return true;
    }

    /** Verify the Signature is Valid for Last 2 Coinbase Tx Outputs. **/
    bool VerifyAddress(const std::vector<unsigned char> script, const std::vector<unsigned char> sig)
    {
        if(script.size() != 37)
            return error("Script Size not 37 Bytes");

        for(int i = 0; i < 37; i++)
            if(script[i] != sig[i])
                return false;

        return true;
    }

    /** Compare Two Vectors Element by Element. **/
    bool VerifyAddressList(const std::vector<unsigned char> script, const std::vector<unsigned char> sigs[13])
    {
        for(int i = 0; i < 13; i++)
            if(VerifyAddress(script, sigs[i]))
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


        /** Check that the time was within range. */
        if (GetBlockTime() > GetUnifiedTimestamp() + MAX_UNIFIED_DRIFT)
            return error("AcceptBlock() : block timestamp too far in the future");


        /** Do not allow blocks to be accepted above the Current Blo>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
            vCoins.push_back(&(*it).ck Version. **/
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
            return error("CheckBlock() : Block Created before Channel Time-Lock. Channel Opens in %" PRId64 " Seconds", (fTestNet ? CHANNEL_TESTNET_TIMELOCK[GetChannel()] : CHANNEL_NETWORK_TIMELOCK[GetChannel()]) - GetUnifiedTimestamp());


        /** Check the Current Version Block Time-Lock. Allow Version (Current -1) Blocks for 1 Hour after Time Lock. **/
        if (nVersion > 1 && nVersion == (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION - 1 : NETWORK_BLOCK_CURRENT_VERSION - 1) && (GetBlockTime() - 3600) > (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
            return error("CheckBlock() : Version %u Blocks have been Obsolete for %" PRId64 " Seconds\n", nVersion, (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2])));


        /** Check the Current Version Block Time-Lock. **/
        if (nVersion >= (fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION) && GetBlockTime() <= (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
            return error("CheckBlock() : Version %u Blocks are not Accepted for %" PRId64 " Seconds\n", nVersion, (GetUnifiedTimestamp() - (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2])));


        /** Check the Required Mining Outputs. **/
        if (IsProofOfWork() && nVersion >= 3) {
            unsigned int nSize = vtx[0].vout.size();

            /** Check the Coinbase Tx Size. **/
            if(nSize < 3)
                return error("CheckBlock() : Coinbase Too Small.");

            if(!fTestNet)
            {
                if (!VerifyAddressList(vtx[0].vout[nSize - 2].scriptPubKey, (nVersion < 5) ? AMBASSADOR_SCRIPT_SIGNATURES : AMBASSADOR_SCRIPT_SIGNATURES_RECYCLED))
                    return error("CheckBlock() : Block %u Ambassador Signature Not Verified.\n", nHeight);

                if (!VerifyAddressList(vtx[0].vout[nSize - 1].scriptPubKey, (nVersion < 5) ? DEVELOPER_SCRIPT_SIGNATURES : DEVELOPER_SCRIPT_SIGNATURES_RECYCLED))
                    return error("CheckBlock() :  Block %u Developer Signature Not Verified.\n", nHeight);
            }

            else
            {
                if (!VerifyAddress(vtx[0].vout[nSize - 2].scriptPubKey, (nVersion < 5) ? TESTNET_DUMMY_SIGNATURE : TESTNET_DUMMY_SIGNATURE_AMBASSADOR_RECYCLED))
                    return error("CheckBlock() :  Block %u Ambassador Signature Not Verified.\n", nHeight);

                if (!VerifyAddress(vtx[0].vout[nSize - 1].scriptPubKey, (nVersion < 5) ? TESTNET_DUMMY_SIGNATURE : TESTNET_DUMMY_SIGNATURE_DEVELOPER_RECYCLED))
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

        /* Ensure the Block is for Proof of Stake Only. */
        if(IsProofOfStake())
        {

            /* Check the Coinstake Time is before Unified Timestamp. */
            if(vtx[0].nTime > (GetUnifiedTimestamp() + MAX_UNIFIED_DRIFT))
                return error("CheckBlock() : Coinstake Transaction too far in Future.");

            /* Make Sure Coinstake Transaction is First. */
            if (!vtx[0].IsCoinStake())
                return error("CheckBlock() : First transaction non-coinstake %s", vtx[0].GetHash().ToString().c_str());

            /* Make Sure Coinstake Transaction Time is Before Block. */
            if (vtx[0].nTime > nTime)
                return error("CheckBlock()  : Coinstake Timestamp to is ahead of block time");

        }

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


        /* Check the Block Signature. */
        if (!CheckBlockSignature())
            return DoS(100, error("CheckBlock() : bad block signature"));

        return true;
    }


    bool CBlock::AcceptBlock()
    {
        /* Check for Duplicate Block. */
        uint1024 hash = GetHash();
        if (mapBlockIndex.count(hash))
            return error("AcceptBlock() : block already in mapBlockIndex");


        /* Find the Previous block from hashPrevBlock. */
        map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
        if (mi == mapBlockIndex.end())
            return DoS(10, error("AcceptBlock() : prev block not found"));
        CBlockIndex* pindexPrev = (*mi).second;
        int nPrevHeight = pindexPrev->nHeight + 1;


        /* Check the Height of Block to Previous Block. */
        if(nPrevHeight != nHeight)
            return DoS(100, error("AcceptBlock() : incorrect block height."));


        /* Loggging of the proof hashes. */
        if(GetArg("-verbose", 0) >= 2)
        {
            /* Get the proof hash for this block. */
            uint1024 hash = (nVersion < 5 ? GetHash() : GetChannel() == 0 ? StakeHash() : ProofHash());

            /* Get the target hash for this block. */
            uint1024 hashTarget = CBigNum().SetCompact(nBits).getuint1024();

            /* Verbose logging of proof and target. */
            printf("  proof:  %s  \n", hash.ToString().substr(0, 30).c_str());

            /* Channel switched output. */
            if(GetChannel() == 1)
                printf("  prime cluster verified of size %f\n", GetDifficulty(nBits, 1));
            else
                printf("  target: %s\n", hashTarget.ToString().substr(0, 30).c_str());
        }


        /* Check that the nBits match the current Difficulty. **/
        if (nBits != GetNextTargetRequired(pindexPrev, GetChannel(), !IsInitialBlockDownload()))
            return DoS(100, error("AcceptBlock() : incorrect proof-of-work/proof-of-stake"));


        /* Check That Block Timestamp is not before previous block. */
        if (GetBlockTime() <= pindexPrev->GetBlockTime())
            return error("AcceptBlock() : block's timestamp too early Block: %" PRId64 " Prev: %" PRId64 "", GetBlockTime(), pindexPrev->GetBlockTime());


        /* Check that Block is Descendant of Hardened Checkpoints. */
        if(!IsInitialBlockDownload() && pindexPrev && !IsDescendant(pindexPrev))
            return error("AcceptBlock() : Not a descendant of Last Checkpoint");


        /* Check the Coinbase Transactions in Block Version 3. */
        if(IsProofOfWork() && nHeight > 0 && nVersion >= 3)
        {
            unsigned int nSize = vtx[0].vout.size();

            /* Add up the Miner Rewards from Coinbase Tx Outputs. */
            int64 nMiningReward = 0;
            for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
                nMiningReward += vtx[0].vout[nIndex].nValue;

            /* Check that the Mining Reward Matches the Coinbase Calculations. */
            if (round_coin_digits(nMiningReward, 3) != round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 0), 3))
                return error("AcceptBlock() : miner reward mismatch %" PRId64 " : %" PRId64 "", round_coin_digits(nMiningReward, 3), round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 0), 3));

            /* Check that the Exchange Reward Matches the Coinbase Calculations. */
            if (round_coin_digits(vtx[0].vout[nSize - 2].nValue, 3) != round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 1), 3))
                return error("AcceptBlock() : exchange reward mismatch %" PRId64 " : %" PRId64 "", round_coin_digits(vtx[0].vout[nSize - 2].nValue, 3), round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 1), 3));

            /* Check that the Developer Reward Matches the Coinbase Calculations. */
            if (round_coin_digits(vtx[0].vout[nSize - 1].nValue, 3) != round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 2), 3))
                return error("AcceptBlock() : developer reward mismatch %" PRId64 " : %" PRId64 "", round_coin_digits(vtx[0].vout[nSize - 1].nValue, 3), round_coin_digits(GetCoinbaseReward(pindexPrev, GetChannel(), 2), 3));

        }

        /* Check the Proof of Stake Claims. */
        else if (IsProofOfStake())
        {
            /* Check that the Coinbase / CoinstakeTimstamp is after Previous Block. */
            if (vtx[0].nTime < pindexPrev->GetBlockTime())
                return error("AcceptBlock() : coinstake transaction too early");

            /* Check the claimed stake limits are met. */
            if(nVersion >= 5 && !CheckStake())
                return DoS(50, error("ConnectBlock() : invalid proof of stake"));
        }


        /* Check that Transactions are Finalized. */
        BOOST_FOREACH(const CTransaction& tx, vtx)
            if (!tx.IsFinal(nHeight, GetBlockTime()))
                return DoS(10, error("AcceptBlock() : contains a non-final transaction"));


        /* Write new Block to Disk. */
        if (!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK, DATABASE_VERSION)))
            return error("AcceptBlock() : out of disk space");

        unsigned int nFile = -1;
        unsigned int nBlockPos = 0;
        if (!WriteToDisk(nFile, nBlockPos))
            return error("AcceptBlock() : WriteToDisk failed");
        if (!AddToBlockIndex(nFile, nBlockPos))
            return error("AcceptBlock() : AddToBlockIndex failed");

        return true;
    }


    bool ProcessBlock(Net::CNode* pfrom, CBlock* pblock)
    {
        //check if this is the corrupted block that needs rewrite
        uint1024 hash = pblock->GetHash();
        if(LLD::hashCorruptedNext != 0 && hash == LLD::hashCorruptedNext)
        {
            printf("ProcessBlock() : Detected corrupted pnext %s... Fixing\n", LLD::hashCorruptedNext.ToString().substr(0, 20).c_str());

            /* Get the index database. */
            LLD::CIndexDB indexdb("r+");

            /* Disconnect the block since corrupted pnext usually leave transaction indexes hanging.
               This will reconnect this block right after this method executes. */
            pblock->DisconnectBlock(indexdb, NULL);

            //reset the corrupted next to 0
            LLD::hashCorruptedNext = 0;
        }

        // Check for duplicate
        if (mapBlockIndex.count(hash))
        {
            return error("ProcessBlock() : already have block %d %s", mapBlockIndex[hash]->nHeight, hash.ToString().substr(0,20).c_str());
        }

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

            if(pfrom)
            {
                if (IsInitialBlockDownload())
                    pfrom->PushGetBlocks(pindexBest, 0);
                else
                    pfrom->AskFor(Net::CInv(Net::MSG_BLOCK, pblock->hashPrevBlock));
            }

            return true;
        }

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

        printg("ProcessBlock: ACCEPTED %s\n", pblock->GetHash().ToString().substr(0, 20).c_str());

        return true;
    }


    bool LastTrustBlock(CTrustKey trustKey, uint1024& hashTrustBlock)
    {
        /* Block Object. */
        CBlock block;

        /* Trust key of said block. */
        std::vector<unsigned char> vTrustKey;

        /* Loop through all previous blocks looking for most recent trust block. */
        while(vTrustKey != trustKey.vchPubKey)
        {
            /* Check map block index for trust block. */
            if(!mapBlockIndex.count(hashTrustBlock))
                return error("LastTrustBlock() : Couldn't find last trust block in mapblockindex");

            /* Get the last trust block. */
            const CBlockIndex* pindex = GetLastChannelIndex(mapBlockIndex[hashTrustBlock]->pprev, 0);
            if(!pindex || !pindex->pprev)
                return error("LastTrustBlock() : couldn't find index");

            /* Check for genesis. */
            if(pindex->GetBlockHash() == trustKey.hashGenesisBlock)
            {
                hashTrustBlock = pindex->GetBlockHash();
                return true;
            }

            /* If serach block isn't proof of stake, return an error. */
            if(!pindex->IsProofOfStake())
                return error("LastTrustBlock() : not proof of stake");

            /* Read the previous block from disk. */
            if(!block.ReadFromDisk(pindex, true))
                return error("LastTrustBlock() : can't read trust block");

            /* Get the trust key from block. */
            if(!block.TrustKey(vTrustKey))
                return error("LastTrustBlock() : can't get trust key");

            /* Set current trust block in recursion. */
            hashTrustBlock = pindex->GetBlockHash();
        }

        return true;
    }


    bool FindGenesis(uint576 cKey, CTrustKey& trustKey, uint1024& hashTrustBlock)
    {
        /* Block Object. */
        CBlock block;

        /* Trust key of said block. */
        std::vector<unsigned char> vTrustKey;

        /* The key to test for. */
        uint576 keyTest;

        /* Loop through all previous blocks looking for most recent trust block. */
        while(hashTrustBlock != hashGenesisBlockOfficial)
        {
            /* Check map block index for trust block. */
            if(!mapBlockIndex.count(hashTrustBlock))
                return error("FindGenesis() : Couldn't find last trust block in mapblockindex");

            /* Get the last trust block. */
            const CBlockIndex* pindex = GetLastChannelIndex(mapBlockIndex[hashTrustBlock]->pprev, 0);
            if(!pindex || !pindex->pprev)
                return error("FindGenesis() : couldn't find index");

            /* If serach block isn't proof of stake, return an error. */
            if(!pindex->IsProofOfStake())
                return error("FindGenesis() : not proof of stake");

            /* Read the previous block from disk. */
            if(!block.ReadFromDisk(pindex, true))
                return error("FindGenesis() : can't read trust block");

            /* Get the trust key from block. */
            if(!block.TrustKey(vTrustKey))
                return error("FindGenesis() : can't get trust key");

            /* Check for genesis. */
            keyTest.SetBytes(vTrustKey);
            if(keyTest == cKey && block.vtx[0].IsGenesis())
            {
                printf("FindGenesis() : Found Genesis (%s). Restoring trust key.\n", block.GetHash().ToString().substr(0, 20).c_str());

                trustKey = Core::CTrustKey(vTrustKey, block.GetHash(), block.vtx[0].GetHash(), block.nTime);

                return true;
            }

            /* Set current trust block in recursion. */
            hashTrustBlock = pindex->GetBlockHash();
        }

        return false;
    }


    bool CBlock::CheckStake()
    {
        /* Check the proof hash of the stake block on version 5 and above. */
        CBigNum bnTarget;
        bnTarget.SetCompact(nBits);
        if(StakeHash() > bnTarget.getuint1024())
            return error("CBlock::CheckStake() : Proof of Stake Hash not meeting Target.");

        /* Weight for Trust transactions combine block weight and stake weight. */
        double nTrustWeight = 0.0, nBlockWeight = 0.0;
        unsigned int nTrustAge = 0, nBlockAge = 0;
        if(vtx[0].IsTrust())
        {

            /* Get the score and make sure it all checks out. */
            if(!TrustScore(nTrustAge))
                return error("CBlock::CheckStake() : failed to get trust score");

            /* Get the weights with the block age. */
            if(!BlockAge(nBlockAge))
                return error("CBlock::CheckStake() : failed to get block age");

            /* Trust Weight Continues to grow the longer you have staked and higher your interest rate */
            nTrustWeight = min(90.0, (((44.0 * log(((2.0 * nTrustAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);

            /* Block Weight Reaches Maximum At Trust Key Expiration. */
            nBlockWeight = min(10.0, (((9.0 * log(((2.0 * nBlockAge) / ((fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))) + 1.0)) / log(3))) + 1.0);

        }

        /* Weight for Gensis transactions are based on your coin age. */
        else
        {
            /* Genesis transaction can't have any transactions. */
            if(vtx.size() != 1)
                return error("CBlock::CheckStake() : Genesis can't include transactions");

            /* Calculate the Average Coinstake Age. */
            LLD::CIndexDB indexdb("r");
            uint64 nCoinAge;
            if(!vtx[0].GetCoinstakeAge(indexdb, nCoinAge))
                return error("CBlock::CheckStake() : Failed to Get Coinstake Age.");

            /* Genesis has to wait for one full trust key timespan. */
            if(nCoinAge < (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
                return error("CBlock::CheckStake() : Genesis age is immature");

            /* Trust Weight For Genesis Transaction Reaches Maximum at 90 day Limit. */
            nTrustWeight = min(10.0, (((9.0 * log(((2.0 * nCoinAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);
        }

        /* Check the energy efficiency requirements. */
        double nThreshold = ((nTime - vtx[0].nTime) * 100.0) / nNonce;
        double nRequired  = ((108.0 - nTrustWeight - nBlockWeight) * MAX_STAKE_WEIGHT) / vtx[0].vout[0].nValue;
        if(nThreshold < nRequired)
            return error("CBlock::CheckStake() : energy threshold too low (%f) required (%f)", nThreshold, nRequired);

        /* Verbose logging. */
        if(GetArg("-verbose", 0) >= 2)
        {
            printf("CheckStake : hash=%s target=%s trustscore=%u blockage=%u trustweight=%f blockweight=%f threshold=%f required=%f time=%u nonce=%" PRIu64 "\n", StakeHash().ToString().substr(0, 20).c_str(), bnTarget.getuint1024().ToString().substr(0, 20).c_str(), nTrustAge, nBlockAge, nTrustWeight, nBlockWeight, nThreshold, nRequired, (unsigned int)(nTime - vtx[0].nTime), nNonce);
        }

        return true;
    }


    bool CBlock::ExtractTrust(uint1024& hashLastBlock, unsigned int& nSequence, unsigned int& nTrustScore)
    {
        /* Don't extract trust if not coinstake. */
        if(!IsProofOfStake())
            return error("CBlock::ExtractTrust() : not proof of stake");

        /* Check the script size matches expected length. */
        if(vtx[0].vin[0].scriptSig.size() != 144)
            return error("CBlock::ExtractTrust() : script not 144 (%u) bytes", vtx[0].vin[0].scriptSig.size());

        /* Put script in deserializing stream. */
        CDataStream scriptPub(vtx[0].vin[0].scriptSig, SER_NETWORK, PROTOCOL_VERSION);

        /* Erase the first 8 bytes of the fib byte series flag. */
        scriptPub.erase(scriptPub.begin(), scriptPub.begin() + 8);

        /* Deserialize the values from stream. */
        scriptPub >> hashLastBlock >> nSequence >> nTrustScore;

        return true;
    }


    bool CBlock::BlockAge(unsigned int& nAge)
    {
        /* No age for non proof of stake or non version 5 blocks */
        if(!IsProofOfStake() || nVersion < 5)
            return error("CBlock::TrustScore() : not proof of stake / version < 5");

        /* Genesis has a age 0. */
        if(vtx[0].IsGenesis())
        {
            nAge = 0;
            return true;
        }

        /* Version 5 - last trust block. */
        uint1024 hashLastBlock;
        unsigned int nSequence;
        unsigned int nTrustScore;

        /* Extract values from coinstake vin. */
        if(!ExtractTrust(hashLastBlock, nSequence, nTrustScore))
            return error("CBlock::BlockAge() : failed to extract values from script");

        /* Check that the last block is in the block index. */
        if(!mapBlockIndex.count(hashLastBlock))
            return error("CBlock::BlockAge() : previous block (%s) not in block index", hashLastBlock.ToString().substr(0, 20).c_str());

        /* Read the previous block from disk. */
        nAge = mapBlockIndex[hashPrevBlock]->GetBlockTime() - mapBlockIndex[hashLastBlock]->GetBlockTime();

        return true;
    }

    bool CBlock::TrustScore(unsigned int& nScore)
    {
        /* Genesis has a trust score of 0. */
        if(vtx[0].IsGenesis())
        {
            nScore = 0;
            return true;
        }

        /* Version 5 - last trust blocks. */
        uint1024 hashLastBlock;
        unsigned int nSequence;
        unsigned int nTrustScore;

        /* Extract values from coinstake vin. */
        if(!ExtractTrust(hashLastBlock, nSequence, nTrustScore))
            return error("CBlock::TrustScore() : failed to extract trust");

        /* Return true with the trust score. */
        nScore = nTrustScore;
        return true;
    }


    /* Calculates the trust score of given trust key included in a block. */
    bool CBlock::CheckTrust()
    {
        /* No trust score for non proof of stake (for now). */
        if(!IsProofOfStake())
            return error("CBlock::CheckTrust() : not proof of stake");

        /* Extract the trust key from the coinstake. */
        uint576 cKey;
        if(!TrustKey(cKey))
            return error("CBlock::CheckTrust() : trust key not found in script");

        /* Genesis has a trust score of 0. */
        if(vtx[0].IsGenesis())
        {
            if(vtx[0].vin[0].scriptSig.size() != 8)
                return error("CBlock::CheckTrust() : genesis unexpected size %u", vtx[0].vin[0].scriptSig.size());

            return true;
        }

        /* Version 5 - last trust blocks. */
        uint1024 hashLastBlock;
        unsigned int nSequence;
        unsigned int nTrustScore;

        /* Extract values from coinstake vin. */
        if(!ExtractTrust(hashLastBlock, nSequence, nTrustScore))
            return error("CBlock::CheckTrust() : failed to extract trust");

        /* Check that the last block is in the block index. */
        if(!mapBlockIndex.count(hashLastBlock))
            return error("CBlock::CheckTrust() : previous block (%s) not in block index", hashLastBlock.ToString().substr(0, 20).c_str());

        /* Read the previous block from disk. */
        CBlock blockPrev;
        if(!blockPrev.ReadFromDisk(mapBlockIndex[hashLastBlock], true))
            return error("CBlock::CheckTrust() : can't read previous block");

        /* Extract the last trust key */
        uint576 keyLast;
        if(!blockPrev.TrustKey(keyLast))
            return error("CBlock::CheckTrust() : trust key not found in previous script");

        /* Enforce the minimum trust key interval of 120 blocks. */
        if(nHeight - blockPrev.nHeight < (fTestNet ? TESTNET_MINIMUM_INTERVAL : MAINNET_MINIMUM_INTERVAL))
            return error("CBlock::CheckTrust() : trust key interval below minimum interval %u", nHeight - blockPrev.nHeight);

        /* Ensure the last block being checked is the same trust key. */
        if(keyLast != cKey)
            return error("CBlock::CheckTrust() : trust key in previous block (%s) mismatch to this one (%s)", cKey.ToString().substr(0, 20).c_str(), keyLast.ToString().substr(0, 20).c_str());

        /* Placeholder in case previous block is a version 4 block. */
        unsigned int nScorePrev = 0, nScore = 0;

        /* If previous block is genesis, set previous score to 0. */
        if(blockPrev.vtx[0].IsGenesis())
        {
            /* Enforce sequence number 1 if previous block was genesis */
            if(nSequence != 1)
                return error("CBlock::CheckTrust() : first trust block and sequence is not 1 (%u)", nSequence);

            /* Genesis results in a previous score of 0. */
            nScorePrev = 0;
        }

        /* Version 4 blocks need to get score from previous blocks calculated score from the trust pool. */
        else if(blockPrev.nVersion < 5)
        {
            /* Establish the index database. */
            LLD::CIndexDB indexdb("r+");

            /* Check the trust pool - this should only execute once transitioning from version 4 to version 5 trust keys. */
            CTrustKey trustKey;
            if(!indexdb.ReadTrustKey(cKey, trustKey))
            {
                if(!FindGenesis(cKey, trustKey, hashPrevBlock))
                    return error("CBlock::CheckTrust() : trust key not found in database");

                indexdb.WriteTrustKey(cKey, trustKey);
            }

            /* Enforce sequence number of 1 for anything made from version 4 blocks. */
            if(nSequence != 1)
                return error("CBlock::CheckTrust() : version 4 block sequence number not 1 (%u)", nSequence);

            /* Ensure that a version 4 trust key is not expired based on new timespan rules. */
            if(trustKey.Expired(hashPrevBlock, (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN)))
                return error("CBlock::CheckTrust() : version 4 key expired.");

            /* Score is the total age of the trust key for version 4. */
            nScorePrev = trustKey.Age(mapBlockIndex[hashPrevBlock]->GetBlockTime());
        }

        /* Version 5 blocks that are trust must pass sequence checks. */
        else
        {
            /* The last block of previous. */
            uint1024 hashBlockPrev; //dummy variable unless we want to do recursive checking of scores all the way back to genesis

            /* Extract the value from the previous block. */
            unsigned int nSequencePrev;
            if(!blockPrev.ExtractTrust(hashBlockPrev, nSequencePrev, nScorePrev))
                return error("CBlock::CheckTrust() : failed to extract trust");

            /* Enforce Sequence numbering, must be +1 always. */
            if(nSequence != nSequencePrev + 1)
                return error("CBlock::CheckTrust() : previous sequence (%u) broken (%u)", nSequencePrev, nSequence);
        }

        /* The time it has been since the last trust block for this trust key. */
        int nTimespan = (mapBlockIndex[hashPrevBlock]->GetBlockTime() - blockPrev.nTime);

        /* Timespan less than required timespan is awarded the total seconds it took to find. */
        if(nTimespan < (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
            nScore = nScorePrev + nTimespan;

        /* Timespan more than required timespan is penalized 3 times the time it took past the required timespan. */
        else
        {
            /* Calculate the penalty for score (3x the time). */
            int nPenalty = (nTimespan - (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN)) * 3;

            /* Catch overflows and zero out if penalties are greater than previous score. */
            if(nPenalty > nScorePrev)
                nScore = 0;
            else
                nScore = (nScorePrev - nPenalty);
        }

        /* Set maximum trust score to seconds passed for interest rate. */
        if(nScore > (60 * 60 * 24 * 28 * 13))
            nScore = (60 * 60 * 24 * 28 * 13);

        /* Debug output. */
        if(GetArg("-verbose", 0) >= 2)
            printf("CheckTrust: score=%u prev=%u timespan=%u change=%i\n", nScore, nScorePrev, nTimespan, (int)(nScore - nScorePrev));

        /* Check that published score in this block is equivilent to calculated score. */
        if(nTrustScore != nScore)
            return error("CBlock::CheckTrust() : published trust score (%u) not meeting calculated score (%u)", nTrustScore, nScore);

        return true;
    }


    bool CBlock::TrustKey(uint576& cKey)
    {
        /* Extract the Key from the Script Signature. */
        vector<std::vector<unsigned char> > vSolutions;
        Wallet::TransactionType whichType;
        const CTxOut& txout = vtx[0].vout[0];

        /* Extract the key from script sig. */
        if (!Solver(txout.scriptPubKey, whichType, vSolutions))
            return error("CBlock::TrustKey() : Couldn't find trust key in script");

        /* Enforce public key rules. */
        if (whichType != Wallet::TX_PUBKEY)
            return error("CBlock::TrustKey() : key not of public key type");

        /* Set the Public Key Integer Key from Bytes. */
        cKey.SetBytes(vSolutions[0]);

        return true;
    }


    bool CBlock::TrustKey(std::vector<unsigned char>& vchTrustKey)
    {
        /* Extract the Key from the Script Signature. */
        vector<std::vector<unsigned char> > vSolutions;
        Wallet::TransactionType whichType;
        const CTxOut& txout = vtx[0].vout[0];

        /* Extract the key from script sig. */
        if (!Solver(txout.scriptPubKey, whichType, vSolutions))
            return error("CBlock::TrustKey() : Couldn't find trust key in script");

        /* Enforce public key rules. */
        if (whichType != Wallet::TX_PUBKEY)
            return error("CBlock::TrustKey() : key not of public key type");

        /* Set the Public Key Integer Key from Bytes. */
        vchTrustKey = vSolutions[0];

        return true;
    }

    /* New proof hash for all channels (version > 5) */
    uint1024 CBlock::StakeHash()
    {
        /* Get the trust key. */
        uint576 cKey;
        TrustKey(cKey);

        //nVersion, hashPrevBlock, nChannel, nHeight, nBits, nOnce, vchTrustKey (extracted from coinstake)
        return SerializeHash(nVersion, hashPrevBlock, nChannel, nHeight, nBits, cKey, nNonce);
    }

    /* New proof hash for all channels (version > 5) */
    uint1024 CBlock::ProofHash() const
    {
        if(GetChannel() == 1)
            return SK1024(BEGIN(nVersion), END(nBits));

        return SK1024(BEGIN(nVersion), END(nNonce));
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
            return key.Sign((nVersion == 4) ? SignatureHash() : GetHash(), vchBlockSig, 1024);
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
            return key.Verify((nVersion == 4) ? SignatureHash() : GetHash(), vchBlockSig, 1024);
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

        FILE* file = fopen((GetDataDir() / strprintf("blk%04u.dat", nFile)).string().c_str(), pszMode);
        if (!file)
            return NULL;

        if (nBlockPos != 0 && !strchr(pszMode, 'a'))
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
        loop() {
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
            nCoinbaseMaturity = 5;

            TRUST_KEY_EXPIRE = 60 * 60 * 12;
            TRUST_KEY_MIN_INTERVAL = 5;
        }

        if(GetArg("-verbose", 0) >= 0)
            printf("%s Network: genesis=0x%s nBitsLimit=0x%08x nBitsInitial=0x%08x nCoinbaseMaturity=%d\n",
            fTestNet? "Test" : "Nexus", hashGenesisBlock.ToString().substr(0, 20).c_str(), bnProofOfWorkLimit[0].GetCompact(), bnProofOfWorkStart[0].GetCompact(), nCoinbaseMaturity);

        /** Initialize Block Index Database. **/
        LLD::CIndexDB indexdb("r+");
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

            assert(txNew.nTime == block.nTime);

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
