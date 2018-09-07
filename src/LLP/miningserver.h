/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef NEXUS_LLP_MINING_SERVER_H
#define NEXUS_LLP_MINING_SERVER_H

#include "../main.h"
#include "../core/core.h"
#include "../LLP/server.h"


#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>

namespace LLP
{

    /** Class to Create and Manage the Pool Payout Coinbase Tx. **/
    class Coinbase
    {
    public:
        /** The Transaction Outputs to be Serialized to Mining LLP. **/
        std::map<std::string, uint64> vOutputs;


        /** The Value of this current Coinbase Payout. **/
        uint64 nMaxValue, nPoolFee;


        /** Constructor to Class. **/
        Coinbase(std::vector<unsigned char> vData, uint64 nValue){ Deserialize(vData, nValue); }


        /** Deserialize the Coinbase Transaction. **/
        void Deserialize(std::vector<unsigned char> vData, uint64 nValue)
        {
            /** Set the Max Value for this Transaction. **/
            nMaxValue = nValue;

            /** First byte of Serialization Packet is the Number of Records. **/
            unsigned int nSize = vData[0], nIterator = 9;

            /** Bytes 1 - 8 is the Pool Fee for that Round. **/
            nPoolFee  = bytes2uint64(vData, 1);

            /** Loop through every Record. **/
            for(unsigned int nIndex = 0; nIndex < nSize; nIndex++)
            {
                /** De-Serialize the Address String and uint64 nValue. **/
                unsigned int nLength = vData[nIterator];

                std::string strAddress = bytes2string(std::vector<unsigned char>(vData.begin() + nIterator + 1, vData.begin() + nIterator + 1 + nLength));
                uint64 nValue = bytes2uint64(std::vector<unsigned char>(vData.begin() + nIterator + 1 + nLength, vData.begin() + nIterator + 1 + nLength + 8));

                /** Add the Transaction as an Output. **/
                vOutputs[strAddress] = nValue;

                /** Increment the Iterator. **/
                nIterator += (nLength + 9);
            }
        }


        /** Flag to Know if the Coinbase Tx has been built Successfully. **/
        bool IsValid()
        {
            uint64 nCurrentValue = nPoolFee;
            for(std::map<std::string, uint64>::iterator nIterator = vOutputs.begin(); nIterator != vOutputs.end(); nIterator++)
                nCurrentValue += nIterator->second;

            return nCurrentValue == nMaxValue;
        }

        /** Output the Transactions in the Coinbase Container. **/
        void Print()
        {
            printf("\n\n +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\n");
            uint64 nTotal = 0;
            for(std::map<std::string, uint64>::iterator nIterator = vOutputs.begin(); nIterator != vOutputs.end(); nIterator++)
            {
                printf("%s:%f\n", nIterator->first.c_str(), nIterator->second / 1000000.0);
                nTotal += nIterator->second;
            }

            printf("Total Value of Coinbase = %f\n", nTotal / 1000000.0);
            printf("Set Value of Coinbase = %f\n", nMaxValue / 1000000.0);
            printf("PoolFee in Coinbase %f\n", nPoolFee / 1000000.0);
            printf("\n\nIs Complete: %s\n", IsValid() ? "TRUE" : "FALSE");
            printf("\n\n +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\n");
        }

    };


    class MiningLLP : public Connection
    {
        Coinbase* pCoinbaseTx = NULL;
        Core::CBlockIndex* pindexBest = NULL;

        Wallet::CReserveKey* pMiningKey = NULL;
        std::map<uint512, Core::CBlock> MAP_BLOCKS;

        Core::CBlock BASE_BLOCK;
        unsigned int nChannel, nBestHeight;

        /** Subscribed To Display how many Blocks connection Subscribed to. **/
        unsigned int nSubscribed = 0;

        enum
        {
            /** DATA PACKETS **/
            BLOCK_DATA   = 0,
            SUBMIT_BLOCK = 1,
            BLOCK_HEIGHT = 2,
            SET_CHANNEL  = 3,
            BLOCK_REWARD = 4,
            SET_COINBASE = 5,
            GOOD_BLOCK   = 6,
            ORPHAN_BLOCK = 7,


            /** DATA REQUESTS **/
            CHECK_BLOCK  = 64,
            SUBSCRIBE    = 65,


            /** REQUEST PACKETS **/
            GET_BLOCK    = 129,
            GET_HEIGHT   = 130,
            GET_REWARD   = 131,


            /** SERVER COMMANDS **/
            CLEAR_MAP    = 132,
            GET_ROUND    = 133,


            /** RESPONSE PACKETS **/
            BLOCK_ACCEPTED       = 200,
            BLOCK_REJECTED       = 201,


            /** VALIDATION RESPONSES **/
            COINBASE_SET  = 202,
            COINBASE_FAIL = 203,

            /** ROUND VALIDATIONS. **/
            NEW_ROUND     = 204,
            OLD_ROUND     = 205,

            /** GENERIC **/
            PING     = 253,
            CLOSE    = 254
        };

    public:
        MiningLLP() : Connection(){ pMiningKey = new Wallet::CReserveKey(pwalletMain); nChannel = 0; nBestHeight = 0; }
        MiningLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : Connection( SOCKET_IN, DDOS_IN ) { pMiningKey = new Wallet::CReserveKey(pwalletMain); nChannel = 0; nBestHeight = 0; }

        ~MiningLLP()
        {
            delete pMiningKey;
            delete pCoinbaseTx;
        }

        inline void ClearMap()
        {
            pCoinbaseTx = NULL;

            MAP_BLOCKS.clear();

            if(GetArg("-verbose", 0) >= 2)
                printf("***** Mining LLP: New Block, Clearing Blocks Map.\n");
        }

        void Event(unsigned char EVENT, unsigned int LENGTH = 0)
        {
            /** Handle any DDOS Packet Filters. **/
            if(EVENT == EVENT_HEADER)
            {
                if(fDDOS)
                {
                    Packet PACKET   = this->INCOMING;
                    if(PACKET.HEADER == BLOCK_DATA)
                        DDOS->Ban();

                    if(PACKET.HEADER == SUBMIT_BLOCK && PACKET.LENGTH > 72)
                        DDOS->Ban();

                    if(PACKET.HEADER == BLOCK_HEIGHT)
                        DDOS->Ban();

                    if(PACKET.HEADER == SET_CHANNEL && PACKET.LENGTH > 4)
                        DDOS->Ban();

                    if(PACKET.HEADER == BLOCK_REWARD)
                        DDOS->Ban();

                    if(PACKET.HEADER == SET_COINBASE && PACKET.LENGTH > 20 * 1024)
                        DDOS->Ban();

                    if(PACKET.HEADER == GOOD_BLOCK)
                        DDOS->Ban();

                    if(PACKET.HEADER == ORPHAN_BLOCK)
                        DDOS->Ban();

                    if(PACKET.HEADER == CHECK_BLOCK && PACKET.LENGTH > 128)
                        DDOS->Ban();

                    if(PACKET.HEADER == SUBSCRIBE && PACKET.LENGTH > 4)
                        DDOS->Ban();

                    if(PACKET.HEADER == BLOCK_ACCEPTED)
                        DDOS->Ban();

                    if(PACKET.HEADER == BLOCK_REJECTED)
                        DDOS->Ban();

                    if(PACKET.HEADER == COINBASE_SET)
                        DDOS->Ban();

                    if(PACKET.HEADER == COINBASE_FAIL)
                        DDOS->Ban();

                    if(PACKET.HEADER == NEW_ROUND)
                        DDOS->Ban();

                    if(PACKET.HEADER == OLD_ROUND)
                        DDOS->Ban();

                }
            }


            /** Handle for a Packet Data Read. **/
            if(EVENT == EVENT_PACKET)
                return;


            /** On Generic Event, Broadcast new block if flagged. **/
            if(EVENT == EVENT_GENERIC)
            {
                /** Skip Generic Event if not Subscribed to Work. **/
                if(nSubscribed == 0)
                    return;

                /** Check the Round Automatically on Subscribed Worker. **/
                if(pindexBest == NULL || !pindexBest || pindexBest->GetBlockHash() != Core::pindexBest->GetBlockHash())
                {
                    pindexBest = Core::pindexBest;
                    ClearMap();

                    /** Construct a response packet by serializing the Block. **/
                    Packet RESPONSE;
                    RESPONSE.HEADER = NEW_ROUND;
                    this->WritePacket(RESPONSE);

                    /* Create a new base block. */
                    BASE_BLOCK = Core::CreateNewBlock(*pMiningKey, pwalletMain, nChannel, MAP_BLOCKS.size() + 1, pCoinbaseTx);
                    if(BASE_BLOCK.IsNull())
                        return;

                    /** Create all Blocks Worker Subscribed to. **/
                    for(int nBlock = 0; nBlock < nSubscribed; nBlock++) {

                        /* Create new block from the base block. */
                        Core::CBlock NEW_BLOCK = BASE_BLOCK;
                        NEW_BLOCK.vtx[0].vin[0].scriptSig = (Wallet::CScript() << MAP_BLOCKS.size() * 513513512151);
                        NEW_BLOCK.BuildMerkleTree();
                        NEW_BLOCK.UpdateTime();

                        /** Handle the Block Key as Merkle Root for Block Submission. **/
                        MAP_BLOCKS[NEW_BLOCK.hashMerkleRoot] = NEW_BLOCK;

                        /** Construct a response packet by serializing the Block. **/
                        Packet RESPONSE;
                        RESPONSE.HEADER = BLOCK_DATA;
                        RESPONSE.DATA   = SerializeBlock(NEW_BLOCK);
                        RESPONSE.LENGTH = RESPONSE.DATA.size();

                        this->WritePacket(RESPONSE);

                        if(GetArg("-verbose", 0) >= 2)
                            printf("***** Mining LLP: Sent Block %s to Worker.\n\n", NEW_BLOCK.GetHash().ToString().c_str());
                    }
                }

                return;
            }

            /** On Connect Event, Assign the Proper Daemon Handle. **/
            if(EVENT == EVENT_CONNECT)
                return;

            /** On Disconnect Event, Reduce the Connection Count for Daemon **/
            if(EVENT == EVENT_DISCONNECT)
                return;

        }

        /** This function is necessary for a template LLP server. It handles your
            custom messaging system, and how to interpret it from raw packets. **/
        bool ProcessPacket()
        {
            Packet PACKET   = this->INCOMING;


            /** If There are no Active nodes, or it is Initial Block Download:
                Send a failed response to the miners, unless this is a regression test. **/
            if(!GetBoolArg("-regtest",false) && Net::vNodes.size() == 0 )
            {
                printf("***** Mining LLP: Rejected Request...No Connections\n"); return false;
            }

            if(Core::IsInitialBlockDownload() )
            {
                printf("***** Mining LLP: Rejected Request...Downloading BLockchain\n"); return false;
            }

            if( pwalletMain->IsLocked())
            {
                printf("***** Mining LLP: Rejected Request...Wallet Locked\n"); return false;
            }


            /** Set the Mining Channel this Connection will Serve Blocks for. **/
            if(PACKET.HEADER == SET_CHANNEL)
            {
                nChannel = bytes2uint(PACKET.DATA);

                /** Don't allow Mining LLP Requests for Proof of Stake Channel. **/
                if(nChannel == 0)
                    return false;

                if(GetArg("-verbose", 0) >= 2)
                    printf("***** Mining LLP: Channel Set %u\n", nChannel);

                return true;
            }


            /** Return a Ping if Requested. **/
            if(PACKET.HEADER == PING){ Packet PACKET; PACKET.HEADER = PING; this->WritePacket(PACKET); return true; }


            /** Set the Mining Channel this Connection will Serve Blocks for. **/
            if(PACKET.HEADER == SET_COINBASE)
            {
                Coinbase* pCoinbase = new Coinbase(PACKET.DATA, GetCoinbaseReward(Core::pindexBest, nChannel, 0));

                if(!pCoinbase->IsValid())
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = COINBASE_FAIL;
                    this->WritePacket(RESPONSE);

                    if(GetArg("-verbose", 0) >= 2)
                        printf("***** Mining LLP: Invalid Coinbase Tx\n") ;
                }
                else
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = COINBASE_SET;
                    this->WritePacket(RESPONSE);

                    ClearMap();
                    pCoinbaseTx = pCoinbase;

                    BASE_BLOCK = Core::CreateNewBlock(*pMiningKey, pwalletMain, nChannel, MAP_BLOCKS.size() + 1, pCoinbaseTx);
                }

                return true;
            }


            /** Clear the Block Map if Requested by Client. **/
            if(PACKET.HEADER == CLEAR_MAP)
            {
                ClearMap();

                return true;
            }


            /** Get Height Process:
                Responds to the Miner with the Height of Current Best Block.
                Used to poll whether a new block needs to be created. **/
            if(PACKET.HEADER == GET_HEIGHT)
            {
                Packet RESPONSE;
                RESPONSE.HEADER = BLOCK_HEIGHT;
                RESPONSE.LENGTH = 4;
                RESPONSE.DATA   = uint2bytes(Core::nBestHeight + 1);

                this->WritePacket(RESPONSE);

                /** Clear the Maps if Requested Height that is a New Best Block. **/
                if(Core::nBestHeight > nBestHeight)
                {
                    ClearMap();
                    nBestHeight = Core::nBestHeight;

                    BASE_BLOCK = Core::CreateNewBlock(*pMiningKey, pwalletMain, nChannel, 1, pCoinbaseTx);
                }

                return true;
            }

            if(PACKET.HEADER == GET_ROUND)
            {
                if(pindexBest == NULL)
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = NEW_ROUND;
                    this->WritePacket(RESPONSE);

                    pindexBest = Core::pindexBest;

                    return true;
                }

                if(!pindexBest)
                    return true;

                Packet RESPONSE;
                RESPONSE.HEADER = OLD_ROUND;

                if(pindexBest->GetBlockHash() != Core::pindexBest->GetBlockHash())
                {
                    pindexBest = Core::pindexBest;
                    RESPONSE.HEADER = NEW_ROUND;

                    ClearMap();

                    BASE_BLOCK = Core::CreateNewBlock(*pMiningKey, pwalletMain, nChannel, 1, pCoinbaseTx);
                }

                this->WritePacket(RESPONSE);

                return true;
            }


            /** Get Reward Process:
                Responds with the Current Block Reward. **/
            if(PACKET.HEADER == GET_REWARD)
            {
                uint64 nCoinbaseReward = GetCoinbaseReward(Core::pindexBest, nChannel, 0);

                Packet RESPONSE;
                RESPONSE.HEADER = BLOCK_REWARD;
                RESPONSE.LENGTH = 8;
                RESPONSE.DATA = uint2bytes64(nCoinbaseReward);
                this->WritePacket(RESPONSE);

                if(GetArg("-verbose", 0) >= 2)
                    printf("***** Mining LLP: Sent Coinbase Reward of %" PRIu64 "\n", nCoinbaseReward);

                return true;
            }

            /** Allow Block Subscriptions. **/
            if(PACKET.HEADER == SUBSCRIBE)
            {
                nSubscribed = bytes2uint(PACKET.DATA);

                /** Don't allow Mining LLP Requests for Proof of Stake Channel. **/
                if(nSubscribed == 0)
                    return false;

                if(GetArg("-verbose", 0) >= 2)
                    printf("***** Mining LLP: Subscribed to %u Blocks\n", nSubscribed);

                return true;
            }

            /** New block Process:
                Keeps a map of requested blocks for this connection.
                Clears map once new block is submitted successfully. **/
            if(PACKET.HEADER == GET_BLOCK)
            {
                /* Reject request if there is no base block created already. */
                if(BASE_BLOCK.IsNull())
                    BASE_BLOCK = Core::CreateNewBlock(*pMiningKey, pwalletMain, nChannel, 1, pCoinbaseTx);


                /* Create new block from base block by changing the input script to search from new merkle root. */
                Core::CBlock NEW_BLOCK = BASE_BLOCK;

                /* Checks for prime min origins. */
                for(int i = MAP_BLOCKS.size(); ; i++)
                {
                    /* Create a vin script sig that will change merkle root */
                    NEW_BLOCK.vtx[0].vin[0].scriptSig = (Wallet::CScript() << (1024 * (MAP_BLOCKS.size() + 1)) << (i * 510));
                    NEW_BLOCK.hashMerkleRoot = NEW_BLOCK.BuildMerkleTree();
                    NEW_BLOCK.UpdateTime();

                    /* Skip if not prime channel or version < 5 */
                    if(nChannel != 1 || NEW_BLOCK.nVersion < 5)
                        break;

                    /* Don't deliver blocks that are below the minimum prime origins. */
                    if(NEW_BLOCK.ProofHash() > Core::bnPrimeMinOrigins.getuint1024())
                        break;
                }

                if(GetArg("-verbose", 0) >= 3)
                    printf("***** Mining LLP: Created new Block %s\n", NEW_BLOCK.hashMerkleRoot.ToString().substr(0, 20).c_str());

                /* Store the new block in the memory map of recent blocks being worked on. */
                MAP_BLOCKS[NEW_BLOCK.hashMerkleRoot] = NEW_BLOCK;


                /* Construct a response packet by serializing the Block. */
                Packet RESPONSE;
                RESPONSE.HEADER = BLOCK_DATA;
                RESPONSE.DATA   = SerializeBlock(NEW_BLOCK);
                RESPONSE.LENGTH = RESPONSE.DATA.size();

                this->WritePacket(RESPONSE);

                return true;
            }


            /** Submit Block Process:
                Accepts a new block Merkle and nNonce for submit.
                This is to correlate where in memory the actual
                block is from MAP_BLOCKS. **/
            if(PACKET.HEADER == SUBMIT_BLOCK)
            {
                uint512 hashMerkleRoot;
                hashMerkleRoot.SetBytes(std::vector<unsigned char>(PACKET.DATA.begin(), PACKET.DATA.end() - 8));


                /* Check that the block exists. */
                if(!MAP_BLOCKS.count(hashMerkleRoot))
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = BLOCK_REJECTED;

                    this->WritePacket(RESPONSE);

                    if(GetArg("-verbose", 0) >= 2)
                        printf("***** Mining LLP: Block Not Found %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());

                    return true;
                }


                /* Create the pointer on the heap. */
                Core::CBlock* NEW_BLOCK = &MAP_BLOCKS[hashMerkleRoot];
                NEW_BLOCK->nNonce = bytes2uint64(std::vector<unsigned char>(PACKET.DATA.end() - 8, PACKET.DATA.end()));
                NEW_BLOCK->UpdateTime();
                NEW_BLOCK->print();


                /* Sign the submitted block. */
                if(!NEW_BLOCK->SignBlock(*pwalletMain))
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = BLOCK_REJECTED;

                    this->WritePacket(RESPONSE);

                    if(GetArg("-verbose", 0) >= 2)
                        printf("***** Mining LLP: Unable to Sign block %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());

                    return true;
                }


                /* Check the Proof of Work for submitted block. */
                if(!Core::CheckWork(NEW_BLOCK, *pwalletMain, *pMiningKey))
                {
                    Packet RESPONSE;
                    RESPONSE.HEADER = BLOCK_REJECTED;

                    this->WritePacket(RESPONSE);

                    if(GetArg("-verbose", 0) >= 2)
                        printf("***** Mining LLP: Invalid Work for block %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());

                    return true;
                }


                if(GetArg("-verbose", 0) >= 2)
                        printf("***** Mining LLP: Found new Block %s\n", NEW_BLOCK->hashMerkleRoot.ToString().substr(0, 10).c_str());

                Packet RESPONSE;
                RESPONSE.HEADER = BLOCK_ACCEPTED;

                ClearMap();

                this->WritePacket(RESPONSE);

                return true;
            }


            /** Check Block Command: Allows Client to Check if a Block is part of the Main Chain. **/
            if(PACKET.HEADER == CHECK_BLOCK)
            {
                uint1024 hashBlock;
                hashBlock.SetBytes(PACKET.DATA);

                Packet RESPONSE;
                RESPONSE.LENGTH = PACKET.LENGTH;
                RESPONSE.DATA   = PACKET.DATA;

                if(Core::mapBlockIndex.count(hashBlock) && (Core::mapBlockIndex[hashBlock]->nHeight == Core::nBestHeight || Core::mapBlockIndex[hashBlock]->pnext))
                    RESPONSE.HEADER = GOOD_BLOCK;
                else
                    RESPONSE.HEADER = ORPHAN_BLOCK;

                this->WritePacket(RESPONSE);

                return true;
            }

            return false;
        }

    private:

        /** Convert the Header of a Block into a Byte Stream for Reading and Writing Across Sockets. **/
        std::vector<unsigned char> SerializeBlock(Core::CBlock BLOCK)
        {
            std::vector<unsigned char> VERSION  = uint2bytes(BLOCK.nVersion);
            std::vector<unsigned char> PREVIOUS = BLOCK.hashPrevBlock.GetBytes();
            std::vector<unsigned char> MERKLE   = BLOCK.hashMerkleRoot.GetBytes();
            std::vector<unsigned char> CHANNEL  = uint2bytes(BLOCK.nChannel);
            std::vector<unsigned char> HEIGHT   = uint2bytes(BLOCK.nHeight);
            std::vector<unsigned char> BITS     = uint2bytes(BLOCK.nBits);
            std::vector<unsigned char> NONCE    = uint2bytes64(BLOCK.nNonce);

            std::vector<unsigned char> DATA;
            DATA.insert(DATA.end(), VERSION.begin(),   VERSION.end());
            DATA.insert(DATA.end(), PREVIOUS.begin(), PREVIOUS.end());
            DATA.insert(DATA.end(), MERKLE.begin(),     MERKLE.end());
            DATA.insert(DATA.end(), CHANNEL.begin(),   CHANNEL.end());
            DATA.insert(DATA.end(), HEIGHT.begin(),     HEIGHT.end());
            DATA.insert(DATA.end(), BITS.begin(),         BITS.end());
            DATA.insert(DATA.end(), NONCE.begin(),       NONCE.end());

            return DATA;
        }
    };

    extern Server<MiningLLP>* MINING_LLP;
}


#endif
