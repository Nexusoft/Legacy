/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

[Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include <boost/assign/list_of.hpp>

#include "core.h"
#include "unifiedtime.h"
#include "../wallet/db.h"
#include "../wallet/wallet.h"
#include "../main.h" //for pwalletmain

#include "../LLD/index.h"
#include "../LLD/trustkeys.h"

using namespace std;

/** Locate the Add Coinstake Inputs Method Here for access. **/
namespace Wallet
{

    bool CWallet::AddCoinstakeInputs(Core::CBlock& block)
    {

        /* Add Each Input to Transaction. */
        vector<const CWalletTx*> vInputs;
        vector<const CWalletTx*> vCoins;

        block.vtx[0].vout[0].nValue = 0;

        {
            LOCK(cs_wallet);

            vCoins.reserve(mapWallet.size());
            for (map<uint512, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
                vCoins.push_back(&(*it).second);
        }

        random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);
        for(auto pcoin : vCoins)
        {
            if (!pcoin->IsFinal() || pcoin->GetDepthInMainChain() < 6)
                continue;

            if ((pcoin->IsCoinBase() || pcoin->IsCoinStake()) && pcoin->GetBlocksToMaturity() > 0)
                continue;

            /* Do not add coins to Genesis block if age less than trust timestamp */
            if (block.vtx[0].IsGenesis() && (block.vtx[0].nTime - pcoin->nTime) < (fTestNet ? Core::TRUST_KEY_TIMESPAN_TESTNET : Core::TRUST_KEY_TIMESPAN))
                continue;

            for (unsigned int i = 0; i < pcoin->vout.size(); i++)
            {
                if (pcoin->IsSpent(i) || !IsMine(pcoin->vout[i]))
                    continue;

                if (pcoin->nTime > block.vtx[0].nTime)
                    continue;

                //if(block.vtx[0].vout[0].nValue > (nBalance - nReserveBalance))
                //    break;

                /* Stop adding Inputs if has reached Maximum Transaction Size. */
                unsigned int nBytes = ::GetSerializeSize(block.vtx[0], SER_NETWORK, PROTOCOL_VERSION);
                if (nBytes >= Core::MAX_BLOCK_SIZE_GEN / 5)
                    break;

                block.vtx[0].vin.push_back(Core::CTxIn(pcoin->GetHash(), i));
                vInputs.push_back(pcoin);

                /** Add the value to the first Output for Coinstake. **/
                block.vtx[0].vout[0].nValue += pcoin->vout[i].nValue;
            }
        }

        if(block.vtx[0].vin.size() == 1)
            return false;

        /** Set the Interest for the Coinstake Transaction. **/
        int64 nInterest;
        LLD::CIndexDB indexdb("cr");
        if(!block.vtx[0].GetCoinstakeInterest(block, indexdb, nInterest))
            return error("AddCoinstakeInputs() : Failed to Get Interest");

        block.vtx[0].vout[0].nValue += nInterest;

        /** Sign Each Input to Transaction. **/
        for(int nIndex = 0; nIndex < vInputs.size(); nIndex++)
        {
            if (!SignSignature(*this, *vInputs[nIndex], block.vtx[0], nIndex + 1))
                return error("AddCoinstakeInputs() : Unable to sign Coinstake Transaction Input.");

        }

        return true;
    }

}

namespace Core
{

    /** Check the Coinstake Transaction is within the rules applied for Proof of Stake. **/
    bool CBlock::VerifyStake()
    {
        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        if(!TrustKey(cKey))
            return error("CBlock::VerifySTake() : cannot extract trust key");

        /* Determine Trust Age if the Trust Key Exists. */
        uint64 nCoinAge = 0, nTrustAge = 0, nBlockAge = 0;
        double nTrustWeight = 0.0, nBlockWeight = 0.0;

        LLD::CIndexDB indexdb("r+");
        if(vtx[0].IsTrust())
        {
            CTrustKey trustKey;
            if(!indexdb.ReadTrustKey(cKey, trustKey))
            {
                if(!FindGenesis(cKey, trustKey, hashPrevBlock))
                    return error("CBlock::VerifyStake() : trust key doesn't exist");

                indexdb.WriteTrustKey(cKey, trustKey);
            }

            /* Check the genesis and trust timestamps. */
            if(trustKey.nGenesisTime > mapBlockIndex[hashPrevBlock]->GetBlockTime())
                return error("CBlock::VerifyStake() : Genesis Time cannot be after Trust Time.");

            nTrustAge = trustKey.Age(mapBlockIndex[hashPrevBlock]->GetBlockTime());
            nBlockAge = trustKey.BlockAge(hashPrevBlock);

            /** Trust Weight Reaches Maximum at 30 day Limit. **/
            nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nTrustAge) / (60 * 60 * 24 * 28)) + 1.0)) / log(3))) + 1.0);

            /** Block Weight Reaches Maximum At Trust Key Expiration. **/
            nBlockWeight = min(20.0, (((19.0 * log(((2.0 * nBlockAge) / (TRUST_KEY_EXPIRE)) + 1.0)) / log(3))) + 1.0);
        }
        else
        {

            /* Calculate the Average Coinstake Age. */
            if(!vtx[0].GetCoinstakeAge(indexdb, nCoinAge))
                return error("CBlock::VerifyStake() : Failed to Get Coinstake Age.");

            /* Trust Weight For Genesis Transaction Reaches Maximum at 90 day Limit. */
            nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nCoinAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);
        }

        /** G] Check the nNonce Efficiency Proportion Requirements. **/
        unsigned int nThreshold = (((nTime - vtx[0].nTime) * 100.0) / nNonce) + 3;
        unsigned int nRequired  = ((50.0 - nTrustWeight - nBlockWeight) * MAX_STAKE_WEIGHT) / std::min((int64)MAX_STAKE_WEIGHT, vtx[0].vout[0].nValue);
        if(!IsInitialBlockDownload() && nThreshold < nRequired)
            error("CBlock::VerifyStake() : Coinstake / nNonce threshold too low %u Required %u. Energy efficiency limits Reached", nThreshold, nRequired);


        /** H] Check the Block Hash with Weighted Hash to Target. **/
        CBigNum bnTarget;
        bnTarget.SetCompact(nBits);

        if(GetHash() > bnTarget.getuint1024())
            return error("CBlock::VerifyStake() : Proof of Stake Hash not meeting Target.");

        if(GetArg("-verbose", 0) >= 2)
        {
            printf("CBlock::VerifyStake() : Stake Hash  %s\n", GetHash().ToString().substr(0, 20).c_str());
            printf("CBlock::VerifyStake() : Target Hash %s\n", bnTarget.getuint1024().ToString().substr(0, 20).c_str());
            printf("CBlock::VerifyStake() : Coin Age %" PRIu64 " Trust Age %" PRIu64 " Block Age %" PRIu64 "\n", nCoinAge, nTrustAge, nBlockAge);
            printf("CBlock::VerifyStake() : Trust Weight %f Block Weight %f\n", nTrustWeight, nBlockWeight);
            printf("CBlock::VerifyStake() : Threshold %u Required %u Time %u nNonce %" PRIu64 "\n", nThreshold, nRequired, (unsigned int)(nTime - vtx[0].nTime), nNonce);
        }

        return true;
    }


    /** Calculate the Age of the current Coinstake. Age is determined by average time from previous transactions. **/
    bool CTransaction::GetCoinstakeAge(LLD::CIndexDB& indexdb, uint64& nAge) const
    {
        /** Output figure to show the amount of coins being staked at their interest rates. **/
        nAge = 0;

        /** Check that the transaction is Coinstake. **/
        if(!IsCoinStake())
            return false;

        /** Check the coin age of each Input. **/
        for(int nIndex = 1; nIndex < vin.size(); nIndex++)
        {
            CTransaction txPrev;
            CTxIndex txindex;

            /** Ignore Outputs that are not in the Main Chain. **/
            if (!txPrev.ReadFromDisk(indexdb, vin[nIndex].prevout, txindex))
                return error("GetCoinstakeAge() : Invalid Previous Transaction");

            /** Read the Previous Transaction's Block from Disk. **/
            CBlock block;
            if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                return error("GetCoinstakeAge() : Failed To Read Block from Disk");

            /** Calculate the Age and Value of given output. **/
            int64 nCoinAge = (nTime - block.GetBlockTime());

            /** Compound the Total Figures. **/
            nAge += nCoinAge;
        }

        nAge /= (vin.size() - 1);

        return true;
    }


    /** Obtains the proper compounded interest from given Coin Stake Transaction. **/
    bool CTransaction::GetCoinstakeInterest(CBlock block, LLD::CIndexDB& indexdb, int64& nInterest) const
    {
        /** Check that the transaction is Coinstake. **/
        if(!IsCoinStake())
            return error("CTransaction::GetCoinstakeInterest() : Not Coinstake Transaction");

        /** Extract the Key from the Script Signature. **/
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(vout[0].scriptPubKey, keyType, vKeys))
            return error("CTransaction::GetCoinstakeInterest() : Failed To Solve Trust Key Script.");

        /** Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. **/
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTransaction::GetCoinstakeInterest() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /* Output figure to show the amount of coins being staked at their interest rates. */
        int64 nTotalCoins = 0, nAverageAge = 0;
        nInterest = 0;

        /* Calculate the Variable Interest Rate for Given Coin Age Input. [0.5% Minimum - 3% Maximum].
            Logarithmic Variable Interest Equation = 0.03 ln((9t / 31449600) + 1) / ln(10) */
        double nInterestRate = 0.05; //genesis interest rate
        if(block.nVersion >= 6)
            nInterestRate = 0.005;

        /* Get the trust key from index database. */
        if(!block.vtx[0].IsGenesis() || block.nVersion >= 6)
        {
            CTrustKey trustKey;
            if(indexdb.ReadTrustKey(cKey, trustKey))
                nInterestRate = trustKey.InterestRate(block, nTime);
        }

        /** Check the coin age of each Input. **/
        for(int nIndex = 1; nIndex < vin.size(); nIndex++)
        {
            CTransaction txPrev;
            CTxIndex txindex;

            /* Ignore Outputs that are not in the Main Chain. */
            if (!txPrev.ReadFromDisk(indexdb, vin[nIndex].prevout, txindex))
                return error("CTransaction::GetCoinstakeInterest() : Invalid Previous Transaction");

            /* Read the Previous Transaction's Block from Disk. */
            CBlock block;
            if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                return error("CTransaction::GetCoinstakeInterest() : Failed To Read Block from Disk");

            /* Calculate the Age and Value of given output. */
            int64 nCoinAge = (nTime - block.GetBlockTime());
            int64 nValue = txPrev.vout[vin[nIndex].prevout.n].nValue;

            /* Compound the Total Figures. */
            nTotalCoins += nValue;
            nAverageAge += nCoinAge;

            /* Interest is 3% of Year's Interest of Value of Coins. Coin Age is in Seconds. */
            nInterest += ((nValue * nInterestRate * nCoinAge) / (60 * 60 * 24 * 28 * 13));

        }

        nAverageAge /= (vin.size() - 1);

        return true;
    }


    bool CTrustKey::IsValid(CBlock cBlock)
    {

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        if(!cBlock.TrustKey(cKey))
            return error("CTrustKey::IsValid() : couldn't get trust key");

        /* Find the last 6 nPoS blocks. */
        CBlock pblock[6];
        const CBlockIndex* pindex[6];

        unsigned int nAverageTime = 0, nTotalGenesis = 0;
        for(int i = 0; i < 6; i++)
        {
            pindex[i] = GetLastChannelIndex(i == 0 ? mapBlockIndex[cBlock.hashPrevBlock] : pindex[i - 1]->pprev, 0);
            if(!pindex[i])
                return error("CTrustKey::IsValid() : Can't Find last Channel Index");

            if(!pblock[i].ReadFromDisk(pindex[i], true))
                return error("CTrustKey::IsValid() : Can't Read Block from Disk");

            if(i > 0)
                nAverageTime += (pblock[i - 1].nTime - pblock[i].nTime);

            if(pblock[i].vtx[0].IsGenesis())
                nTotalGenesis++;
        }
        nAverageTime /= 5;

        /* Create an LLD instance for Tx Lookups. */
        LLD::CIndexDB indexdb("cr");

        /* Handle Genesis Transaction Rules. Genesis is checked after Trust Key Established. */
        if(cBlock.vtx[0].IsGenesis())
        {
            /* RULE: Average Block time of last six blocks not to go below 30 seconds */
            if(nTotalGenesis > 3 && nAverageTime < 30)
                return error("\x1b[31m SOFTBAN: \u001b[37;1m 5 Genesis Average block time < 20 seconds %u \x1b[0m", nAverageTime );

            /* Check the coin age of each Input. */
            for(int nIndex = 1; nIndex < cBlock.vtx[0].vin.size(); nIndex++)
            {
                CTransaction txPrev;
                CTxIndex txindex;

                /* Ignore Outputs that are not in the Main Chain. */
                if (!txPrev.ReadFromDisk(indexdb, cBlock.vtx[0].vin[nIndex].prevout, txindex))
                    return error("CTrustKey::IsValid() : Invalid Previous Transaction");

                /* Read the Previous Transaction's Block from Disk. */
                CBlock block;
                if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                    return error("CTrustKey::IsValid() : Failed To Read Block from Disk");

                /* Check that Genesis has no Transactions. */
                if(cBlock.vtx.size() != 1)
                    return error("CTrustKey::IsValid() : Cannot Include Transactions with Genesis Transaction");

                /* RULE: No Genesis if coin age is less than 1 days old. */
                if((cBlock.nTime - block.GetBlockTime()) < 24 * 60 * 60)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m Genesis Input less than 24 hours Age \x1b[0m");
            }

            /* Genesis Rules: Less than 1000 NXS in block. */
            if(cBlock.vtx[0].GetValueOut() < 1000 * COIN)
            {
                /* RULE: More than 2 conesuctive Genesis with < 1000 NXS */
                if (pblock[0].vtx[0].GetValueOut() < 1000 * COIN &&
                    pblock[1].vtx[0].GetValueOut() < 1000 * COIN)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m More than 2 Consecutive blocks < 1000 NXS \x1b[0m");
            }
        }

        /** Handle Adding Trust Transactions. **/
        else if(cBlock.vtx[0].IsTrust())
        {

            /* RULE: Average Block time of last six blocks not to go below 30 seconds */
            if(nAverageTime < 20 && (cBlock.nTime - pblock[0].nTime) < 30)
                return error("\x1b[31m SOFTBAN: \u001b[37;1m Trust Average block time < 30 seconds %u \x1b[0m", nAverageTime);


            /* Check the coin age of each Input. */
            for(int nIndex = 1; nIndex < cBlock.vtx[0].vin.size(); nIndex++)
            {
                CTransaction txPrev;
                CTxIndex txindex;

                /* Ignore Outputs that are not in the Main Chain. */
                if (!txPrev.ReadFromDisk(indexdb, cBlock.vtx[0].vin[nIndex].prevout, txindex))
                    return error("CTrustKey() : Invalid Previous Transaction");

                /* Read the Previous Transaction's Block from Disk. */
                CBlock block;
                if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                    return error("CTrustKey() : Failed To Read Block from Disk");

                /* RULE: Inputs need to have at least 100 confirmations */
                if(cBlock.nHeight - block.nHeight < 100)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m Trust Input less than 100 confirmations \x1b[0m");
            }

            /* Genesis Rules: Less than 1000 NXS in block. */
            if(cBlock.vtx[0].GetValueOut() < 1000 * COIN)
            {
                /* RULE: More than 2 conesuctive Genesis with < 1000 NXS */
                if (pblock[0].vtx[0].GetValueOut() < 1000 * COIN &&
                    pblock[1].vtx[0].GetValueOut() < 1000 * COIN)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m More than 2 Consecutive Trust < 1000 NXS \x1b[0m");
            }
        }

        return true;
    }


    /** Interest is Determined By Logarithmic Equation from Genesis Key. **/
    double CTrustKey::InterestRate(CBlock block, unsigned int nTime) const
    {
        /* Genesis interest rate is 0.5% */
        if(block.vtx[0].IsGenesis())
            return 0.005;

        /* Block version 4 is the age of key from timestamp. */
        unsigned int nTrustScore;
        if(block.nVersion == 4)
            nTrustScore = (nTime - nGenesisTime);

        /* Block version 5 is the trust score of the key. */
        else if(!block.TrustScore(nTrustScore))
            return 0.0; //this will trigger an interest rate failure

        return min(0.03, ((((0.025 * log(((9.0 * (nTrustScore)) / (60 * 60 * 24 * 28 * 13)) + 1.0)) / log(10))) + 0.005));
    }

    /** Break the Chain Age in Minutes into Days, Hours, and Minutes. **/
    void GetTimes(unsigned int nAge, unsigned int& nDays, unsigned int& nHours, unsigned int& nMinutes)
    {
        nDays = nAge / 1440;
        nHours = (nAge - (nDays * 1440)) / 60;
        nMinutes = nAge % 60;
    }


    /** Should not be called until key is established in block chain.
        Block must have been received and be part of the main chain. **/
    bool CTrustKey::CheckGenesis(CBlock cBlock) const
    {
        /* Invalid if Null. */
        if(IsNull())
            return false;

        /* Trust Keys must be created from only Proof of Stake Blocks. */
        if(!cBlock.IsProofOfStake())
            return error("CTrustKey::CheckGenesis() : genesis has to be proof of stake");

        /* Trust Key Timestamp must be the same as Genesis Key Block Timestamp. */
        if(nGenesisTime != cBlock.nTime)
            return error("CTrustKey::CheckGenesis() : genesis time mismatch");

        /* Genesis Key Transaction must match Trust Key Genesis Hash. */
        if(cBlock.vtx[0].GetHash() != hashGenesisTx)
            return error("CTrustKey::CheckGenesis() : genesis coinstake hash mismatch");

        /* Check the genesis block hash. */
        if(cBlock.GetHash() != hashGenesisBlock)
            return error("CTrustKey::CheckGenesis() : genesis hash mismatch");

        return true;
    }


    /* Key is Expired if Time between Network Previous Best Block and
     Trust Best Previous is Greater than Expiration Time. */
    bool CTrustKey::Expired(uint1024 hashThisBlock, unsigned int nTimespan) const
    {
        if(BlockAge(hashThisBlock) > nTimespan)
            return true;

        return false;
    }


    /** Key is Expired if it is Invalid or Time between Network Best Block and Best Previous is Greater than Expiration Time. **/
    uint64 CTrustKey::Age(unsigned int nTime) const
    {
        return (uint64)(nTime - nGenesisTime);
    }


    /* The Age of a Key in Block age as in the Time it has been since Trust Key has produced block. */
    uint64 CTrustKey::BlockAge(uint1024 hashBestBlock) const
    {
        /* Check the index for the best block. */
        if(!mapBlockIndex.count(hashBestBlock))
            return error("CTrustKey::BlockAge() : best block not found %s", hashBestBlock.ToString().substr(0, 20).c_str());

        /* Check the index for the last block. */
        uint1024 hashLastBlock = hashBestBlock;
        if(!LastTrustBlock(*this, hashLastBlock))
            return error("CTrustKey::BlockAge() : last trust block not found %s", hashLastBlock.ToString().substr(0, 20).c_str());

        /* Block Age is Time to Previous Block's Time. */
        return (uint64)(mapBlockIndex[hashBestBlock]->pprev->GetBlockTime() - mapBlockIndex[hashLastBlock]->GetBlockTime());
    }



    /** Proof of Stake local CPU miner. Uses minimal resources. **/
    void StakeMinter(void* parg)
    {
        printf("Stake Minter Started\n");
        SetThreadPriority(THREAD_PRIORITY_LOWEST);

        /* Thread waiting while downloading or wallet locked. */
        while(!fShutdown)
        {
            /* Sleep call to keep the thread from running. */
            Sleep(1000);

            /* Don't stake if the wallet is locked. */
            if (pwalletMain->IsLocked())
                continue;

            /* Don't stake if there are no available nodes. */
            if (!Net::vNodes.empty() && !IsInitialBlockDownload())
                break;
        }

        // Each thread has its own key and counter
        Wallet::CReserveKey reservekey(pwalletMain);

        /* Lower Level Database Instance. */
        LLD::CTrustDB trustdb("r+");

        /* The trust key in a byte vector. */
        std::vector<unsigned char> vchTrustKey;

        /* Trust Key is written from version 5 rules. */
        CTrustKey trustKey;
        trustKey.SetNull();

        /* See if key is cached in database. */
        if(trustdb.ReadMyKey(trustKey))
        {

            /* Check if my key has a version 4 previous. */
            uint1024 hashLastBlock = Core::hashBestChain;
            if(LastTrustBlock(trustKey, hashLastBlock) && mapBlockIndex[hashLastBlock]->nVersion == 4 &&
                (pindexBest->pprev->GetBlockTime() - mapBlockIndex[hashLastBlock]->GetBlockTime()) > (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
            {
                /* Debug notification. */
                error("Stake Minter : Version 4 key never made it through grace period.\n");

                /* Erase expired trust key. */
                LLD::CIndexDB indexdb("r+");
                indexdb.EraseTrustKey(trustKey.GetKey());

                /* Erase my key if expired. */
                trustdb.EraseMyKey();

                /* Set trust key to null state. */
                trustKey.SetNull();
            }
            else
                vchTrustKey = trustKey.vchPubKey;

        }

        /* Search for trust key if it is not cached. */
        if(trustKey.IsNull())
        {
            LLD::CIndexDB indexdb("r+");
            std::vector<uint576> vKeys;
            if(indexdb.GetTrustKeys(vKeys))
            {
                for(auto key : vKeys)
                {
                    CTrustKey trustCheck;
                    if(!indexdb.ReadTrustKey(key, trustCheck))
                        continue;

                    Wallet::NexusAddress address;
                    address.SetPubKey(trustCheck.vchPubKey);
                    if(pwalletMain->HaveKey(address))
                    {
                        /* Check for keys that are expired version 4 previous. */
                        uint1024 hashLastBlock = Core::hashBestChain;
                        if(LastTrustBlock(trustCheck, hashLastBlock) && mapBlockIndex[hashLastBlock]->nVersion == 4 &&
                            (pindexBest->pprev->GetBlockTime() - mapBlockIndex[hashLastBlock]->GetBlockTime()) > (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
                        {
                            /* Debug notification. */
                            error("Stake Minter : Version 4 key never made it through grace period.\n");

                            /* Erase expired trust key. */
                            indexdb.EraseTrustKey(key);

                            continue;
                        }

                        /* Set the binary data of trust key. */
                        vchTrustKey = trustCheck.vchPubKey;

                        /* Set the trust key if found. */
                        trustKey = trustCheck;

                        /* Write my key to disk. */
                        trustdb.WriteMyKey(trustKey);

                        /* Debug output for key finder. */
                        printf("Stake Minter : Found my Trust Key %s\n", HexStr(key.begin(), key.end()).c_str());
                    }
                }
            }

            if(vchTrustKey.empty())
                vchTrustKey = reservekey.GetReservedKey();
        }

        while(!fShutdown)
        {
            Sleep(1000);

            /* Take a snapshot of the best block. */
            uint1024 hashBest = hashBestChain;

            /* Create the block to work on. */
            CBlock block = CreateNewBlock(reservekey, pwalletMain, 0);
            if(block.IsNull())
                continue;

            /* Write the trust key into the output script. */
            block.vtx[0].vout[0].scriptPubKey << vchTrustKey << Wallet::OP_CHECKSIG;

            /* Trust transaction. */
            if(!trustKey.IsNull())
            {
                /* Set the key from bytes. */
                uint576 key;
                key.SetBytes(trustKey.vchPubKey);

                /* Check that the database has key. */
                LLD::CIndexDB indexdb("r+");
                CTrustKey keyCheck;
                if(!indexdb.ReadTrustKey(key, keyCheck))
                {
                    error("Stake Minter : trust key was disconnected");

                    /* Erase my key from trustdb. */
                    trustdb.EraseMyKey();

                    /* Set the trust key to null state. */
                    trustKey.SetNull();

                    continue;
                }

                /* Previous out needs to be 0 in coinstake transaction. */
                block.vtx[0].vin[0].prevout.n = 0;

                /* Previous out hash is trust key hash */
                block.vtx[0].vin[0].prevout.hash = trustKey.GetHash();

                /* Get the last block of this trust key. */
                uint1024 hashLastBlock = hashBest;
                if(!LastTrustBlock(trustKey, hashLastBlock))
                {
                    error("Stake Minter : failed to find last block for trust key");
                    continue;
                }

                /* Get the last block index from map block index. */
                CBlock blockPrev;
                if(!blockPrev.ReadFromDisk(mapBlockIndex[hashLastBlock], true))
                {
                    error("Stake Minter : failed to read last block for trust key");
                    continue;
                }

                /* Enforce the minimum trust key interval of 120 blocks. */
                if(block.nHeight - blockPrev.nHeight < (fTestNet ? TESTNET_MINIMUM_INTERVAL : MAINNET_MINIMUM_INTERVAL))
                {
                    //error("Stake Minter : trust key interval below minimum interval %u", block.nHeight - blockPrev.nHeight);
                    continue;
                }

                /* Get the sequence and previous trust. */
                unsigned int nTrustPrev = 0, nSequence = 0, nScore = 0;

                /* Handle if previous block was a genesis. */
                if(blockPrev.vtx[0].IsGenesis())
                {
                    nSequence   = 1;
                    nTrustPrev  = 0;
                }

                /* Handle if previous block was version 4 */
                else if(blockPrev.nVersion < 5)
                {
                    nSequence   = 1;
                    nTrustPrev  = trustKey.Age(mapBlockIndex[block.hashPrevBlock]->GetBlockTime());
                }

                /* Handle if previous block is version 5 trust block. */
                else
                {
                    /* Extract the trust from the previous block. */
                    uint1024 hashDummy;
                    if(!blockPrev.ExtractTrust(hashDummy, nSequence, nTrustPrev))
                    {
                        error("Stake Minter : failed to extract trust from previous block");
                        continue;
                    }

                    /* Increment Sequence Number. */
                    nSequence ++;

                }

                /* The time it has been since the last trust block for this trust key. */
                int nTimespan = (mapBlockIndex[block.hashPrevBlock]->GetBlockTime() - blockPrev.nTime);

                /* Timespan less than required timespan is awarded the total seconds it took to find. */
                if(nTimespan < (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
                    nScore = nTrustPrev + nTimespan;

                /* Timespan more than required timespan is penalized 3 times the time it took past the required timespan. */
                else
                {
                    /* Calculate the penalty for score (3x the time). */
                    int nPenalty = (nTimespan - (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN)) * 3;

                    /* Catch overflows and zero out if penalties are greater than previous score. */
                    if(nPenalty < nTrustPrev)
                        nScore = (nTrustPrev - nPenalty);
                    else
                        nScore = 0;
                }

                /* Set maximum trust score to seconds passed for interest rate. */
                if(nScore > (60 * 60 * 24 * 28 * 13))
                    nScore = (60 * 60 * 24 * 28 * 13);

                /* Serialize the sequence and last block into vin. */
                CDataStream scriptPub(block.vtx[0].vin[0].scriptSig, SER_NETWORK, PROTOCOL_VERSION);
                scriptPub << hashLastBlock << nSequence << nScore;

                /* Set the script sig (CScript doesn't support serializing all types needed) */
                block.vtx[0].vin[0].scriptSig.clear();
                block.vtx[0].vin[0].scriptSig.insert(block.vtx[0].vin[0].scriptSig.end(), scriptPub.begin(), scriptPub.end());
            }
            else
                block.vtx[0].vin[0].prevout.SetNull();

            /* Add the coinstake inputs */
            if (!pwalletMain->AddCoinstakeInputs(block))
            {
                error("Stake Minter : no spendable inputs available");
                continue;
            }

            /* Weight for Trust transactions combine block weight and stake weight. */
            double nTrustWeight = 0.0, nBlockWeight = 0.0;
            if(block.vtx[0].IsTrust())
            {
                /* Get the score and make sure it all checks out. */
                unsigned int nTrustAge;
                if(!block.TrustScore(nTrustAge))
                {
                    error("Stake Minter : failed to get trust score");
                    continue;
                }

                /* Get the weights with the block age. */
                unsigned int nBlockAge;
                if(!block.BlockAge(nBlockAge))
                {
                    error("Stake Minter : failed to get block age");
                    continue;
                }

                /* Trust Weight Continues to grow the longer you have staked and higher your interest rate */
                nTrustWeight = min(90.0, (((44.0 * log(((2.0 * nTrustAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);

                /* Block Weight Reaches Maximum At Trust Key Expiration. */
                nBlockWeight = min(10.0, (((9.0 * log(((2.0 * nBlockAge) / ((fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))) + 1.0)) / log(3))) + 1.0);

                /* Set the Reporting Variables for the Qt. */
                dTrustWeight = nTrustWeight;
                dBlockWeight = nBlockWeight;
            }

            /* Weight for Gensis transactions are based on your coin age. */
            else
            {

                /* Calculate the Average Coinstake Age. */
                LLD::CIndexDB indexdb("cr");
                uint64 nCoinAge;
                if(!block.vtx[0].GetCoinstakeAge(indexdb, nCoinAge))
                {
                    error("Stake Minter : failed to get coinstake age");
                    continue;
                }

                /* Genesis has to wait for one full trust key timespan. */
                if(nCoinAge < (fTestNet ? TRUST_KEY_TIMESPAN_TESTNET : TRUST_KEY_TIMESPAN))
                {
                    fIsWaitPeriod = true;
                    error("Stake Minter : genesis age is immature");
                    continue;
                }

                /* Trust Weight For Genesis Transaction Reaches Maximum at 90 day Limit. */
                nTrustWeight = min(10.0, (((9.0 * log(((2.0 * nCoinAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);

                /* Set the Reporting Variables for the Qt. */
                dTrustWeight = nTrustWeight;
                dBlockWeight = 0;
            }

            /* Calculate the energy efficiency requirements. */
            double nRequired  = ((108.0 - nTrustWeight - nBlockWeight) * MAX_STAKE_WEIGHT) / block.vtx[0].vout[0].nValue;

            /* Calculate the target value based on difficulty. */
            CBigNum bnTarget;
            bnTarget.SetCompact(block.nBits);
            uint1024 hashTarget = bnTarget.getuint1024();

            /* Set the interest rate variable. */
            dInterestRate = trustKey.InterestRate(block, mapBlockIndex[block.hashPrevBlock]->GetBlockTime());

            /* Sign the new Proof of Stake Block. */
            if(GetArg("-verbose", 0) >= 0)
                printf("Stake Minter : staking from block %s at weight %f and rate %f\n", hashBest.ToString().substr(0, 20).c_str(), (dTrustWeight + dBlockWeight), dInterestRate);

            /* Search for the proof of stake hash. */
            while(hashBest == hashBestChain)
            {
                /* Update the block time for difficulty accuracy. */
                block.UpdateTime();
                if(block.nTime == block.vtx[0].nTime)
                    continue;

                /* Calculate the Efficiency Threshold. */
                double nThreshold = (double)((block.nTime - block.vtx[0].nTime) * 100.0) / (block.nNonce + 1);

                /* Allow the Searching For Stake block if Below the Efficiency Threshold. */
                if(nThreshold < nRequired)
                {
                    Sleep(1);
                    continue;
                }

                /* Increment the nOnce. */
                block.nNonce ++;

                /* Debug output. */
                if(block.nNonce % 1000 == 0 && GetArg("-verbose", 0) >= 3)
                    printf("Stake Minter : below threshold %f required %f incrementing nonce %" PRIu64 "\n", nThreshold, nRequired, block.nNonce);

                /* Handle if block is found. */
                if (block.StakeHash() < hashTarget)
                {
                    /* Sign the new Proof of Stake Block. */
                    if(GetArg("-verbose", 0) >= 0)
                        printf("Stake Minter : found new stake hash %s\n", block.StakeHash().ToString().substr(0, 20).c_str());

                    /* Set the staking thread priorities. */
                    SetThreadPriority(THREAD_PRIORITY_NORMAL);

                    /* Add the transactions into the block from memory pool. */
                    if (!block.vtx[0].IsGenesis())
                        AddTransactions(block.vtx, pindexBest);

                    /* Build the Merkle Root. */
                    block.hashMerkleRoot   = block.BuildMerkleTree();

                    /* Sign the block. */
                    if (!block.SignBlock(*pwalletMain))
                    {
                        printf("Stake Minter : failed to sign block");
                        break;
                    }

                    /* Check the block. */
                    if (!block.CheckBlock())
                    {
                        error("Stake Minter : check block failed");
                        break;
                    }

                    /* Check the stake. */
                    if (!block.CheckStake())
                    {
                        error("Stake Minter : check stake failed");
                        break;
                    }

                    /* Check the stake. */
                    if (!block.CheckTrust())
                    {
                        error("Stake Minter : check stake failed");
                        break;
                    }

                    /* Check the work for the block. */
                    if(!CheckWork(&block, *pwalletMain, reservekey))
                    {
                        error("Stake Minter : check work failed");
                        break;
                    }

                    /* Write the trust key to the key db. */
                    if(trustKey.IsNull())
                    {
                        CTrustKey trustKeyNew(vchTrustKey, block.GetHash(), block.vtx[0].GetHash(), block.nTime);
                        trustdb.WriteMyKey(trustKeyNew);

                        trustKey = trustKeyNew;
                        printf("Stake Minter : new trust key written\n");
                    }

                    break;
                }
            }

            SetThreadPriority(THREAD_PRIORITY_LOWEST);
        }
    }
}
