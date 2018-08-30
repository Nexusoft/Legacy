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
    const unsigned int NETWORK_BLOCK_CURRENT_VERSION = 4;
    const unsigned int TESTNET_BLOCK_CURRENT_VERSION = 4;


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


    /* NOTE: For Blocks Version 5 and Above.
    *     The Maximum Grace time before trust begins to be reduced from no trust blocks seen.. **/
    int TRUST_KEY_MAX_TIMESPAN = 60 * 60 * 8;


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


    /** Time - Lock for the Nexus Network Launch. Allowed Binary Distribution before Time-Lock but no Mining until after Time-Lock. **/
    const unsigned int NEXUS_TESTNET_TIMELOCK  = 1421250000;   //--- Nexus Testnet Activation:        1/14/2015 08:38:00 GMT - 6
    const unsigned int NEXUS_NETWORK_TIMELOCK  = 1411510800;   //--- Nexus Network Launch:           09/23/2014 16:20:00 GMT - 6


    /* TestNet Time - Lock for the First Nexus Block Update. Version 1 Blocks Rejected after this Time - Lock, Version 2 Blocks Rejected Before it. */
    const unsigned int TESTNET_VERSION_TIMELOCK[]    = {  1412676000,        //--- Block Version 2 Testnet Activation:        10/07/2014 04:00:00 GMT - 6
                                                                            1421293891,        //--- Block Version 3 Testnet Activation:        01/15/2015 07:51:31 GMT - 6
                                                                            1421949600,        //--- Block Version 4 Testnet Activation:        05/10/2015 08:01:00 GMT - 6
                                                                            1482679327 };    //--- Block Version 5 Testnet Activation:        12/25/2016 08:22:07 GMT - 7

    /* Network Time - Lock for the First Nexus Block Update. Version 1 Blocks Rejected after this Time - Lock, Version 2 Blocks Rejected Before it. */
    const unsigned int NETWORK_VERSION_TIMELOCK[]    = {  1412964000,        //--- Block Version 2 Activation:                10/10/2014 12:00:00 GMT - 6
                                                                            1421949600,        //--- Block Version 3 Activation:                01/22/2015 12:00:00 GMT - 6
                                                                            1438369200,        //--- Block Version 4 Activation:                07/31/2015 12:00:00 GMT - 7
                                                                            1483294271 };    //--- Block Version 5 Activation:                01/01/2017 11:11:11 GMT - 7


    /** Time - Lock for the Nexus Channels on the Testnet. Each Channel Cannot produce blocks before their corresponding Time - Locks. **/
    const unsigned int CHANNEL_TESTNET_TIMELOCK[] = {     1421949600,        //--- POS Testnet Activation:              05/10/2015 08:01:00 GMT - 6
                                                                        1411437371,        //--- CPU Testnet Activation:              09/22/2014 18:56:11 GMT - 6
                                                                        1411437371 };    //--- GPU Testnet Activation:              09/22/2014 18:56:11 GMT - 6

    /** Time - Lock for the Nexus Channels. Each Channel Cannot produce blocks before their corresponding Time - Locks. **/
    const unsigned int CHANNEL_NETWORK_TIMELOCK[] = {     1438369200,        //--- POS Channel Activation:              07/31/2015 12:00:00 GMT - 7
                                                                        1411510800,        //--- CPU Channel Activation:              09/23/2014 16:20:00 GMT - 6
                                                                        1413914400 };    //--- GPU Channel Activation:              10/21/2014 12:00:00 GMT - 6




    /** Dummy address for running on the Testnet. **/
    const string TESTNET_DUMMY_ADDRESS   = "4jzgcyvCM6Yv8uoAPwCwe5eSikccs7ofJBnxsRWtmePGuJYnV8E";


    /** Signature to Check Testnet Blocks are Produced Correctly. **/
    const unsigned char TESTNET_DUMMY_SIGNATURE[] = { 0x76, 0xa9, 0x20, 0xa1, 0x55, 0x49, 0x08, 0x63, 0xc2, 0xba, 0xe9, 0xcd, 0x7b, 0xa2, 0x5b, 0x83, 0x46, 0x88, 0xa2 , 0x4d, 0x29, 0x02, 0x3e, 0x75, 0x28, 0x2c, 0xe5, 0x0d, 0x0b, 0x15, 0xcd, 0xa3, 0x49, 0x72, 0xd3, 0x88, 0xac };


    /** Addresses of the Exchange Channels. **/
    const string CHANNEL_ADDRESSES[] =  {  "2Qn8MsUCkv9S8X8sMvUPkr9Ekqqg5VKzeKJk2ftcFMUiJ5QnWRc",
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
    const unsigned char DEVELOPER_SCRIPT_SIGNATURES[][37] =         {     {0x76, 0xa9, 0x20, 0x16, 0x22, 0xe0, 0xbe, 0xd6, 0xd0, 0x3b, 0x5e, 0x70, 0x93, 0xfa, 0xcf, 0x1e, 0x54, 0x3e, 0xf0, 0x62, 0x9c, 0xb6, 0x7c, 0x3e, 0xb3, 0xbb, 0x30, 0x1b, 0x41, 0x57, 0xa0, 0x60, 0xe2, 0x7b, 0xc1, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xd4, 0xb8, 0x15, 0x5a, 0x18, 0x8e, 0x37, 0xf7, 0xe8, 0xaf, 0x03, 0xe7, 0x26, 0x97, 0xc8, 0x15, 0x12, 0x9c, 0xc4, 0xf0, 0x02, 0x67, 0x23, 0x89, 0x80, 0xa2, 0x8b, 0x37, 0x80, 0xdb, 0x5c, 0x77, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xff, 0xd7, 0x11, 0x70, 0x08, 0x9c, 0x45, 0x50, 0xf2, 0xfa, 0x60, 0x26, 0x9c, 0xb3, 0xec, 0xae, 0xd5, 0x42, 0x82, 0x3a, 0xdb, 0xa5, 0x4f, 0xb0, 0x62, 0xcf, 0x32, 0xd5, 0x3d, 0x88, 0x7d, 0x65, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x25, 0xf9, 0xad, 0x56, 0x7f, 0x94, 0x8e, 0x6c, 0x5d, 0x5c, 0x8c, 0x2f, 0x0f, 0x83, 0x7c, 0x6a, 0x9f, 0xe3, 0x31, 0x14, 0xda, 0xbb, 0xf1, 0xad, 0x2b, 0xd4, 0x91, 0x51, 0xb1, 0x0b, 0x7f, 0xc9, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xad, 0xb9, 0xaf, 0x04, 0x27, 0xe3, 0x46, 0x04, 0x07, 0x88, 0x51, 0xe6, 0xa8, 0x95, 0x69, 0xe6, 0x47, 0x33, 0x2d, 0x6b, 0x08, 0x95, 0xd1, 0x85, 0x58, 0xae, 0xb4, 0xa0, 0x06, 0x24, 0xff, 0x83, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xb5, 0xbc, 0x07, 0xbf, 0x70, 0x16, 0x5e, 0x0b, 0x01, 0xe1, 0xcc, 0x69, 0x13, 0xdc, 0x9f, 0x30, 0xe3, 0x9d, 0x39, 0xb1, 0x0a, 0x11, 0x71, 0xc6, 0x79, 0x33, 0xd4, 0xfe, 0xae, 0x66, 0xb0, 0xe2, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xc6, 0x49, 0xc7, 0x8e, 0xa8, 0x96, 0x8b, 0x77, 0x20, 0xda, 0x20, 0x71, 0x33, 0xc1, 0x95, 0xc4, 0x16, 0x84, 0xd2, 0x25, 0x10, 0xc6, 0x43, 0x58, 0x42, 0x16, 0xd3, 0x2e, 0x68, 0xb6, 0x5b, 0x0f, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xbe, 0xfa, 0xcb, 0xb7, 0x9a, 0x85, 0x47, 0x24, 0xac, 0x03, 0x47, 0xdb, 0xa7, 0x98, 0xa0, 0xcb, 0xe3, 0x86, 0x5f, 0xca, 0x09, 0xfd, 0x83, 0x28, 0x8b, 0x48, 0x1b, 0x16, 0x74, 0xf1, 0x09, 0x89, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x6d, 0x55, 0xa9, 0x72, 0xc3, 0x9d, 0xcb, 0x23, 0xbf, 0xd6, 0x66, 0x7e, 0xc0, 0xdb, 0xf7, 0x4f, 0x19, 0x0c, 0xc7, 0x98, 0x29, 0xd7, 0xdb, 0xaf, 0x41, 0xb1, 0x94, 0x3b, 0x5c, 0x58, 0xcf, 0x53, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xe2, 0xd1, 0xfe, 0xa6, 0x31, 0x97, 0x36, 0xf7, 0x2e, 0x91, 0x6b, 0x44, 0x38, 0x2a, 0xcc, 0x4b, 0xbb, 0xa4, 0x87, 0x4d, 0x54, 0xbb, 0x0d, 0x89, 0x87, 0xbd, 0x1b, 0x2b, 0xe4, 0x74, 0x5b, 0xa3, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xcf, 0xcd, 0x1b, 0x9f, 0xbe, 0x54, 0xda, 0x07, 0x40, 0x30, 0xa1, 0x9b, 0x05, 0x0e, 0x74, 0xd0, 0xa3, 0x7d, 0xe6, 0xa1, 0xcb, 0xe8, 0x0b, 0xe9, 0x7b, 0xa0, 0x21, 0xd8, 0x75, 0x5c, 0x9c, 0xd9, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xcc, 0x83, 0xcc, 0xef, 0xe8, 0xa6, 0xb8, 0x53, 0xea, 0xb7, 0x5f, 0x11, 0xff, 0xdf, 0xff, 0x4e, 0x00, 0xf0, 0x06, 0xd6, 0x89, 0xd4, 0x70, 0x80, 0x47, 0x34, 0x8f, 0x77, 0xf2, 0x53, 0x59, 0xb5, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xde, 0xbe, 0xc5, 0x41, 0x4a, 0x90, 0xd5, 0xa8, 0x01, 0x3e, 0x07, 0x0d, 0xca, 0x94, 0xe2, 0x3b, 0xb1, 0x7b, 0x9c, 0x42, 0x01, 0x0d, 0x21, 0xb4, 0x73, 0x6c, 0x95, 0x46, 0x12, 0x8e, 0x35, 0xc8, 0x88, 0xac} };

    /** Binary Data of Each Channel Address for Signature Verification of Developer Transaction in Accept Block. **/
    const unsigned char AMBASSADOR_SCRIPT_SIGNATURES[][37] =             {    {0x76, 0xa9, 0x20, 0x11, 0xd9, 0x8f, 0xf9, 0x1a, 0x37, 0x34, 0xbf, 0x53, 0x13, 0x1d, 0x23, 0x95, 0xf6, 0x7b, 0x45, 0xe1, 0x04, 0x15, 0xf7, 0x81, 0x7c, 0xd0, 0xb7, 0x3b, 0x24, 0xcd, 0xc9, 0x3c, 0xe3, 0x10, 0x13, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xba, 0xbc, 0x6c, 0x12, 0x21, 0xe1, 0x2f, 0xcd, 0x08, 0xf3, 0xd1, 0x9f, 0x98, 0xba, 0x60, 0x07, 0xe0, 0x09, 0x64, 0xb5, 0x22, 0x2f, 0x0d, 0x69, 0x15, 0x18, 0x43, 0x6d, 0x6b, 0x5b, 0x07, 0x94, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x4c, 0xa4, 0x8e, 0x79, 0x81, 0x38, 0xf1, 0x05, 0xcf, 0x47, 0x4e, 0xf0, 0x74, 0xff, 0x1e, 0x55, 0xfb, 0xdb, 0x35, 0xc8, 0x5c, 0xcc, 0xb9, 0xd3, 0xd1, 0x48, 0xbd, 0x1e, 0x60, 0x2d, 0x23, 0x94, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x17, 0x1b, 0x98, 0x2e, 0x6e, 0x9a, 0x75, 0xea, 0x1a, 0x89, 0xa9, 0xe6, 0xd0, 0x0f, 0x68, 0x5b, 0x27, 0x8e, 0xd0, 0x93, 0xa5, 0x1c, 0xd7, 0xa9, 0x4f, 0x14, 0x9f, 0x8c, 0x87, 0x7d, 0x81, 0x64, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x55, 0x15, 0x07, 0x9a, 0xbb, 0xae, 0x8d, 0x0d, 0xce, 0x65, 0xcf, 0xec, 0x53, 0x99, 0x95, 0x3a, 0x1d, 0x7a, 0x13, 0x40, 0x81, 0x2f, 0x6e, 0xa3, 0x37, 0x7e, 0x9c, 0xec, 0xd8, 0x98, 0x03, 0x2b, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xfb, 0xb6, 0xce, 0x08, 0xbc, 0xdc, 0xe5, 0xb2, 0x62, 0xce, 0x5e, 0x71, 0x93, 0xda, 0x1e, 0xac, 0xb8, 0xed, 0x90, 0x2d, 0xf4, 0x2e, 0x79, 0x6c, 0x6e, 0x16, 0xe5, 0x53, 0xdb, 0x95, 0x4a, 0xa9, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xf0, 0x04, 0x0d, 0x0b, 0x52, 0x94, 0x1d, 0xbf, 0xbd, 0x7c, 0x4b, 0xb2, 0x5b, 0xe9, 0x56, 0x78, 0xd2, 0x2c, 0x7b, 0xa0, 0xe2, 0xe7, 0x20, 0x9e, 0x01, 0x5c, 0xf0, 0x8d, 0x37, 0x50, 0x4a, 0x21, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xdf, 0xcc, 0x8b, 0x40, 0x6c, 0x5d, 0xc3, 0x38, 0x0d, 0x04, 0x77, 0x61, 0xb8, 0x48, 0xf4, 0x2d, 0x7b, 0x25, 0x23, 0xf0, 0xe3, 0x69, 0x63, 0x45, 0x88, 0xb0, 0xb0, 0x9c, 0x35, 0x1a, 0x71, 0x0f, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xab, 0xb4, 0xb9, 0x7c, 0x3a, 0xc4, 0x0e, 0x8e, 0xfa, 0x07, 0xaf, 0x41, 0x75, 0x9c, 0x8d, 0xfc, 0x2f, 0xc7, 0x90, 0x13, 0xcc, 0xb8, 0x78, 0x4f, 0xac, 0x15, 0xd8, 0x21, 0xce, 0x10, 0x7d, 0xfc, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xf6, 0x9a, 0xf7, 0x3f, 0x55, 0x05, 0xce, 0xfa, 0x9e, 0x6d, 0xae, 0x10, 0x5e, 0xae, 0x72, 0xd0, 0x5e, 0x40, 0x0f, 0x5d, 0x46, 0xea, 0x88, 0x33, 0xf5, 0x57, 0x93, 0xa8, 0x4c, 0xf2, 0x46, 0x17, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0xdb, 0x9f, 0x80, 0x65, 0x6e, 0x4c, 0x39, 0xe7, 0x6f, 0xe1, 0xb3, 0x26, 0x09, 0xee, 0x45, 0xb3, 0xe9, 0x0e, 0xe0, 0xe9, 0xcc, 0x75, 0x05, 0x1b, 0xec, 0x92, 0x72, 0xfc, 0x83, 0xdb, 0xd7, 0x3c, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x90, 0xc9, 0x19, 0x58, 0x78, 0xd2, 0xe3, 0xb9, 0x53, 0xaf, 0xf1, 0xdb, 0x5c, 0x3d, 0xad, 0xce, 0x77, 0xc5, 0x83, 0x2b, 0xd8, 0x01, 0x0a, 0xd5, 0x29, 0x1d, 0x54, 0xf2, 0x34, 0xcd, 0xc9, 0x81, 0x88, 0xac},
                                                                {0x76, 0xa9, 0x20, 0x96, 0xd3, 0x5f, 0xb1, 0x67, 0xda, 0xfb, 0x4d, 0x2c, 0x7d, 0xd3, 0x3d, 0x41, 0x34, 0x4f, 0x69, 0xa6, 0x91, 0xda, 0x7c, 0xc1, 0x18, 0xfb, 0x5b, 0x43, 0x3d, 0x25, 0x74, 0x3b, 0x08, 0xf3, 0x41, 0x88, 0xac} };


    CCriticalSection cs_setpwalletRegistered;

    /** List of Registered Transactions to Queue adding to Wallet. **/
    set<Wallet::CWallet*> setpwalletRegistered;

    /** Main Mutex to enable thread Synchronization and Queueing while working on main Data Objects. **/
    CCriticalSection cs_main;

    /** Transaction Holding Structure to wait to be confirmed in the Network. **/
    CTxMemPool mempool;

    /** Trust Key Holding Structure To Verify Trust Keys Seen on Blockchain. **/
    CTrustPool cTrustPool;

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

    /** Alternate Difficulty Adjustments for Regression Tests. **/
    CBigNum bnProofOfWorkLimitRegtest[] = { CBigNum(~uint1024(0) >> 5), CBigNum(100000), CBigNum(~uint1024(0) >> 17) };
    CBigNum bnProofOfWorkStartRegtest[] = { CBigNum(~uint1024(0) >> 7), CBigNum(100000), CBigNum(~uint1024(0) >> 22) };

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

    bool fRepairMode = false;
    int nBestHeightSeen = 0;


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
