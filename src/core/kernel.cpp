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

using namespace std;

/** Locate the Add Coinstake Inputs Method Here for access. **/
namespace Wallet
{

    bool CWallet::AddCoinstakeInputs(Core::CTransaction& txNew)
    {

        /* Add Each Input to Transaction. */
        vector<const CWalletTx*> vInputs;
        vector<const CWalletTx*> vCoins;

        txNew.vout[0].nValue = 0;

        {
        LOCK(cs_wallet);

        vCoins.reserve(mapWallet.size());
        for (map<uint512, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
            vCoins.push_back(&(*it).second);
        }

        random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);
        BOOST_FOREACH(const CWalletTx* pcoin, vCoins)
        {
            if (!pcoin->IsFinal() || pcoin->GetDepthInMainChain() < 60)
                continue;

            if ((pcoin->IsCoinBase() || pcoin->IsCoinStake()) && pcoin->GetBlocksToMaturity() > 0)
                continue;

            /* Do not add coins to Genesis block if age < 24hrs */
            if (txNew.IsGenesis() && (txNew.nTime - pcoin->nTime) < 24 * 60 * 60)
            continue;

            for (unsigned int i = 0; i < pcoin->vout.size(); i++)
            {
                if (pcoin->IsSpent(i) || !IsMine(pcoin->vout[i]))
                    continue;

                if (pcoin->nTime > txNew.nTime)
                    continue;

                //if(txNew.vout[0].nValue > (nBalance - nReserveBalance))
                //    break;

                /* Stop adding Inputs if has reached Maximum Transaction Size. */
                unsigned int nBytes = ::GetSerializeSize(txNew, SER_NETWORK, PROTOCOL_VERSION);
                if (nBytes >= Core::MAX_BLOCK_SIZE_GEN / 5)
                    break;

                txNew.vin.push_back(Core::CTxIn(pcoin->GetHash(), i));
                vInputs.push_back(pcoin);

                /** Add the value to the first Output for Coinstake. **/
                txNew.vout[0].nValue += pcoin->vout[i].nValue;
            }
        }

        if(txNew.vin.size() == 1)
            return false;

        /** Set the Interest for the Coinstake Transaction. **/
        int64 nInterest;
        LLD::CIndexDB indexdb("rw");
        if(!txNew.GetCoinstakeInterest(indexdb, nInterest))
            return error("AddCoinstakeInputs() : Failed to Get Interest");

        txNew.vout[0].nValue += nInterest;

        /** Sign Each Input to Transaction. **/
        for(int nIndex = 0; nIndex < vInputs.size(); nIndex++)
        {
            if (!SignSignature(*this, *vInputs[nIndex], txNew, nIndex + 1))
                return error("AddCoinstakeInputs() : Unable to sign Coinstake Transaction Input.");

        }

        return true;
    }

}

namespace Core
{

    /** Check the Coinstake Transaction is within the rules applied for Proof of Stake. **/
    bool CBlock::VerifyStake() const
    {

        /* Check Average age is above Limit if No Trust Key Seen. */
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CBlock::VerifyStake() : Failed To Solve Trust Key Script.");

        /* Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. */
        if (keyType != Wallet::TX_PUBKEY)
            return error("CBlock::VerifyStake() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /* Determine Trust Age if the Trust Key Exists. */
        uint64 nCoinAge = 0, nTrustAge = 0, nBlockAge = 0;
        double nTrustWeight = 0.0, nBlockWeight = 0.0;
        if(!cTrustPool.Exists(cKey))
            return error("CBlock::VerifyStake() : No Trust Key in Trust Pool (Missing Accept)");

        if(vtx[0].IsTrust())
        {

            /* Check the genesis and trust timestamps. */
            if(cTrustPool.Find(cKey).nGenesisTime > mapBlockIndex[hashPrevBlock]->GetBlockTime())
                return error("CBlock::VerifyStake() : Genesis Time cannot be after Trust Time.");

            nTrustAge = cTrustPool.Find(cKey).Age(mapBlockIndex[hashPrevBlock]->GetBlockTime());
            nBlockAge = cTrustPool.Find(cKey).BlockAge(GetHash(), hashPrevBlock);

            /** Trust Weight Reaches Maximum at 30 day Limit. **/
            nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nTrustAge) / (60 * 60 * 24 * 28)) + 1.0)) / log(3))) + 1.0);

            /** Block Weight Reaches Maximum At Trust Key Expiration. **/
            nBlockWeight = min(20.0, (((19.0 * log(((2.0 * nBlockAge) / (TRUST_KEY_EXPIRE)) + 1.0)) / log(3))) + 1.0);
        }
        else
        {

            /** Calculate the Average Coinstake Age. **/
            LLD::CIndexDB indexdb("r");
            if(!vtx[0].GetCoinstakeAge(indexdb, nCoinAge))
                return error("CBlock::VerifyStake() : Failed to Get Coinstake Age.");

            /** Trust Weight For Genesis Transaction Reaches Maximum at 90 day Limit. **/
            nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nCoinAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);
        }

        /** G] Check the nNonce Efficiency Proportion Requirements. **/
        double nThreshold = ((nTime - vtx[0].nTime) * 100.0) / nNonce;
        double nRequired  = ((50.0 - nTrustWeight - nBlockWeight) * MAX_STAKE_WEIGHT) / std::min((int64)MAX_STAKE_WEIGHT, vtx[0].vout[0].nValue);
        if(nThreshold < nRequired)
            return error("CBlock::VerifyStake() : Coinstake / nNonce threshold too low %f Required %f. Energy efficiency limits Reached", nThreshold, nRequired);


        /** H] Check the Block Hash with Weighted Hash to Target. **/
        CBigNum bnTarget;
        bnTarget.SetCompact(nBits);
        uint1024 hashTarget = bnTarget.getuint1024();

        if(GetHash() > hashTarget)
            return error("CBlock::VerifyStake() : Proof of Stake Hash not meeting Target.");

        if(GetArg("-verbose", 0) >= 2)
        {
            printf("CBlock::VerifyStake() : Stake Hash  %s\n", GetHash().ToString().substr(0, 20).c_str());
            printf("CBlock::VerifyStake() : Target Hash %s\n", hashTarget.ToString().substr(0, 20).c_str());
            printf("CBlock::VerifyStake() : Coin Age %" PRIu64 " Trust Age %" PRIu64 " Block Age %" PRIu64 "\n", nCoinAge, nTrustAge, nBlockAge);
            printf("CBlock::VerifyStake() : Trust Weight %f Block Weight %f\n", nTrustWeight, nBlockWeight);
            printf("CBlock::VerifyStake() : Threshold %f Required %f Time %u nNonce %" PRIu64 "\n", nThreshold, nRequired, (unsigned int)(nTime - vtx[0].nTime), nNonce);
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
    bool CTransaction::GetCoinstakeInterest(LLD::CIndexDB& indexdb, int64& nInterest) const
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

        /** Set the Public Key Integer Key from Bytes. **/
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /** Output figure to show the amount of coins being staked at their interest rates. **/
        int64 nTotalCoins = 0, nAverageAge = 0;
        nInterest = 0;

        /** Calculate the Variable Interest Rate for Given Coin Age Input. [0.5% Minimum - 3% Maximum].
            Logarithmic Variable Interest Equation = 0.03 ln((9t / 31449600) + 1) / ln(10) **/
        double nInterestRate = cTrustPool.InterestRate(cKey, nTime);

        /** Check the coin age of each Input. **/
        for(int nIndex = 1; nIndex < vin.size(); nIndex++)
        {
            CTransaction txPrev;
            CTxIndex txindex;

            /** Ignore Outputs that are not in the Main Chain. **/
            if (!txPrev.ReadFromDisk(indexdb, vin[nIndex].prevout, txindex))
                return error("CTransaction::GetCoinstakeInterest() : Invalid Previous Transaction");

            /** Read the Previous Transaction's Block from Disk. **/
            CBlock block;
            if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                return error("CTransaction::GetCoinstakeInterest() : Failed To Read Block from Disk");

            /** Calculate the Age and Value of given output. **/
            int64 nCoinAge = (nTime - block.GetBlockTime());
            int64 nValue = txPrev.vout[vin[nIndex].prevout.n].nValue;

            /** Compound the Total Figures. **/
            nTotalCoins += nValue;
            nAverageAge += nCoinAge;

            /** Interest is 2% of Year's Interest of Value of Coins. Coin Age is in Seconds. **/
            nInterest += ((nValue * nInterestRate * nCoinAge) / (60 * 60 * 24 * 28 * 13));

        }

        nAverageAge /= (vin.size() - 1);

        return true;
    }


    /** TODO: Trust Key Reactivation
    *
    * Function to Check the Current Trust Key to see if it is expired.
    * If Key is Empty, check Trust Pool for Possible Key owned by this wallet. **/
    bool CTrustPool::HasTrustKey(unsigned int nTime)
    {
        /** First Check if the Current Key is Expired. **/
        if(!vchTrustKey.empty())
        {
            uint576 cKey;
            cKey.SetBytes(vchTrustKey);

            /* Handle if the Trust Pool does not have current Assigned Trust Key. */
            if(!Exists(cKey))
            {
                vchTrustKey.clear();

                return error("CTrustPool::HasTrustKey() : Current Trust Key not in Pool.");
            }

            /* Handle Expired Trust Key already declared. */
            if(mapTrustKeys[cKey].Expired(0, pindexBest->GetBlockHash()))
            {
                vchTrustKey.clear();

                return error("CTrustPool::HasTrustKey() : Current Trust Key is Expired.");
            }

            /* Set the Interest Rate for the GUI. */
            dInterestRate = cTrustPool.InterestRate(cKey, nTime);

            return true;
        }
        else
            dInterestRate = 0.005;

        /** Check each Trust Key to See if we Own it if there is no Key. **/
        CTrustKey keyBestTrust;
        for(std::map<uint576, CTrustKey>::iterator i = mapTrustKeys.begin(); i != mapTrustKeys.end() && vchTrustKey.empty(); ++i)
        {

            /* Check the Wallet and Trust Keys in Trust Pool to see if we own any keys. */
            Wallet::NexusAddress address;
            address.SetPubKey(i->second.vchPubKey);
            if(pwalletMain->HaveKey(address))
            {
                if(i->second.Expired(0, pindexBest->GetBlockHash()))
                    continue;

                if(i->second.Age(nTime) > keyBestTrust.Age(nTime) || keyBestTrust.IsNull())
                {
                    keyBestTrust = i->second;
<<<<<<< HEAD

                    if(GetArg("-verbose", 0) >= 0)
                        printf("CTrustPool::HasTrustKey() : Trying Trust Key %s\n", HexStr(keyBestTrust.vchPubKey.begin(), keyBestTrust.vchPubKey.end()).c_str());
=======

                    if(GetArg("-verbose", 0) >= 0)
                        printf("CTrustPool::HasTrustKey() : Checking Trust Key %s\n", HexStr(keyBestTrust.vchPubKey.begin(), keyBestTrust.vchPubKey.end()).c_str());
>>>>>>> 2.4.4-validation
                }
            }
        }

        /* If a Trust key was Found. */
        if(!keyBestTrust.IsNull())
        {
            /* Assigned Extracted Key to Trust Pool. */
            if(GetArg("-verbose", 0) >= 0) {
                printf("CTrustPool::HasTrustKey() : Selected Trust Key %s\n", HexStr(keyBestTrust.vchPubKey.begin(), keyBestTrust.vchPubKey.end()).c_str());

                keyBestTrust.Print();
            }

            /* Set the Interest Rate from Key. */
            vchTrustKey = keyBestTrust.vchPubKey;
            dInterestRate = cTrustPool.InterestRate(keyBestTrust.GetKey(), nTime);

            return true;
        }

        return false;
    }

    bool CTrustPool::IsValid(CBlock cBlock)
    {
        /* Lock Accepting Trust Keys to Mutex. */
        LOCK(cs);

        /* Extract the Key from the Script Signature. */
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustPool::IsValid() : Failed To Solve Trust Key Script.");

        /* Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. */
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustPool::IsValid() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Verify the Stake Efficiency Threshold. */
        if(!cBlock.VerifyStake())
            return error("CTrustPool::IsValid() : Invalid Proof of Stake");

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /* Find the last 6 nPoS blocks. */
        CBlock pblock[6];
        const CBlockIndex* pindex[6];

        unsigned int nAverageTime = 0, nTotalGenesis = 0;
        for(int i = 0; i < 6; i++)
        {
            pindex[i] = GetLastChannelIndex(i == 0 ? mapBlockIndex[cBlock.hashPrevBlock] : pindex[i - 1]->pprev, 0);
            if(!pindex[i])
                return error("CTrustPool::IsValid() : Can't Find last Channel Index");

            if(!pblock[i].ReadFromDisk(pindex[i]->nFile, pindex[i]->nBlockPos, true))
                return error("CTrustPool::IsValid() : Can't Read Block from Disk");

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
                    return error("CTrustPool::IsValid() : Invalid Previous Transaction");

                /* Read the Previous Transaction's Block from Disk. */
                CBlock block;
                if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                    return error("CTrustPool::IsValid() : Failed To Read Block from Disk");

                /* Check that Transaction is not Genesis when Trust Key is Established. */
                if(cBlock.GetHash() != cTrustPool.Find(cKey).hashGenesisBlock)
                    return error("CTrustPool::IsValid() : Duplicate Genesis not Allowed");

                /* Check that Genesis has no Transactions. */
                if(cBlock.vtx.size() != 1)
                    return error("CTrustPool::IsValid() : Cannot Include Transactions with Genesis Transaction");

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
                    return error("GetCoinstakeAge() : Invalid Previous Transaction");

                /* Read the Previous Transaction's Block from Disk. */
                CBlock block;
                if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                    return error("GetCoinstakeAge() : Failed To Read Block from Disk");

                /* RULE: Inputs need to have at least 100 confirmations */
                if(cBlock.nHeight - block.nHeight < 100)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m Trust Input less than 100 confirmations \x1b[0m");
            }

            /* Get the time since last block. */
            uint64 nTrustAge = mapTrustKeys[cKey].Age(GetUnifiedTimestamp());
            uint64 nBlockAge = mapTrustKeys[cKey].BlockAge(cBlock.GetHash(), cBlock.hashPrevBlock);

            /* Genesis Rules: Less than 1000 NXS in block. */
            if(cBlock.vtx[0].GetValueOut() < 1000 * COIN)
            {
                /* RULE: More than 2 conesuctive Genesis with < 1000 NXS */
                if (pblock[0].vtx[0].GetValueOut() < 1000 * COIN &&
                    pblock[1].vtx[0].GetValueOut() < 1000 * COIN)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m More than 2 Consecutive Trust < 1000 NXS \x1b[0m");

                /* RULE: Trust with < 1000 made within 8 hours of last */
                if(nBlockAge < 8 * 60 * 60)
                    return error("\x1b[31m SOFTBAN: \u001b[37;1m Less than 8 hours since last Trust made with < 1000 NXS \x1b[0m");
            }
        }

        return true;
    }



    /** Check a Block's Coinstake Transaction to see if it fits Trust Key Protocol.

        If Key doesn't exist Transaction must meet Genesis Protocol Requirements.
        If Key does exist it Must Meet Trust Protocol Requirements.

    **/
    bool CTrustPool::Check(CBlock cBlock)
    {
        /* Lock Accepting Trust Keys to Mutex. */
        LOCK(cs);

        /* Ensure the Block is for Proof of Stake Only. */
        if(!cBlock.IsProofOfStake())
            return error("CTrustPool::check() : Cannot Accept non Coinstake Transactions.");

        /* Check the Coinstake Time is before Unified Timestamp. */
        if(cBlock.vtx[0].nTime > (GetUnifiedTimestamp() + MAX_UNIFIED_DRIFT))
            return error("CTrustPool::check() : Coinstake Transaction too far in Future.");

        /* Make Sure Coinstake Transaction is First. */
        if (!cBlock.vtx[0].IsCoinStake())
            return error("CTrustPool::check() : First transaction non-coinstake %s", cBlock.vtx[0].GetHash().ToString().c_str());

        /* Make Sure Coinstake Transaction Time is Before Block. */
        if (cBlock.vtx[0].nTime > cBlock.nTime)
            return error("CTrustPool::check()  : Coinstake Timestamp to far into Future.");

        /* Check that the Coinbase / CoinstakeTimstamp is after Previous Block. */
        if (mapBlockIndex[cBlock.hashPrevBlock]->GetBlockTime() > cBlock.vtx[0].nTime)
            return error("CTrustPool::check()  : Coinstake Timestamp too Early.");

        /* Extract the Key from the Script Signature. */
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustPool::check() : Failed To Solve Trust Key Script.");

        /** Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. **/
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustPool::check() : Trust Key must be of Public Key Type Created from Keypool.");

        return true;
    }


    bool CTrustPool::Connect(CBlock cBlock, bool fInit)
    {
        /* Lock Accepting Trust Keys to Mutex. */
        LOCK(cs);

        /* Extract the Key from the Script Signature. */
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustPool::Connect() : Failed To Solve Trust Key Script.");

        /* Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. */
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustPool::Connect() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /* Handle Genesis Transaction Rules. Genesis is checked after Trust Key Established. */
        if(cBlock.vtx[0].IsGenesis())
        {

            std::vector< std::pair<uint1024, bool> >::iterator itFalse = std::find(mapTrustKeys[cKey].hashPrevBlocks.begin(), mapTrustKeys[cKey].hashPrevBlocks.end(), std::make_pair(cBlock.GetHash(), false) );

            if(itFalse != mapTrustKeys[cKey].hashPrevBlocks.end())
                (*itFalse).second = true;
            else //Accept key if genesis not found
            {
                if(!Accept(cBlock, fInit))
                    return error("CTrustPool::Connect() : Failed to Accept Genesis Key");

                return Connect(cBlock, fInit);
            }

            /* Create the Trust Key from Genesis Transaction Block. */
            CTrustKey cTrustKey(vKeys[0], cBlock.GetHash(), cBlock.vtx[0].GetHash(), cBlock.nTime);
            if(!cTrustKey.CheckGenesis(cBlock))
                return error("CTrustPool::check() : Invalid Genesis Transaction.");

            /* Only Debug when Not Initializing. */
            if(GetArg("-verbose", 0) >= 1 && !fInit) {
                printf("CTrustPool::Connect() : New Genesis Coinstake Transaction From Block %u\n", cBlock.nHeight);
            }
        }

        /* Handle Adding Trust Transactions. */
        else if(cBlock.vtx[0].IsTrust())
        {
            /* No Trust Transaction without a Genesis. */
            if(!mapTrustKeys.count(cKey))
                return error("CTrustPool::Connect() : Cannot Create Trust Transaction without Genesis.");

            /* Check that the Trust Key and Current Block match. */
            if(mapTrustKeys[cKey].vchPubKey != vKeys[0])
                return error("CTrustPool::Connect() : Trust Key and Block Key Mismatch.");

            /* Trust Keys can only exist after the Genesis Transaction. */
            if(!mapBlockIndex.count(mapTrustKeys[cKey].hashGenesisBlock))
                return error("CTrustPool::Connect() : Block Not Found.");

            /* Don't allow Expired Trust Keys. Check Expiration from Previous Block Timestamp. */
            if(mapTrustKeys[cKey].Expired(cBlock.GetHash(), cBlock.hashPrevBlock))
                return error("CTrustPool::Connect() : Cannot Create Block for Expired Trust Key.");

            /* Don't allow Blocks Created without First Input Previous Output hash of Trust Key Hash.
                This Secures and Anchors the Trust Key to all Descending Trust Blocks of that Key. */
            if(cBlock.vtx[0].vin[0].prevout.hash != mapTrustKeys[cKey].GetHash()) {

                return error("CTrustPool::Connect() : Trust Block Input Hash Mismatch to Trust Key Hash\n%s\n%s", cBlock.vtx[0].vin[0].prevout.hash.ToString().c_str(), mapTrustKeys[cKey].GetHash().ToString().c_str());
            }

            /* Read the Genesis Transaction's Block from Disk. */
            CBlock cBlockGenesis;
            if(!cBlockGenesis.ReadFromDisk(mapBlockIndex[mapTrustKeys[cKey].hashGenesisBlock]->nFile, mapBlockIndex[mapTrustKeys[cKey].hashGenesisBlock]->nBlockPos, true))
                return error("CTrustPool::Connect() : Could not Read Previous Block.");

            /* Double Check the Genesis Transaction. */
            if(!mapTrustKeys[cKey].CheckGenesis(cBlockGenesis))
                return error("CTrustPool::Connect() : Invalid Genesis Transaction.");

            /* Don't allow Blocks Created Before Minimum Interval. */
            if((cBlock.nHeight - mapBlockIndex[mapTrustKeys[cKey].Back()]->nHeight) < TRUST_KEY_MIN_INTERVAL)
                return error("CTrustPool::Connect() : Trust Block Created Before Minimum Trust Key Interval.");

            std::vector< std::pair<uint1024, bool> >::iterator itFalse = std::find(mapTrustKeys[cKey].hashPrevBlocks.begin(), mapTrustKeys[cKey].hashPrevBlocks.end(), std::make_pair(cBlock.GetHash(), false) );

            if(itFalse != mapTrustKeys[cKey].hashPrevBlocks.end())
                (*itFalse).second = true;
            else
            {
                std::vector< std::pair<uint1024, bool> >::iterator itTrue = std::find(mapTrustKeys[cKey].hashPrevBlocks.begin(), mapTrustKeys[cKey].hashPrevBlocks.end(), std::make_pair(cBlock.GetHash(), true) );

                if(itTrue == mapTrustKeys[cKey].hashPrevBlocks.end())
                {
                    if(!Accept(cBlock, fInit))
                        return error("CTrustPool::Connect() : Failed to Accept Trust Key");

                    return Connect(cBlock, fInit);
                }
            }
        }

        /* Dump the Trust Key to Console if not Initializing. */
        if(!fInit && GetArg("-verbose", 0) >= 2)
            mapTrustKeys[cKey].Print();

        /* Only Debug when Not Initializing. */
        if(!fInit && GetArg("-verbose", 0) >= 1)
            printf("CTrustPool::ACCEPTED %s\n", cKey.ToString().substr(0, 20).c_str());

        return true;
    }


    /** Remove a Block from Trust Key. **/
    bool CTrustPool::Disconnect(CBlock cBlock, bool fInit)
    {
        /** Lock Accepting Trust Keys to Mutex. **/
        LOCK(cs);

        /** Extract the Key from the Script Signature. **/
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustPool::Disconnect() : Failed To Solve Trust Key Script.");

        /** Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. **/
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustPool::Disconnect() : Trust Key must be of Public Key Type Created from Keypool.");

        /** Set the Public Key Integer Key from Bytes. **/
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /** Handle Genesis Transaction Rules. Genesis is checked after Trust Key Established. **/
        if(cBlock.vtx[0].IsGenesis())
        {
            /** Only Remove Trust Key from Map if Key Exists. **/
            if(!mapTrustKeys.count(cKey))
                return error("CTrustPool::Disconnect() : Key %s Doesn't Exist in Trust Pool\n", cKey.ToString().substr(0, 20).c_str());

            /** Remove the Trust Key from the Trust Pool. **/
            mapTrustKeys.erase(cKey);

            if(GetArg("-verbose", 0) >= 2)
                printf("CTrustPool::Disconnect() : Removed Genesis Trust Key %s From Trust Pool\n", cKey.ToString().substr(0, 20).c_str());

            return true;
        }

        /** Handle Adding Trust Transactions. **/
        else if(cBlock.vtx[0].IsTrust())
        {
            /** Get the Index of the Block in the Trust Key. **/
            std::vector< std::pair<uint1024, bool> >::iterator it = std::find(mapTrustKeys[cKey].hashPrevBlocks.begin(), mapTrustKeys[cKey].hashPrevBlocks.end(), std::make_pair(cBlock.GetHash(), true) );

            if(it == mapTrustKeys[cKey].hashPrevBlocks.end())
            {
                std::vector< std::pair<uint1024, bool> >::iterator itFalse = std::find(mapTrustKeys[cKey].hashPrevBlocks.begin(), mapTrustKeys[cKey].hashPrevBlocks.end(), std::make_pair(cBlock.GetHash(), false) );
                if(itFalse == mapTrustKeys[cKey].hashPrevBlocks.end())
                    return error("CTrustPool::Disconnect() Block %s not found in Trust Key", cBlock.GetHash().ToString().substr(0, 20).c_str());
            }
            else
                (*it).second = false;

            printf("CTrustPool::Disconnect() : Removed Block %s From Trust Key\n", cBlock.GetHash().ToString().substr(0, 20).c_str());

            return true;
        }

        return false;
    }


    /** Accept a Block's Coinstake into the Trust Pool Assigning it to Specified Trust Key.
        This Method shouldn't be called before CTrustPool::Check **/
    bool CTrustPool::Accept(CBlock cBlock, bool fInit)
    {
        /* Lock Accepting Trust Keys to Mutex. */
        LOCK(cs);

        /* Extract the Key from the Script Signature. */
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustPool::accept() : Failed To Solve Trust Key Script.");

        /* Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. */
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustPool::accept() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Set the Public Key Integer Key from Bytes. */
        uint576 cKey;
        cKey.SetBytes(vKeys[0]);

        /* Handle Genesis Transaction Rules. Genesis is checked after Trust Key Established. */
        if(cBlock.vtx[0].IsGenesis())
        {
            /* Add the New Trust Key to the Trust Pool Memory Map. */
            CTrustKey cTrustKey(vKeys[0], cBlock.GetHash(), cBlock.vtx[0].GetHash(), cBlock.nTime);

            /* Set the first block as genesis and connected flag as false. */
            cTrustKey.hashPrevBlocks.push_back(std::make_pair(cBlock.GetHash(), false));

            /* Add the key to the trust pool. */
            mapTrustKeys[cKey] = cTrustKey;

            /* Dump the Trust Key To Console if not Initializing. */
            if(!fInit && GetArg("-verbose", 0) >= 2)
                cTrustKey.Print();

            /* Only Debug when Not Initializing. */
            if(GetArg("-verbose", 0) >= 1 && !fInit) {
                printf("CTrustPool::accept() : New Genesis Coinstake Transaction From Block %u\n", cBlock.nHeight);
                printf("CTrustPool::ACCEPTED %s\n", cKey.ToString().substr(0, 20).c_str());
            }

            return true;
        }

        /** Handle Adding Trust Transactions. **/
        else if(cBlock.vtx[0].IsTrust())
        {
            /** Add the new block to the Trust Key. **/
            mapTrustKeys[cKey].hashPrevBlocks.push_back(std::make_pair(cBlock.GetHash(), false));

            /** Only Debug when Not Initializing. **/
            if(!fInit && GetArg("-verbose", 0) >= 1) {
                printf("CTrustPool::ACCEPTED %s\n", cKey.ToString().substr(0, 20).c_str());
            }

            return true;
        }

        return error("CTrustPool::accept() : Missing Trust or Genesis Transaction in Block.");
    }


    /** Interest is Determined By Logarithmic Equation from Genesis Key. **/
    double CTrustPool::InterestRate(uint576 cKey, unsigned int nTime) const
    {
        /** Genesis and First Trust Block awarded 0.5% interest. **/
        if(!Exists(cKey) || IsGenesis(cKey)) //TODO detect genesis flag with block hash
            return 0.005;

        return min(0.03, (((0.025 * log(((9.0 * (nTime - Find(cKey).nGenesisTime)) / (60 * 60 * 24 * 28 * 13)) + 1.0)) / log(10))) + 0.005);
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
            return error("CTrustKey::CheckGenesis() : Genesis Key Invalid for non Proof of Stake blocks.");

        /* Trust Key Timestamp must be the same as Genesis Key Block Timestamp. */
        if(nGenesisTime != cBlock.nTime)
            return error("CTrustKey::CheckGenesis() : Time Mismatch for Trust key to Genesis Trust Block");

        /* Genesis Key Transaction must match Trust Key Genesis Hash. */
        if(cBlock.vtx[0].GetHash() != hashGenesisTx)
            return error("CTrustKey::CheckGenesis() : Genesis Key Hash Mismatch to Genesis Transaction Hash");

        /* Extract the Key from the Script Signature. **/
        vector< std::vector<unsigned char> > vKeys;
        Wallet::TransactionType keyType;
        if (!Wallet::Solver(cBlock.vtx[0].vout[0].scriptPubKey, keyType, vKeys))
            return error("CTrustKey::IsInvalid() : Failed To Solve Trust Key Script.");

        /* Ensure the Key is Public Key. No Pay Hash or Script Hash for Trust Keys. */
        if (keyType != Wallet::TX_PUBKEY)
            return error("CTrustKey::CheckGenesis() : Trust Key must be of Public Key Type Created from Keypool.");

        /* Set the Public Key. */
        if(vKeys[0] != vchPubKey)
            return error("CTrustKey::CheckGenesis() : Trust Key Public Key and Genesis Trust Block Public Key Mismatch\n");

        return true;
    }


    /* Key is Expired if Time between Network Previous Best Block and
     Trust Best Previous is Greater than Expiration Time. */
    bool CTrustKey::Expired(uint1024 hashThisBlock, uint1024 hashPrevBlock) const
    {
        if(BlockAge(hashThisBlock, hashPrevBlock) > TRUST_KEY_EXPIRE)
            return true;

        return false;
    }


    /** Key is Expired if it is Invalid or Time between Network Best Block and Best Previous is Greater than Expiration Time. **/
    uint64 CTrustKey::Age(unsigned int nTime) const
    {
        if(nGenesisTime == 0)
            return 0;

        /* Catch overflow attacks. */
        if(nGenesisTime > nTime)
            return 1;

        return (uint64)(nTime - nGenesisTime);
    }


    /* The Age of a Key in Block age as in the Time it has been since Trust Key has produced block. */
    uint64 CTrustKey::BlockAge(uint1024 hashThisBlock, uint1024 hashPrevBlock) const
    {
        /* Genesis Transaction Block Age is Time to Genesis Time. */
        if(!mapBlockIndex.count(hashPrevBlock))
<<<<<<< HEAD
            return 0;

=======
            return error("CTrustKey::BlockAge() : %s not in Map Block Index", hashPrevBlock.ToString().c_str());

>>>>>>> 2.4.4-validation
        /* Catch overflow attacks. Should be caught in verify stake but double check here. */
        if(nGenesisTime > mapBlockIndex[hashPrevBlock]->GetBlockTime())
            return error("CTrustKey::BlockAge() : %u Time is < Genesis %u", (unsigned int) mapBlockIndex[hashPrevBlock]->GetBlockTime(), nGenesisTime);

        /* Find the block previous to pindexNew. */
        uint1024 hashBlockLast = Back(hashThisBlock);
        if(!mapBlockIndex.count(hashBlockLast))
            return 0;

        /* Make sure there aren't timestamp overflows. */
        if(mapBlockIndex[hashBlockLast]->GetBlockTime() > mapBlockIndex[hashPrevBlock]->GetBlockTime())
            return BlockAge(hashBlockLast, hashPrevBlock); //recursively look back from last back if block times not satisfied

        /* Block Age is Time to Previous Block's Time. */
        return (uint64)(mapBlockIndex[hashPrevBlock]->GetBlockTime() - mapBlockIndex[hashBlockLast]->GetBlockTime());
    }



    /** Proof of Stake local CPU miner. Uses minimal resources. **/
    void StakeMinter(void* parg)
    {
        printf("Stake Minter Started\n");
        SetThreadPriority(THREAD_PRIORITY_LOWEST);

        // Each thread has its own key and counter
        Wallet::CReserveKey reservekey(pwalletMain);

        while(!fShutdown)
        {
            /* Sleep call to keep the thread from running. */
            Sleep(10);

            /* Don't stake if the wallet is locked. */
            if (pwalletMain->IsLocked())
                continue;

            /* Don't stake if there are no available nodes. */
            if (Net::vNodes.empty() || IsInitialBlockDownload())
                continue;

            /* Lower Level Database Instance. */
            LLD::CIndexDB indexdb("r");

            /* Take a snapshot of the best block. */
            uint1024 hashBest = hashBestChain;

            /* Create the block(s) to work on. */
            CBlock baseBlock = CreateNewBlock(reservekey, pwalletMain, 0);
            if(baseBlock.IsNull())
                continue;

            /* Check the Trust Keys. */
            uint576 cKey;
            if(!cTrustPool.HasTrustKey(pindexBest->GetBlockTime()))
            {
                cKey.SetBytes(reservekey.GetReservedKey());
                baseBlock.vtx[0].vout[0].scriptPubKey << reservekey.GetReservedKey() << Wallet::OP_CHECKSIG;
            }
            else
            {
                cKey.SetBytes(cTrustPool.vchTrustKey);

                baseBlock.vtx[0].vout[0].scriptPubKey << cTrustPool.vchTrustKey << Wallet::OP_CHECKSIG;
                baseBlock.vtx[0].vin[0].prevout.n = 0;
                baseBlock.vtx[0].vin[0].prevout.hash = cTrustPool.Find(cKey).GetHash();
            }


            /* Determine Trust Age if the Trust Key Exists. */
            uint64 nCoinAge = 0, nTrustAge = 0, nBlockAge = 0;
            double nTrustWeight = 0.0, nBlockWeight = 0.0;
            if(cTrustPool.Exists(cKey))
            {
                nTrustAge = cTrustPool.Find(cKey).Age(pindexBest->GetBlockTime());
                nBlockAge = cTrustPool.Find(cKey).BlockAge(0, pindexBest->GetBlockHash());

                /* Trust Weight Reaches Maximum at 30 day Limit. */
                nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nTrustAge) / (60 * 60 * 24 * 28)) + 1.0)) / log(3))) + 1.0);

                /* Block Weight Reaches Maximum At Trust Key Expiration. */
                nBlockWeight = min(20.0, (((19.0 * log(((2.0 * nBlockAge) / (TRUST_KEY_EXPIRE)) + 1.0)) / log(3))) + 1.0);
            }
            else
            {
                /* Calculate the Average Coinstake Age. */
                CTransaction txNew = baseBlock.vtx[0];
                if (!pwalletMain->AddCoinstakeInputs(txNew))
                {
                    if(GetArg("-verbose", 0) >= 2)
                        error("Stake Minter : Genesis - Failed to Add Coinstake Inputs");

                    Sleep(1000);

                    continue;
                }

                if(!txNew.GetCoinstakeAge(indexdb, nCoinAge))
                {
                    if(GetArg("-verbose", 0) >= 2)
                        error("Stake Minter : Genesis - Failed to Get Coinstake Age.");

                    Sleep(1000);

                    continue;
                }


                /** Trust Weight For Genesis Transaction Reaches Maximum at 90 day Limit. **/
                nTrustWeight = min(17.5, (((16.5 * log(((2.0 * nCoinAge) / (60 * 60 * 24 * 28 * 3)) + 1.0)) / log(3))) + 1.0);
            }

            /* Get the Total Weight. */
            int combinedWeight = floor(nTrustWeight + nBlockWeight);
            int nTotalWeight = max(combinedWeight, 8);


            /* Make sure coinstake is created. */
            int i = 0;

            /* Copy the block pointers. */
            std::vector<CBlock> block;
            block.resize(nTotalWeight);

            for(i = 0; i < nTotalWeight; i++)
            {
                block[i] = baseBlock;
                if (!pwalletMain->AddCoinstakeInputs(block[i].vtx[0]))
                    break;

                if (!block[i].vtx[0].IsGenesis())
                    AddTransactions(block[i].vtx, pindexBest);

                block[i].hashMerkleRoot   = block[i].BuildMerkleTree();
            }

            /* Retry if coinstake wasn't created properly. */
            if(i != nTotalWeight)
                continue;

            /* Assigned Extracted Key to Trust Pool. */
            if(GetArg("-verbose", 0) >= 0 && cTrustPool.Exists(cKey))
                printf("Stake Minter : Active Trust Key %s\n", HexStr(cTrustPool.vchTrustKey.begin(), cTrustPool.vchTrustKey.end()).c_str());

            if(GetArg("-verbose", 0) >= 2)
            {
                printf("Stake Minter : Created New Block %s\n", block[0].GetHash().ToString().substr(0, 20).c_str());
                printf("Stake Minter : Total Nexus to Stake %f at %f %% Variable Interest\n", (double)block[0].vtx[0].GetValueOut() / COIN, cTrustPool.InterestRate(cKey, pindexBest->GetBlockTime()) * 100.0);
            }


            /* Set the Reporting Variables for the Qt. */
            dTrustWeight = nTrustWeight;
            dBlockWeight = nBlockWeight;
<<<<<<< HEAD


=======


>>>>>>> 2.4.4-validation
            if(GetArg("-verbose", 0) >= 0)
                printf("Stake Minter : Staking at Total Weight %u | Trust Weight %f | Block Weight %f | Coin Age %" PRIu64 " | Trust Age %" PRIu64 "| Block Age %" PRIu64 "\n", nTotalWeight, nTrustWeight, nBlockWeight, nCoinAge, nTrustAge, nBlockAge);

            bool fFound = false;
            while(!fFound)
            {
                Sleep(120);

                if(hashBestChain != hashBest)
                {
                    if(GetArg("-verbose", 0) >= 2)
                        printf("Stake Minter : New Best Block\n");

                    break;
                }

                for(int i = 0; i < nTotalWeight; i++)
                {

                    /* Update the block time for difficulty accuracy. */
                    block[i].UpdateTime();
                    if(block[i].nTime == block[i].vtx[0].nTime)
                        continue;

                    /* Calculate the Efficiency Threshold. */
                    double nThreshold = (double)((block[i].nTime - block[i].vtx[0].nTime) * 100.0) / (block[i].nNonce + 1); //+1 to account for increment if that nNonce is chosen
                    double nRequired  = ((50.0 - nTrustWeight - nBlockWeight) * MAX_STAKE_WEIGHT) / std::min((int64)MAX_STAKE_WEIGHT, block[i].vtx[0].vout[0].nValue);

                    /* Allow the Searching For Stake block if Below the Efficiency Threshold. */
                    if(nThreshold < nRequired)
                        continue;

                    block[i].nNonce ++;

                    CBigNum hashTarget;
                    hashTarget.SetCompact(block[i].nBits);

                    if(block[i].nNonce % (unsigned int)((nTrustWeight + nBlockWeight) * 5) == 0 && GetArg("-verbose", 0) >= 3)
                        printf("Stake Minter : Below Threshold %f Required %f Incrementing nNonce %" PRIu64 "\n", nThreshold, nRequired, block[i].nNonce);

                    if (block[i].GetHash() < hashTarget.getuint1024())
                    {

                        /* Sign the new Proof of Stake Block. */
                        if(GetArg("-verbose", 0) >= 0)
                            printf("Stake Minter : Found New Block Hash %s\n", block[i].GetHash().ToString().substr(0, 20).c_str());

                        if (!block[i].SignBlock(*pwalletMain))
                        {
                            if(GetArg("-verbose", 0) >= 1)
                                printf("Stake Minter : Could Not Sign Proof of Stake Block.");

                            break;
                        }

                        if(!cTrustPool.Check(block[i]))
                        {
                            if(GetArg("-verbose", 0) >= 1)
                                error("Stake Minter : Check Trust Key Failed...");

                            break;
                        }

                        if (!block[i].CheckBlock())
                        {
                            if(GetArg("-verbose", 0) >= 1)
                                error("Stake Minter : Check Block Failed...");

                            break;
                        }

                        if(GetArg("-verbose", 0) >= 1)
                            block[i].print();

                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        CheckWork(&block[i], *pwalletMain, reservekey);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);

                        fFound = true;
                    }
                }
            }
        }
    }
}
