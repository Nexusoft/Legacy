/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../main.h"
#include "unifiedtime.h"
#include "../LLP/server.h"

#include "../wallet/db.h"
#include "../util/ui_interface.h"

#include "../LLD/index.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost;


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
		map<uint512, Core::CBlock> MAP_BLOCKS;
        
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
				printf("%%%%%%%%%% Mining LLP: New Block, Clearing Blocks Map.\n");
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
							printf("%%%%%%%%%% Mining LLP: Sent Block %s to Worker.\n\n", NEW_BLOCK.GetHash().ToString().c_str());
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
				printf("%%%%%%%%%% Mining LLP: Rejected Request...No Connections\n"); return false; 
			}

			if(Core::IsInitialBlockDownload() ) 
			{ 
				printf("%%%%%%%%%% Mining LLP: Rejected Request...Downloadning BLockchain\n"); return false; 
			}

			if( pwalletMain->IsLocked()) 
			{ 
				printf("%%%%%%%%%% Mining LLP: Rejected Request...Wallet Locked\n"); return false; 
			}
			
			
			/** Set the Mining Channel this Connection will Serve Blocks for. **/
			if(PACKET.HEADER == SET_CHANNEL)
			{ 
				nChannel = bytes2uint(PACKET.DATA); 
				
				/** Don't allow Mining LLP Requests for Proof of Stake Channel. **/
				if(nChannel == 0)
					return false; 
				
				if(GetArg("-verbose", 0) >= 2)
					printf("%%%%%%%%%% Mining LLP: Channel Set %u\n", nChannel); 
				
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
						printf("%%%%%%%%%% Mining LLP: Invalid Coinbase Tx\n") ;
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
					printf("%%%%%%%%%% Mining LLP: Sent Coinbase Reward of %" PRIu64 "\n", nCoinbaseReward);
				
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
					printf("%%%%%%%%%% Mining LLP: Subscribed to %u Blocks\n", nSubscribed); 
				
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
                NEW_BLOCK.vtx[0].vin[0].scriptSig = (Wallet::CScript() << (1024 * (MAP_BLOCKS.size() + 1)));
                NEW_BLOCK.hashMerkleRoot = NEW_BLOCK.BuildMerkleTree();
                NEW_BLOCK.UpdateTime();
                
                if(GetArg("-verbose", 0) >= 3)
                    printf("%%%%%%%%%%% Mining LLP: Created new Block %s\n", NEW_BLOCK.hashMerkleRoot.ToString().substr(0, 20).c_str());
                
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
						printf("%%%%%%%%%% Mining LLP: Block Not Found %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());
					
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
						printf("%%%%%%%%%% Mining LLP: Unable to Sign block %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());
					
					return true;
				}
				
				
				/* Check the Proof of Work for submitted block. */
				if(!Core::CheckWork(NEW_BLOCK, *pwalletMain, *pMiningKey))
				{
					Packet RESPONSE;
					RESPONSE.HEADER = BLOCK_REJECTED;
					
					this->WritePacket(RESPONSE);
					
					if(GetArg("-verbose", 0) >= 2)
						printf("%%%%%%%%%% Mining LLP: Invalid Work for block %s\n", hashMerkleRoot.ToString().substr(0, 20).c_str());
					
					return true;
				}
				
				
				if(GetArg("-verbose", 0) >= 2)
						printf("%%%%%%%%%% Mining LLP: Found new Block %s\n", NEW_BLOCK->hashMerkleRoot.ToString().substr(0, 10).c_str());
					
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
}
		


namespace Core
{
	LLP::Server<LLP::MiningLLP>* MINING_LLP;
	
	/** Used to Iterate the Coinbase Addresses used For Exchange Channels and Developer Fund. **/
	static unsigned int nCoinbaseCounter = 0;
	static boost::mutex COUNTER_MUTEX;
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
	
	/** Entry point for the Mining LLP. **/
	void StartMiningLLP() { MINING_LLP = new LLP::Server<LLP::MiningLLP>(fTestNet ? TESTNET_MINING_LLP_PORT : NEXUS_MINING_LLP_PORT, GetArg("-mining_threads", 10), true, GetArg("-mining_cscore", 5), GetArg("-mining_rscore", 50), GetArg("-mining_timout", 60)); }
	
	
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

			COUNTER_MUTEX.lock();
				
			/** Reset the Coinbase Transaction Counter. **/
			if(nCoinbaseCounter >= 13)
				nCoinbaseCounter = 0;
						
			/** Set the Proper Addresses for the Coinbase Transaction. **/
			txNew.vout.resize(txNew.vout.size() + 2);
			txNew.vout[txNew.vout.size() - 2].scriptPubKey.SetNexusAddress(fTestNet ? TESTNET_DUMMY_ADDRESS : CHANNEL_ADDRESSES[nCoinbaseCounter]);
			txNew.vout[txNew.vout.size() - 1].scriptPubKey.SetNexusAddress(fTestNet ? TESTNET_DUMMY_ADDRESS : DEVELOPER_ADDRESSES[nCoinbaseCounter]);
			
			/** Set the Proper Coinbase Output Amounts for Recyclers and Developers. **/
			txNew.vout[txNew.vout.size() - 2].nValue = GetCoinbaseReward(pindexPrev, nChannel, 1);
			txNew.vout[txNew.vout.size() - 1].nValue = GetCoinbaseReward(pindexPrev, nChannel, 2);
					
			nCoinbaseCounter++;
			COUNTER_MUTEX.unlock();
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
		uint1024 hash = pblock->GetHash();
		uint1024 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint1024();
		unsigned int nBits;
		
		if(pblock->GetChannel() > 0 && !pblock->VerifyWork())
			return error("Nexus Miner : proof of work not meeting target.");
		
		if(GetArg("-verbose", 0) >= 1){		
			printf("Nexus Miner: new %s block found\n", GetChannelName(pblock->GetChannel()).c_str());
			printf("  hash: %s  \n", hash.ToString().substr(0, 30).c_str());
		}
		
		if(pblock->GetChannel() == 1)
			printf("  prime cluster verified of size %f\n", GetDifficulty(nBits, 1));
		else
			printf("  target: %s\n", hashTarget.ToString().substr(0, 30).c_str());
			
		printf("%s ", DateTimeStrFormat(GetUnifiedTimestamp()).c_str());
		//printf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue).c_str());

		// Found a solution
		{
			LOCK(cs_main);
			if (pblock->hashPrevBlock != hashBestChain && mapBlockIndex[pblock->hashPrevBlock]->GetChannel() == pblock->GetChannel())
				return error("Nexus Miner : generated block is stale");

			// Track how many getdata requests this block gets
			{
				LOCK(wallet.cs_wallet);
				wallet.mapRequestCount[pblock->GetHash()] = 0;
			}

			/** Process the Block to see if it gets Accepted into Blockchain. **/
			if (!ProcessBlock(NULL, pblock))
				return error("Nexus Miner : ProcessBlock, block not accepted\n");
				
			/** Keep the Reserve Key only if it was used in a block. **/
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
