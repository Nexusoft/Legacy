/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "unifiedtime.h"

#include "../wallet/db.h"
#include "../LLD/index.h"
#include "../LLP/miningserver.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost;

namespace Core
{

    /** Used to Iterate the Coinbase Addresses used For Exchange Channels and Developer Fund. **/
    static boost::mutex PROCESS_MUTEX;

    class COrphan
    {
    public:
        CTransaction* ptx;
        set<uint512> setDependsOn;
        double dPriority;

        COrphan(CTransaction* ptxIn)
        {
            ptx = ptxIn;
            dPriority = 0;
        }

        void print() const
        {
            printf("COrphan(hash=%s, dPriority=%.1f)\n", ptx->GetHash().ToString().substr(0,10).c_str(), dPriority);
            BOOST_FOREACH(uint512 hash, setDependsOn)
                printf("   setDependsOn %s\n", hash.ToString().substr(0,10).c_str());
        }
    };


    /** Constructs a new block **/
    CBlock CreateNewBlock(Wallet::CReserveKey& reservekey, Wallet::CWallet* pwallet, unsigned int nChannel, unsigned int nID, LLP::Coinbase* pCoinbase)
    {
        CBlock cBlock;
        cBlock.SetNull();

        /** Create the block from Previous Best Block. **/
        CBlockIndex* pindexPrev = pindexBest;

        /** Modulate the Block Versions if they correspond to their proper time stamp **/
        if(GetUnifiedTimestamp() >= (fTestNet ? TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] : NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
            cBlock.nVersion = fTestNet ? TESTNET_BLOCK_CURRENT_VERSION : NETWORK_BLOCK_CURRENT_VERSION; // --> New Block Versin Activation Switch
        else
            cBlock.nVersion = fTestNet ? TESTNET_BLOCK_CURRENT_VERSION - 1 : NETWORK_BLOCK_CURRENT_VERSION - 1;

        /** Create the Coinbase / Coinstake Transaction. **/
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vin[0].prevout.SetNull();

        /** Set the First output to Reserve Key. **/
        txNew.vout.resize(1);


        /** Create the Coinstake Transaction if on Proof of Stake Channel. **/
        if (nChannel == 0)
        {
            /** Mark the Coinstake Transaction with First Input Byte Signature. **/
            txNew.vin[0].scriptSig.resize(8);
            txNew.vin[0].scriptSig[0] = 1;
            txNew.vin[0].scriptSig[1] = 2;
            txNew.vin[0].scriptSig[2] = 3;
            txNew.vin[0].scriptSig[3] = 5;
            txNew.vin[0].scriptSig[4] = 8;
            txNew.vin[0].scriptSig[5] = 13;
            txNew.vin[0].scriptSig[6] = 21;
            txNew.vin[0].scriptSig[7] = 34;

            /** Update the Coinstake Timestamp. **/
            txNew.nTime = pindexPrev->GetBlockTime() + 1;
        }

        /** Create the Coinbase Transaction if the Channel specifies. **/
        else
        {
            /** Set the Coinbase Public Key. **/
            txNew.vout[0].scriptPubKey << reservekey.GetReservedKey() << Wallet::OP_CHECKSIG;

            /** Set the Proof of Work Script Signature. **/
            txNew.vin[0].scriptSig = (Wallet::CScript() << nID * 513513512151);

            /** Customized Coinbase Transaction if Submitted. **/
            if(pCoinbase)
            {

                /** Dummy Transaction to Allow the Block to be Signed by Pool Wallet. [For Now] **/
                txNew.vout[0].nValue = pCoinbase->nPoolFee;

                unsigned int nTx = 1;
                txNew.vout.resize(pCoinbase->vOutputs.size() + 1);
                for(std::map<std::string, uint64>::iterator nIterator = pCoinbase->vOutputs.begin(); nIterator != pCoinbase->vOutputs.end(); nIterator++)
                {
                    /** Set the Appropriate Outputs. **/
                    txNew.vout[nTx].scriptPubKey.SetNexusAddress(nIterator->first);
                    txNew.vout[nTx].nValue = nIterator->second;

                    nTx++;
                }

                int64 nMiningReward = 0;
                for(int nIndex = 0; nIndex < txNew.vout.size(); nIndex++)
                    nMiningReward += txNew.vout[nIndex].nValue;

                /** Double Check the Coinbase Transaction Fits in the Maximum Value. **/
                if(nMiningReward != GetCoinbaseReward(pindexPrev, nChannel, 0))
                    return cBlock;

            }
            else
                txNew.vout[0].nValue = GetCoinbaseReward(pindexPrev, nChannel, 0);

            /* Make coinbase counter mod 13 of height. */
            int nCoinbaseCounter = pindexPrev->nHeight % 13;

            /** Set the Proper Addresses for the Coinbase Transaction. **/
            txNew.vout.resize(txNew.vout.size() + 2);
            txNew.vout[txNew.vout.size() - 2].scriptPubKey.SetNexusAddress(fTestNet ? (cBlock.nVersion < 5 ? TESTNET_DUMMY_ADDRESS : TESTNET_DUMMY_AMBASSADOR_RECYCLED) : (cBlock.nVersion < 5 ? CHANNEL_ADDRESSES[nCoinbaseCounter] : AMBASSADOR_ADDRESSES_RECYCLED[nCoinbaseCounter]));

            txNew.vout[txNew.vout.size() - 1].scriptPubKey.SetNexusAddress(fTestNet ? (cBlock.nVersion < 5 ? TESTNET_DUMMY_ADDRESS : TESTNET_DUMMY_DEVELOPER_RECYCLED) : (cBlock.nVersion < 5 ? DEVELOPER_ADDRESSES[nCoinbaseCounter] : DEVELOPER_ADDRESSES_RECYCLED[nCoinbaseCounter]));

            /* Set the Proper Coinbase Output Amounts for Recyclers and Developers. */
            txNew.vout[txNew.vout.size() - 2].nValue = GetCoinbaseReward(pindexPrev, nChannel, 1);
            txNew.vout[txNew.vout.size() - 1].nValue = GetCoinbaseReward(pindexPrev, nChannel, 2);
        }

        /** Add our Coinbase / Coinstake Transaction. **/
        cBlock.vtx.push_back(txNew);

        /** Add in the Transaction from Memory Pool only if it is not a Genesis. **/
        if(nChannel > 0)
            AddTransactions(cBlock.vtx, pindexPrev);

        /** Populate the Block Data. **/
        cBlock.hashPrevBlock  = pindexPrev->GetBlockHash();
        cBlock.hashMerkleRoot = cBlock.BuildMerkleTree();
        cBlock.nChannel       = nChannel;
        cBlock.nHeight        = pindexPrev->nHeight + 1;
        cBlock.nBits          = GetNextTargetRequired(pindexPrev, cBlock.GetChannel(), false);
        cBlock.nNonce         = 1;

        cBlock.UpdateTime();

        return cBlock;
    }


    void AddTransactions(std::vector<CTransaction>& vtx, CBlockIndex* pindexPrev)
    {
        /** Collect Memory Pool Transactions into Block. **/
        std::vector<CTransaction> vRemove;
        int64 nFees = 0;
        {
            LOCK2(cs_main, mempool.cs);
            LLD::CIndexDB indexdb("r");

            // Priority order to process transactions
            list<COrphan> vOrphan; // list memory doesn't move
            map<uint512, vector<COrphan*> > mapDependers;
            multimap<double, CTransaction*> mapPriority;
            for (map<uint512, CTransaction>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi)
            {
                CTransaction& tx = (*mi).second;
                if (tx.IsCoinBase() || tx.IsCoinStake() || !tx.IsFinal())
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Transaction Is Coinbase/Coinstake or Not Final %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    continue;
                }

                COrphan* porphan = NULL;
                double dPriority = 0;
                BOOST_FOREACH(const CTxIn& txin, tx.vin)
                {
                    // Read prev transaction
                    CTransaction txPrev;
                    CTxIndex txindex;
                    if (!txPrev.ReadFromDisk(indexdb, txin.prevout, txindex))
                    {
                        // Has to wait for dependencies
                        if (!porphan)
                        {
                            // Use list for automatic deletion
                            vOrphan.push_back(COrphan(&tx));
                            porphan = &vOrphan.back();
                        }

                        mapDependers[txin.prevout.hash].push_back(porphan);
                        porphan->setDependsOn.insert(txin.prevout.hash);

                        continue;
                    }
                    int64 nValueIn = txPrev.vout[txin.prevout.n].nValue;

                    // Read block header
                    int nConf = txindex.GetDepthInMainChain();

                    dPriority += (double) nValueIn * nConf;

                    if(GetArg("-verbose", 0) >= 3)
                        printf("priority     nValueIn=%-12" PRI64d " nConf=%-5d dPriority=%-20.1f\n", nValueIn, nConf, dPriority);
                }

                // Priority is sum(valuein * age) / txsize
                dPriority /= ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

                if (porphan)
                    porphan->dPriority = dPriority;
                else
                    mapPriority.insert(make_pair(-dPriority, &(*mi).second));

                if(GetArg("-verbose", 0) >= 3)
                {
                    printf("priority %-20.1f %s\n%s", dPriority, tx.GetHash().ToString().substr(0,10).c_str(), tx.ToString().c_str());
                    if (porphan)
                        porphan->print();
                }
            }

            // Collect transactions into block
            map<uint512, CTxIndex> mapTestPool;
            uint64 nBlockSize = 1000;
            uint64 nBlockTx = 0;
            int nBlockSigOps = 100;
            while (!mapPriority.empty())
            {
                // Take highest priority transaction off priority queue
                CTransaction& tx = *(*mapPriority.begin()).second;
                mapPriority.erase(mapPriority.begin());

                // Size limits
                unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
                if (nBlockSize + nTxSize >= MAX_BLOCK_SIZE_GEN)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Block Size Limits Reached on Transaction %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    continue;
                }

                // Legacy limits on sigOps:
                unsigned int nTxSigOps = tx.GetLegacySigOpCount();
                if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Too Many Legacy Signature Operations %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    continue;
                }


                // Timestamp limit
                if (tx.nTime > GetUnifiedTimestamp() + MAX_UNIFIED_DRIFT)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Transaction Time Too Far in Future %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    continue;
                }


                // Nexus: simplify transaction fee - allow free = false
                int64 nMinFee = tx.GetMinFee(nBlockSize, false, GMF_BLOCK);


                // Connecting shouldn't fail due to dependency on other memory pool transactions
                // because we're already processing them in order of dependency
                map<uint512, CTxIndex> mapTestPoolTmp(mapTestPool);
                MapPrevTx mapInputs;
                bool fInvalid;
                if (!tx.FetchInputs(indexdb, mapTestPoolTmp, false, true, mapInputs, fInvalid))
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Failed to get Inputs %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    vRemove.push_back(tx);
                    continue;
                }

                int64 nTxFees = tx.GetValueIn(mapInputs) - tx.GetValueOut();
                if (nTxFees < nMinFee)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Not Enough Fees %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    vRemove.push_back(tx);
                    continue;
                }

                nTxSigOps += tx.TotalSigOps(mapInputs);
                if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Too many P2SH Signature Operations %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    vRemove.push_back(tx);
                    continue;
                }

                if (!tx.ConnectInputs(indexdb, mapInputs, mapTestPoolTmp, CDiskTxPos(1,1,1), pindexPrev, false, true))
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("AddTransactions() : Failed to Connect Inputs %s\n", tx.GetHash().ToString().substr(0, 10).c_str());

                    vRemove.push_back(tx);
                    continue;
                }

                mapTestPoolTmp[tx.GetHash()] = CTxIndex(CDiskTxPos(1,1,1), tx.vout.size());
                swap(mapTestPool, mapTestPoolTmp);


                // Added
                vtx.push_back(tx);
                nBlockSize += nTxSize;
                ++nBlockTx;
                nBlockSigOps += nTxSigOps;
                nFees += nTxFees;


                // Add transactions that depend on this one to the priority queue
                uint512 hash = tx.GetHash();
                if (mapDependers.count(hash))
                {
                    BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
                    {
                        if (!porphan->setDependsOn.empty())
                        {
                            porphan->setDependsOn.erase(hash);
                            if (porphan->setDependsOn.empty())
                                mapPriority.insert(make_pair(-porphan->dPriority, porphan->ptx));
                        }
                    }
                }
            }

            nLastBlockTx = nBlockTx;
            nLastBlockSize = nBlockSize;

        }

        //BOOST_FOREACH(CTransaction& tx, vRemove)
        //{
            //printf("AddTransactions() : removed invalid tx %s from mempool\n", tx.GetHash().ToString().substr(0, 10).c_str());
            //mempool.remove(tx);
        //}
    }


    /** Work Check Before Submit. This checks the work as a miner, a lot more conservatively than the network will check it
        to ensure that you do not submit a bad block. **/
    bool CheckWork(CBlock* pblock, Wallet::CWallet& wallet, Wallet::CReserveKey& reservekey)
    {
        uint1024 hash = (pblock->nVersion < 5 ? pblock->GetHash() : pblock->GetChannel() == 0 ? pblock->StakeHash() : pblock->ProofHash());
        uint1024 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint1024();

        if(pblock->GetChannel() > 0 && !pblock->VerifyWork())
            return error("Nexus Miner : proof of work not meeting target.");

        if(pblock->GetChannel() == 0)
        {
            CBigNum bnTarget;
            bnTarget.SetCompact(pblock->nBits);

            if((pblock->nVersion < 5 ? pblock->GetHash() : pblock->StakeHash()) > bnTarget.getuint1024())
                return error("Nexus Miner : proof of stake not meeting target");
        }

        if(GetArg("-verbose", 0) >= 1)
        {
            printf("Nexus Miner: new %s block found\n", GetChannelName(pblock->GetChannel()).c_str());
            printf("  hash:   %s  \n", hash.ToString().substr(0, 30).c_str());
        }

        if(pblock->GetChannel() == 1)
            printf("  prime cluster verified of size %f\n", GetDifficulty(pblock->nBits, 1));
        else
            printf("  target: %s\n", hashTarget.ToString().substr(0, 30).c_str());

        printf("%s ", DateTimeStrFormat(GetUnifiedTimestamp()).c_str());
        {
            LOCK(cs_main);
            if (pblock->hashPrevBlock != hashBestChain)
                return error("Nexus Miner : generated block is stale");

            // Track how many getdata requests this block gets
            {
                LOCK(wallet.cs_wallet);
                wallet.mapRequestCount[pblock->GetHash()] = 0;
            }

            /* Print the newly found block. */
            pblock->print();

            /** Process the Block to see if it gets Accepted into Blockchain. **/
            if (!ProcessBlock(NULL, pblock))
                return error("Nexus Miner : ProcessBlock, block not accepted\n");

            /* Keep the Reserve Key only if it was used in a block. */
            reservekey.KeepKey();
        }

        return true;
    }



    string GetChannelName(int nChannel)
    {
        if(nChannel == 2)
            return "SK-1024";
        else if(nChannel == 1)
            return "Prime";

        return "Stake";
    }
}
