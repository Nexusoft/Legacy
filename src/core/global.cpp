/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

[Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "../main.h"

using namespace std;

namespace Core
{

    /** Hash to start the Main Net Blockchain. **/
    const uint1024 hashGenesisBlockOfficial("0x00000bb8601315185a0d001e19e63551c34f1b7abeb3445105077522a9cbabf3e1da963e3bfbb87d260218b7c6207cb9c6df90b86e6a3ce84366434763bc8bcbf6ccbd1a7d5958996aecbe8205c20b296818efb3a59c74acbc7a2d1a5a6b64aab63839b8b11a6b41c4992f835cbbc576d338404fb1217bdd7909ca7db63bbc02");


    /** Hash to start the Test Net Blockchain. **/
    const uint1024 hashGenesisBlockTestNet("0x00002a0ccd35f2e1e9e1c08f5a2109a82834606b121aca891d5862ba12c6987d55d1e789024fcd5b9adaf07f5445d24e78604ea136a0654497ed3db0958d63e72f146fae2794e86323126b8c3d8037b193ce531c909e5222d090099b4d574782d15c135ddd99d183ec14288437563e8a6392f70259e761e62d2ea228977dd2f7");


    /** The current Block Version Activating in this Release. **/
    const unsigned int NETWORK_BLOCK_CURRENT_VERSION = 6;
    const unsigned int TESTNET_BLOCK_CURRENT_VERSION = 6;


    /** Nexus Max Block size is 2 MB. This is to stay consistent with Nexus's 1 MB limit with 256 bit hashes, where Nexus Transactions are 512 bit. **/
    const unsigned int MAX_BLOCK_SIZE = 2000000;
    const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
    const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
    const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;

    /** Nexus Transactions are Free for Everyone. **/
    const int64 MIN_TX_FEE = CENT;
    const int64 MIN_RELAY_TX_FEE = CENT;

    /** Maximum Coins that can be Sent in 1 Transaction is 1 Million Nexus. **/
    const int64 MAX_TXOUT_AMOUNT = 1000000 * COIN;


    /** Maximum Coins that can be Sent in 1 Transaction is 50 Million Nexus. **/
    const int64 MAX_TRANSACTION_AMOUNT = 50000000 * COIN;


    /** Minimum Output can be 1 Satoshi. **/
    const int64 MIN_TXOUT_AMOUNT = 1;


    /** Nexus New Supply Matures in 100 Blocks. **/
    const int COINBASE_MATURITY = 100;


    /** TODO: Remove this Threshold. **/
    const int LOCKTIME_THRESHOLD = 500000000;


    /** Proof of Stake blocks Set to 1 Minute Span. **/
    const int STAKE_TARGET_SPACING = 150;


    /** Trust Key Expiration Marks the Maximum Time between Trust Blocks. **/
    int TRUST_KEY_EXPIRE   = 60 * 60 * 24;


    /* nVersion > 4 - timespan is 3 days **/
    int TRUST_KEY_TIMESPAN = 60 * 60 * 24 * 3;


    /* nVersion > 4 - timespan is 30 minutes for testnet **/
    int TRUST_KEY_TIMESPAN_TESTNET = 60 * 30;


    /* Difficulty Trheshold Weight for Trust Keys. MAX_TIMESPAN is influenced linearly with this number. */
    double TRUST_KEY_DIFFICULTY_THRESHOLD = 70.4;


    /* NOTE: For Blocks Version 5 and Above
    * The Maximum tolerance that a trust key can have for inconsistent behavior. */
    double TRUST_KEY_MIN_CONSISTENCY_TOLERANCE = 0.8;
    double TRUST_KEY_MAX_CONSISTENCY_TOLERANCE = 1.3;


    /* NOTE: For Blocks Version 5 and Above
    * The Maximum Blocks to Check Backwards for Consistency Tolerance. */
    int TRUST_KEY_CONSISTENCY_HISTORY = 50;


    /** The Minimum Number of Blocks Between Trust Key's Staking Blocks. **/
    int TRUST_KEY_MIN_INTERVAL = 5;


    /** Set the Maximum Output Value of Coinstake Transaction. **/
    const uint64 MAX_STAKE_WEIGHT = 1000 * COIN;


    /** Minimum span between trust blocks testnet. **/
    int TESTNET_MINIMUM_INTERVAL = 3;


    /** Minimum span between trust blocks mainnet. **/
    int MAINNET_MINIMUM_INTERVAL = 120;


    /** Time - Lock for the Nexus Network Launch. Allowed Binary Distribution before Time-Lock but no Mining until after Time-Lock. **/
    const unsigned int NEXUS_TESTNET_TIMELOCK  = 1421250000;   //--- Nexus Testnet Activation:        1/14/2015 08:38:00 GMT - 6
    const unsigned int NEXUS_NETWORK_TIMELOCK  = 1411510800;   //--- Nexus Network Launch:           09/23/2014 16:20:00 GMT - 6


    /* TestNet Time - Lock for the First Nexus Block Update. Version 1 Blocks Rejected after this Time - Lock, Version 2 Blocks Rejected Before it. */
    const unsigned int TESTNET_VERSION_TIMELOCK[]   = { 1412676000,        //--- Block Version 2 Testnet Activation:  10/07/2014 04:00:00 GMT - 6
                                                        1421293891,        //--- Block Version 3 Testnet Activation:  01/15/2015 07:51:31 GMT - 6
                                                        1421949600,        //--- Block Version 4 Testnet Activation:  05/10/2015 08:01:00 GMT - 6
                                                        1536562800,        //--- Block Version 5 Testnet Activation:  09/10/2018 00:00:00 GMT - 7
                                                        1537167600 };      //--- Block Version 6 Testnet Activation:  09/17/2018 00:00:00 GMT - 7

    /* Network Time - Lock for the First Nexus Block Update. Version 1 Blocks Rejected after this Time - Lock, Version 2 Blocks Rejected Before it. */
    const unsigned int NETWORK_VERSION_TIMELOCK[]   = { 1412964000,        //--- Block Version 2 Activation:          10/10/2014 12:00:00 GMT - 6
                                                        1421949600,        //--- Block Version 3 Activation:          01/22/2015 12:00:00 GMT - 6
                                                        1438369200,        //--- Block Version 4 Activation:          07/31/2015 12:00:00 GMT - 7
                                                        1536977460,        //--- Block Version 5 Activation:          09/14/2018 19:11:00 GMT - 7
                                                        1538791860 };      //--- Block Version 6 Activation:          10/05/2018 19:11:00 GMT - 7


    /** Time - Lock for the Nexus Channels on the Testnet. Each Channel Cannot produce blocks before their corresponding Time - Locks. **/
    const unsigned int CHANNEL_TESTNET_TIMELOCK[] = {   1421949600,        //--- POS Testnet Activation:              05/10/2015 08:01:00 GMT - 6
                                                        1411437371,        //--- CPU Testnet Activation:              09/22/2014 18:56:11 GMT - 6
                                                        1411437371 };      //--- GPU Testnet Activation:              09/22/2014 18:56:11 GMT - 6

    /** Time - Lock for the Nexus Channels. Each Channel Cannot produce blocks before their corresponding Time - Locks. **/
    const unsigned int CHANNEL_NETWORK_TIMELOCK[] = {   1438369200,        //--- POS Channel Activation:              07/31/2015 12:00:00 GMT - 7
                                                        1411510800,        //--- CPU Channel Activation:              09/23/2014 16:20:00 GMT - 6
                                                        1413914400 };      //--- GPU Channel Activation:              10/21/2014 12:00:00 GMT - 6




    /** Dummy address for running on the Testnet. **/
    const string TESTNET_DUMMY_ADDRESS             = "4jzgcyvCM6Yv8uoAPwCwe5eSikccs7ofJBnxsRWtmePGuJYnV8E";

    /** new testnet dummy addresses. */
    const string TESTNET_DUMMY_AMBASSADOR_RECYCLED = "4kRwiTAu6h3ZPTABfZ7wYjVfovHWWHJxShATUYCYYsSWVdiuCYa";
    const string TESTNET_DUMMY_DEVELOPER_RECYCLED  = "4kUF9T3tCMFtRPyoFX5Kyhn6BVwxi5dgnVyDwPxUr8kZpwdr6Zy";


    /** Signature to Check Testnet Blocks are Produced Correctly. **/
    std::vector<unsigned char> TESTNET_DUMMY_SIGNATURE;

    /** new testnet dummy signatures. **/
    std::vector<unsigned char> TESTNET_DUMMY_SIGNATURE_AMBASSADOR_RECYCLED;
    std::vector<unsigned char> TESTNET_DUMMY_SIGNATURE_DEVELOPER_RECYCLED;


    /** Addresses of the Exchange Channels. **/
    const string CHANNEL_ADDRESSES[] =  {   "2Qn8MsUCkv9S8X8sMvUPkr9Ekqqg5VKzeKJk2ftcFMUiJ5QnWRc",
                                            "2S4WLATVCdJXTpcfdkDNDVKK5czDi4rjR5HrCRjayktJZ4rN8oA",
                                            "2RE29WahXWawQ9huhyaGhfvEMmUWHH9Hfo1anbNk8eW3nTU7H2g",
                                            "2QpSfg6MBZYCjQKXjTgo9eHoKMCJsYjLQsNT3xeeAYhrQmNBEUd",
                                            "2RHjigCh1qt1j3WKz4mShFBiVE5g6z9vrFpGMT6EDQsFJbtx4hr",
                                            "2SZ87FB1zukH5h7BLDT4yUyMTQnEJEt2KzpGYFxuUzMqAxEFN7Y",
                                            "2STyHuCeBgE81ZNjhH5QB8UXViXW7WPYM1YQgmXfLvMJXaKAFCs",
                                            "2SLq49uDrhLyP1N7Xnkj86WCHQUKxn6zx38LBNoTgwsAjfV1seq",
                                            "2RwtQdi3VPPQqht15QmXnS4KELgxrfaH2hXSywtJrfDdCJMnwPQ",
                                            "2SWscUR6rEezZKgFh5XkEyhmRmja2qrHMRsfmtxdapwMymmM96Q",
                                            "2SJzPMXNPEgW2zJW4489qeiCjdUanDTqCuSNAMmZXm1KX269jAt",
                                            "2Rk2mBEYWkGDMzQhEqdpSGZ77ZGvp9HWAbcsY6mDtbWKJy4DQuq",
                                            "2Rnh3qFvzeRQmSJEHtz6dNphq3r7uDSGQdjucnVFtpcuzBbeiLx" };


    /** Addresses for the Developer Accounts. **/
    const string DEVELOPER_ADDRESSES[] = {  "2Qp1rHzLCCsL4RcmLRcNhhAjnFShDXQaAjyBB9YpDSomCJsfGfS",
                                            "2SFx2tc8tLHBtkLkfK7nNjkU9DwvZZMNKKLaeX4xcG8ev4HQqVP",
                                            "2SawW67sUcVtLNarcAkVaFR2L1R8AWujkoryJHi8L47bdDP8hwC",
                                            "2QvzSNP3jy4MjqmB7jRy6jRGrDz6s6ALzTwu8htdohraU6Fdgrc",
                                            "2RxmzQ1XomQrbzQimajfnC2FubYnSzbwz5EkU2du7bDxuJW7i2L",
                                            "2S2JaEbGoBFx7N2YGEUJbWRjLf35tY7kezV8R9vzq9Wu1f5cwVz",
                                            "2S9bR5wB6RcBm1weFPBXCZai5pRFisa9zQSEedrdi9QLmd5Am8y",
                                            "2S6NjGDuTozyCWjMziwEBYKnnaC6fy5zRjDmd2NQhHBjuzKw4bg",
                                            "2RURDVPFD14eYCC7brgio2HF77LP22SdN5CeAvwQAwdSPdS95dT",
                                            "2SNAEJ6mbmpPXbP6ZkmH7FgtWTWcNnw2Pcs3Stb4PDaq3vH1GgE",
                                            "2SDnQuMgW9UdESUUekrrWegxSHLQWnFWJ2BNWAUQVecKNwBeNh5",
                                            "2SCLU4SKxh2P27foN9NRoAdtUZMdELMvBpfmVER98HayRRqGKFx",
                                            "2SLN8urU2mERZRQajqYe9VgQQgK7tPWWQ1679c5B3scZKP2vDxi" };


    /** Binary Data of Each Developer Address for Signature Verification of Exchange Transaction in Accept Block. **/
    std::vector<unsigned char> AMBASSADOR_SCRIPT_SIGNATURES[13];
    std::vector<unsigned char> DEVELOPER_SCRIPT_SIGNATURES[13];


    /* New ambassador addresses recycled. */
    const string AMBASSADOR_ADDRESSES_RECYCLED[] = {    "2RSWG4zGzJZdkem23CeuqSEVjjbwUbVe2oZRpcA5ZpSqTojzQYy",
                                                        "2RmF9e5k2W4RvKsZsKXK8y6Md1Hd5joNQFrrEKeLMpv3CfFMjQZ",
                                                        "2QyxbcfCckkr5HQzxGqrKdChnVwXzwuAjt8ADMBYw5i3jxYu96H",
                                                        "2Ruf4e6FWEkJKoPewJ9DPi5gjgLCR5a8NGvFd99ycsPjxmrzbo2",
                                                        "2S7diRGZQF9nwmJ3J1hH1Zapgvo551eBtPT69arsSLrhuyqfpwR",
                                                        "2RdnyGpVhQarMswVj9We7eNs6TTazWEcnFCwVyK7zReg5hKq8kc",
                                                        "2RHsaGpRjSheYDGpBsvDPyW5tSuQjHXkBdbZZ6QLpLBhcWHJyUZ",
                                                        "2RhMNr6qaEnQndMUiuSwBpNToQovcDbVK6FDGkxrMALdcPY1zWv",
                                                        "2SaPcUuSMj7J7szWeuH5ZHhPg9oQGtfJFrU7Prezkw6aqENXGvc",
                                                        "2Rjsdzb6HPPNtjCmDbJ1fNHFKNmQLfxVGYqE1gCdfPnJh78bKwz",
                                                        "2RsG4LMrMCCuPnsy21FRSXrbXRVLJWguQbzL4aZFJEWSkvu75Qb",
                                                        "2QmAPn1ymoJj2UVYGedetEkN3WkPXPDL1Tn13W6vAoDS71543UJ",
                                                        "2RbJ6uqpnVmNvzzHz73v6m3J4M3ks3Jv6E7bDZhTgtXkSTEsiob" };


    /* New developer addresses recycled. */
    const string DEVELOPER_ADDRESSES_RECYCLED[] = {     "2SPinzyuXJdf9iFchK4fvH7Fcu6SLqTasugDaPpUTrV4WDo27Vx",
                                                        "2RS4jz5TdHNvhnQPGQCfhsddyT6PXc4tip2uRQ61hMK9dkfFZE4",
                                                        "2Qip9FFJH3CjhHLv7ZfxpjenKohmbEz2zVe27TfE5gqT1rg8Y8P",
                                                        "2R7gedizZWpe9RySUXmxVznoJJie3c7SypBVHkRZ8Dbo6mBj8zw",
                                                        "2Rz6Z6XMPH8A2iyeKnYS14cV9rHDwGWPCs2z4EZ6Qc4FX8DBdE3",
                                                        "2SRto8HnG6GfXY6m37zxSbKD9shAXYtdWn62umezHhC85i61LKB",
                                                        "2Rdfzmijbn8m7yWRkhWdsfJJ78XSAp7VPks2Ci18x98hFfru4bN",
                                                        "2S7JbY11Kfss3A2bhchzzxeervQZ6JpGDSC7FBJ3oXq1xoCbHse",
                                                        "2RVKmatLPGP6iWGvMdXzQrkrkNcMkgeoHXdJafrg7UTHhjxFLcR",
                                                        "2S81rREFrxgGoBhp6g7yi14sKpnDNEnwp28q9Eqg1KG5JMvRjXF",
                                                        "2ReZqUDgSMBFJ6W4qGrDDxecmgChYt2RZjonAT7eNzUdCZQyHA9",
                                                        "2QkNsRC3jsqCSeeUpJgCusMX11QTcUHYNrh798HsSdWTyFQ2du3",
                                                        "2Qv9haWgvomJkawpy2EDDmtCaE3rXpnvtH4pRRuM8JgJQTxoCt8" };

    /* Binary Data of Each Developer Address for Signature Verification of Exchange Transaction in Accept Block. */
    std::vector<unsigned char> AMBASSADOR_SCRIPT_SIGNATURES_RECYCLED[13];
    std::vector<unsigned char> DEVELOPER_SCRIPT_SIGNATURES_RECYCLED[13];



    //TODO: Recycled Byte Code for new addresses

    CCriticalSection cs_setpwalletRegistered;

    /** List of Registered Transactions to Queue adding to Wallet. **/
    set<Wallet::CWallet*> setpwalletRegistered;

    /** Main Mutex to enable thread Synchronization and Queueing while working on main Data Objects. **/
    CCriticalSection cs_main;

    /** Transaction Holding Structure to wait to be confirmed in the Network. **/
    CTxMemPool mempool;

    /** In memory Indexing of Blocks into Blockchain. **/
    map<uint1024, CBlockIndex*> mapBlockIndex;
    map<uint1024, uint1024>     mapInvalidBlocks;

    /** In Memory Holdings of each Address Balance. **/
    map<uint256, std::vector<std::pair<bool, uint512> > > mapRichList;
    map<uint256, uint64> mapAddressTransactions;

    /** Anchored Genesis Block to start the Chain. **/
    uint1024 hashGenesisBlock = hashGenesisBlockOfficial;

    /** Initial Difficulty Adjustments. **/
    CBigNum bnProofOfWorkLimit[] = { CBigNum(~uint1024(0) >> 5), CBigNum(20000000), CBigNum(~uint1024(0) >> 17) };
    CBigNum bnProofOfWorkStart[] = { CBigNum(~uint1024(0) >> 7), CBigNum(25000000), CBigNum(~uint1024(0) >> 22) };
    CBigNum bnPrimeMinOrigins    =  CBigNum(~uint1024(0) >> 8); //minimum prime origins of 1016 bits

    int nCoinbaseMaturity = COINBASE_MATURITY;
    CBlockIndex* pindexGenesisBlock = NULL;
    unsigned int nBestHeight = 0;

    uint64 nBestChainTrust = 0;
    uint1024 hashBestChain = 0;
    CBlockIndex* pindexBest = NULL;
    int64 nTimeBestReceived = 0;


    double dTrustWeight = 0.0;
    double dBlockWeight = 0.0;
    double dInterestRate = 0.005;
    bool fIsWaitPeriod = false;
    bool fStakeMinterInitializing = false;
    bool fGracePeriod = false;


    uint64 nLastBlockTx = 0;
    uint64 nLastBlockSize = 0;


    CMajority<int> cPeerBlockCounts; // Amount of blocks that other nodes claim to have

    map<uint1024, CBlock*> mapOrphanBlocks;
    multimap<uint1024, CBlock*> mapOrphanBlocksByPrev;
    map<uint1024, uint1024> mapProofOfStake;

    map<uint512, CDataStream*> mapOrphanTransactions;
    map<uint512, map<uint512, CDataStream*> > mapOrphanTransactionsByPrev;

    const string strMessageMagic = "Nexus Signed Message:\n";

    // Settings
    int64 nTransactionFee = MIN_TX_FEE;

}
