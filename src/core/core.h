/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_H
#define NEXUS_CORE_H

#include "../LLU/bignum.h"
#include "../net/net.h"
#include "../wallet/script.h"

#ifdef WIN32
#include <io.h> /* for _commit */
#define __STDC_FORMAT_MACROS 1
#endif

#ifdef USE_UPNP
static const int fHaveUPnP = true;
#else
static const int fHaveUPnP = false;
#endif

#include <list>
#include <inttypes.h>

class CDataStream;

/** Net Namespace: Lowest Level Below Core Namespace. Handles all the raw
    Data through the network sockets in Nexus Network, and organizes
    into usable objects that are fed into the Core Namespace.	**/
namespace Net { class CNode; }


/** Wallet Namespace: Outer layer on top of Core Namespace.
    Handles all the wallet functionality such as Private Keys,
    Nexus Addresses, Transaction Database, and Transaction Creation. **/
namespace Wallet
{
	class CWallet;
	class CKeyItem;
	class CKey;
	class CKeyStore;
	class CReserveKey;
	class CTxDB;
	class CScript;
	class CWalletTx;
}

/** Holds the Servers and Clients for the LLP Protocols. The two LLP's in Nexus are the Core LLP and the Mining LLP. **/
namespace LLP { class Coinbase; }

/** Index Database of the LLD. **/
namespace LLD { class CIndexDB; }


/** Core Namespace: This namespace contains all the core level functions. 
    This is the middle layer in which the other two namespaces will communicate with.
    The functions handled within the core are block / transaction processing, checkpointing, 
	difficulty, time release, mining, node messaging, and wallet dispatching.	**/
namespace Core
{
	/** Core Namespace Class Forward Declarations **/
	class CBlock;
	class CBlockIndex;
	class CBlockLocator;
	class CTransaction;
	class CTrustKey;
	class CTxIndex;
	class COutPoint;
	
	
	/**************************************** CORE EXTERNAL VARIABLES ****************************************/
	
	/** Constant Global Externals. Use to share global settings across entire scope. **/
	extern const unsigned int MAX_BLOCK_SIZE;
	extern const unsigned int MAX_BLOCK_SIZE_GEN;
	extern const unsigned int MAX_BLOCK_SIGOPS;
	extern const unsigned int MAX_ORPHAN_TRANSACTIONS;
	extern const unsigned int NEXUS_NETWORK_TIMELOCK;
	extern const unsigned int NEXUS_TESTNET_TIMELOCK;
	
	extern const unsigned int TESTNET_VERSION2_TIMELOCK;
	extern const unsigned int NETWORK_VERSION2_TIMELOCK;
	
	extern const unsigned int TESTNET_VERSION3_TIMELOCK;
	extern const unsigned int NETWORK_VERSION3_TIMELOCK;
	
	extern const unsigned int TESTNET_VERSION4_TIMELOCK;
	extern const unsigned int NETWORK_VERSION4_TIMELOCK;
	
	extern const unsigned int CHANNEL_NETWORK_TIMELOCK[];
	extern const unsigned int CHANNEL_TESTNET_TIMELOCK[];
	
	extern const int64 MIN_TX_FEE;
	extern const int64 MIN_RELAY_TX_FEE;
	extern const int64 MAX_TXOUT_AMOUNT;
	extern const int64 MIN_TXOUT_AMOUNT;
	extern const int COINBASE_MATURITY;
	extern const int MODIFIER_INTERVAL_RATIO;
	extern const int LOCKTIME_THRESHOLD;
	extern const int STAKE_TARGET_SPACING;
	
	extern int TRUST_KEY_EXPIRE;
	extern int TRUST_KEY_MIN_INTERVAL;
	
	extern const uint64 MAX_STAKE_WEIGHT;
	extern const uint1024 hashGenesisBlockOfficial;
	extern const uint1024 hashGenesisBlockTestNet;
	extern const std::string strMessageMagic;
	
	
	/** The current block versions to reject any attempts to manipulate Version. **/
	extern const unsigned int NETWORK_BLOCK_CURRENT_VERSION;
	extern const unsigned int TESTNET_BLOCK_CURRENT_VERSION;
	
	
	/** Vectors to hold the addresses for Nexus Channels and Developers. **/
	extern const std::string TESTNET_DUMMY_ADDRESS;
	extern const std::string CHANNEL_ADDRESSES[];
	extern const std::string DEVELOPER_ADDRESSES[];
	
	extern const unsigned char DEVELOPER_SIGNATURES[][37];
	extern const unsigned char CHANNEL_SIGNATURES[][37];
	extern const unsigned char TESTNET_DUMMY_SIGNATURE[];
	
	
	/** BigNum Global Externals **/
	extern CBigNum bnProofOfWorkLimit[];
	extern CBigNum bnProofOfWorkStart[];
	
	/** Chain Trust **/
	extern uint64 nBestChainTrust;
	
	/** Standard Library Global Externals **/
	extern std::map<uint1024, CBlock*> mapOrphanBlocks;
	
	/** Map to keep track of the addresses and their corresponding Transactions. **/
	extern std::map<uint256, uint64> mapAddressTransactions;
	
	/** The "Block Chain" or index of the chain linking each block to its previous block. **/
	extern std::map<uint1024, CBlockIndex*> mapBlockIndex;
	
	extern std::map<uint1024, uint1024> mapProofOfStake;
	extern std::map<uint512, CDataStream*> mapOrphanTransactions;
	extern std::map<uint512, std::map<uint512, CDataStream*> > mapOrphanTransactionsByPrev;
	extern std::set<Wallet::CWallet*> setpwalletRegistered;
	extern std::multimap<uint1024, CBlock*> mapOrphanBlocksByPrev;
	extern std::string strMintMessage;
	extern std::string strMintWarning;
	
	
	/** Numerical Global Variables Externals **/
	extern int nCoinbaseMaturity;
	extern int64 nTransactionFee;
	extern int64 nTimeBestReceived;
	extern uint64 nLastBlockTx;
	extern uint64 nLastBlockSize;
	extern uint1024 hashGenesisBlock;
	extern uint1024 hashBestChain;
	extern unsigned int nCurrentBlockFile;
	extern unsigned int nBestHeight;
	
	
	/** Reporting Constant for Current Weight. **/
	extern double dTrustWeight;
	extern double dBlockWeight;
	extern double dInterestRate;
	
	/** Block Accounting Externals **/
	extern CBlockIndex* pindexGenesisBlock;
	extern CBlockIndex* pindexBest;

	
	/** Critical Sections for Locks **/
	extern CCriticalSection cs_main;
	extern CCriticalSection cs_setpwalletRegistered;
	
	
	/** Filter to gain average block count among peers **/
	extern CMajority<int> cPeerBlockCounts;
	
	
	/** Small function to ensure the global supply remains within reasonable bounds **/
	inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_TXOUT_AMOUNT); }
	
		
		
		
		
	
	
	/**************************************** CORE FUNCTION REFERENCES ****************************************/
	
	/**  BLOCK.CPP **/
	uint1024 GetOrphanRoot(const CBlock* pblock);
	uint1024 WantedByOrphan(const CBlock* pblockOrphan);
	int64 GetProofOfWorkReward(unsigned int nBits);
	int64 GetProofOfStakeReward(int64 nCoinAge);
	const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
	const CBlockIndex* GetLastChannelIndex(const CBlockIndex* pindex, int nChannel);
	int GetNumBlocksOfPeers();
	bool IsInitialBlockDownload();
	bool ProcessBlock(Net::CNode* pfrom, CBlock* pblock);
	bool CheckDiskSpace(uint64 nAdditionalBytes = 0);
	FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode);
	FILE* AppendBlockFile(unsigned int& nFileRet);
	bool LoadBlockIndex(bool fAllowNew = true);
	bool CheckBlockIndex(uint1024 hashBlock);
	
	
	/** DISPATCH.CPP **/
	void RegisterWallet(Wallet::CWallet* pwalletIn);
	void UnregisterWallet(Wallet::CWallet* pwalletIn);
	void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false, bool fConnect = true);
	bool IsFromMe(CTransaction& tx);
	bool GetTransaction(const uint512& hashTx, Wallet::CWalletTx& wtx);
	void EraseFromWallets(uint512 hash);
	void SetBestChain(const CBlockLocator& loc);
	void UpdatedTransaction(const uint512& hashTx);
	void Inventory(const uint1024& hash);
	void ResendWalletTransactions();
	void PrintWallets(const CBlock& block);
	
	
	/** CHECKPOINTS **/
	uint1024 GetLastCheckpoint();
	bool IsNewTimespan(CBlockIndex* pindex);
	bool IsDescendant(CBlockIndex* pindex);
	bool HardenCheckpoint(CBlockIndex* pcheckpoint, bool fInit = false);
	
	
	/** DIFFICULTY.CPP **/
	double GetDifficulty(unsigned int nBits, int nChannel);
	unsigned int ComputeMinWork(const CBlockIndex* pcheckpoint, int64 nTime, int nChannel);
	unsigned int GetNextTargetRequired(const CBlockIndex* pindex, int nChannel, bool output = false);
	unsigned int RetargetPOS(const CBlockIndex* pindex, bool output);
	unsigned int RetargetCPU(const CBlockIndex* pindex, bool output);
	unsigned int RetargetGPU(const CBlockIndex* pindex, bool output);
	
	
	/** MESSAGE.CPP **/
	std::string GetWarnings(std::string strFor);
	bool AlreadyHave(LLD::CIndexDB& indexdb, const Net::CInv& inv);
	bool ProcessMessage(Net::CNode* pfrom, std::string strCommand, CDataStream& vRecv);
	bool ProcessMessages(Net::CNode* pfrom);
	bool SendMessages(Net::CNode* pto, bool fSendTrickle);
	
	
	/** MINING.CPP **/
	void StartMiningLLP();
	void StartStaking(Wallet::CWallet *pwallet);
	CBlock* CreateNewBlock(Wallet::CReserveKey& reservekey, Wallet::CWallet* pwallet, unsigned int nChannel, unsigned int nID = 1, LLP::Coinbase* pCoinbase = NULL);
	void AddTransactions(std::vector<CTransaction>& vtx, CBlockIndex* pindexPrev);
	
	bool CheckWork(CBlock* pblock, Wallet::CWallet& wallet, Wallet::CReserveKey& reservekey);
	void StakeMinter(void* parg);
	std::string GetChannelName(int nChannel);
	
	
	/** RELEASE.CPP **/
	int64 GetSubsidy(int nMinutes, int nType);
	int64 CompoundSubsidy(int nMinutes, int nInterval);
	int64 CompoundSubsidy(int nMinutes);
	int64 GetMoneySupply(CBlockIndex* pindex);
	int64 GetChainAge(int64 nTime);
	int64 GetFractionalSubsidy(int nMinutes, int nType, double nFraction);
	int64 GetCoinbaseReward(const CBlockIndex* pindex, int nChannel, int nType);
	int64 ReleaseRewards(int nTimespan, int nStart, int nType);
	int64 GetReleasedReserve(const CBlockIndex* pindex, int nChannel, int nType);
	bool  ReleaseAvailable(const CBlockIndex* pindex, int nChannel);

	
	/** PRIME.CPP **/
	unsigned int SetBits(double nDiff);
	double GetPrimeDifficulty(CBigNum prime, int checks);
	unsigned int GetPrimeBits(CBigNum prime);
	unsigned int GetFractionalDifficulty(CBigNum composite);
	bool PrimeCheck(CBigNum test, int checks);
	CBigNum FermatTest(CBigNum n, CBigNum a);
	bool Miller_Rabin(CBigNum n, int checks);


	/** TRANSACTION.CPP **/
	bool AddOrphanTx(const CDataStream& vMsg);
	void EraseOrphanTx(uint512 hash);
	unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans);
	bool GetTransaction(const uint512 &hash, CTransaction &tx, uint1024 &hashBlock);


	
	/**************************************** CORE CLASS DEFINITIONS ****************************************/

	/** Position on disk for a particular transaction. */
	class CDiskTxPos
	{
	public:
		unsigned int nFile;
		unsigned int nBlockPos;
		unsigned int nTxPos;

		CDiskTxPos()
		{
			SetNull();
		}

		CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
		{
			nFile = nFileIn;
			nBlockPos = nBlockPosIn;
			nTxPos = nTxPosIn;
		}

		IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
		void SetNull() { nFile = -1; nBlockPos = 0; nTxPos = 0; }
		bool IsNull() const { return (nFile == -1); }

		friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
		{
			return (a.nFile     == b.nFile &&
					a.nBlockPos == b.nBlockPos &&
					a.nTxPos    == b.nTxPos);
		}

		friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
		{
			return !(a == b);
		}

		std::string ToString() const
		{
			if (IsNull())
				return "null";
			else
				return strprintf("(nFile=%d, nBlockPos=%d, nTxPos=%d)", nFile, nBlockPos, nTxPos);
		}

		void print() const
		{
			printf("%s", ToString().c_str());
		}
	};

	
	/** An inpoint - a combination of a transaction and an index n into its vin */
	class CInPoint
	{
	public:
		CTransaction* ptx;
		unsigned int n;

		CInPoint() { SetNull(); }
		CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
		void SetNull() { ptx = NULL; n = -1; }
		bool IsNull() const { return (ptx == NULL && n == -1); }
	};


	/** An outpoint - a combination of a transaction hash and an index n into its vout */
	class COutPoint
	{
	public:
		uint512 hash;
		unsigned int n;

		COutPoint() { SetNull(); }
		COutPoint(uint512 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
		IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
		void SetNull() { hash = 0; n = -1; }
		bool IsNull() const { return (hash == 0 && n == -1); }

		friend bool operator<(const COutPoint& a, const COutPoint& b)
		{
			return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
		}

		friend bool operator==(const COutPoint& a, const COutPoint& b)
		{
			return (a.hash == b.hash && a.n == b.n);
		}

		friend bool operator!=(const COutPoint& a, const COutPoint& b)
		{
			return !(a == b);
		}

		std::string ToString() const
		{
			return strprintf("COutPoint(%s, %d)", hash.ToString().substr(0,10).c_str(), n);
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};



	/** An input of a transaction.  It contains the location of the previous
	 * transaction's output that it claims and a signature that matches the
	 * output's public key.
	 */
	class CTxIn
	{
	public:
		COutPoint prevout;
		Wallet::CScript scriptSig;
		unsigned int nSequence;

		CTxIn()
		{
			nSequence = std::numeric_limits<unsigned int>::max();
		}

		explicit CTxIn(COutPoint prevoutIn, Wallet::CScript scriptSigIn=Wallet::CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
		{
			prevout = prevoutIn;
			scriptSig = scriptSigIn;
			nSequence = nSequenceIn;
		}

		CTxIn(uint512 hashPrevTx, unsigned int nOut, Wallet::CScript scriptSigIn=Wallet::CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
		{
			prevout = COutPoint(hashPrevTx, nOut);
			scriptSig = scriptSigIn;
			nSequence = nSequenceIn;
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(prevout);
			READWRITE(scriptSig);
			READWRITE(nSequence);
		)

		bool IsFinal() const
		{
			return (nSequence == std::numeric_limits<unsigned int>::max());
		}
		
		bool IsStakeSig() const
		{
			if( scriptSig.size() != 8)
				return false;
				
			if( scriptSig[0] != 1 || scriptSig[1] != 2 || scriptSig[2] != 3 || scriptSig[3] != 5 || 
				scriptSig[4] != 8 || scriptSig[5] != 13 || scriptSig[6] != 21 || scriptSig[7] != 34)
				return false;
			
			return true;
		}

		friend bool operator==(const CTxIn& a, const CTxIn& b)
		{
			return (a.prevout   == b.prevout &&
					a.scriptSig == b.scriptSig &&
					a.nSequence == b.nSequence);
		}

		friend bool operator!=(const CTxIn& a, const CTxIn& b)
		{
			return !(a == b);
		}

		std::string ToStringShort() const
		{
			return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
		}

		std::string ToString() const
		{
			std::string str;
			str += "CTxIn(";
			str += prevout.ToString();
			if (prevout.IsNull())
			{	
				if(IsStakeSig())
					str += strprintf(", genesis %s", HexStr(scriptSig).c_str());
				else
					str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
			}
			else if(IsStakeSig())
				str += strprintf(", trust %s", HexStr(scriptSig).c_str());
			else
				str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
			if (nSequence != std::numeric_limits<unsigned int>::max())
				str += strprintf(", nSequence=%u", nSequence);
			str += ")";
			return str;
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};



	/** An output of a transaction.  It contains the public key that the next input
	 * must be able to sign with to claim it.
	 */
	class CTxOut
	{
	public:
		int64 nValue;
		Wallet::CScript scriptPubKey;

		CTxOut()
		{
			SetNull();
		}

		CTxOut(int64 nValueIn, Wallet::CScript scriptPubKeyIn)
		{
			nValue = nValueIn;
			scriptPubKey = scriptPubKeyIn;
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(nValue);
			READWRITE(scriptPubKey);
		)

		void SetNull()
		{
			nValue = -1;
			scriptPubKey.clear();
		}

		bool IsNull()
		{
			return (nValue == -1);
		}

		void SetEmpty()
		{
			nValue = 0;
			scriptPubKey.clear();
		}

		bool IsEmpty() const
		{
			return (nValue == 0 && scriptPubKey.empty());
		}

		uint512 GetHash() const
		{
			return SerializeHash(*this);
		}

		friend bool operator==(const CTxOut& a, const CTxOut& b)
		{
			return (a.nValue       == b.nValue &&
					a.scriptPubKey == b.scriptPubKey);
		}

		friend bool operator!=(const CTxOut& a, const CTxOut& b)
		{
			return !(a == b);
		}

		std::string ToStringShort() const
		{
			return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString(true).c_str());
		}

		std::string ToString() const
		{
			if (IsEmpty()) return "CTxOut(empty)";
			if (scriptPubKey.size() < 6)
				return "CTxOut(error)";
			return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};




	enum GetMinFee_mode
	{
		GMF_BLOCK,
		GMF_RELAY,
		GMF_SEND,
	};

	typedef std::map<uint512, std::pair<CTxIndex, CTransaction> > MapPrevTx;
	

	
	/** Class to Store the Trust Key and It's Interest Rate. **/
	class CTrustKey
	{
	public:
	
		/** The Public Key associated with Trust Key. **/
		std::vector<unsigned char> vchPubKey;
		
		unsigned int nVersion;
		uint1024  hashGenesisBlock;
		uint512   hashGenesisTx;
		unsigned int nGenesisTime;
		
		/** Previous Blocks Vector to store list of blocks of this Trust Key. **/
		mutable std::stack<uint1024> hashPrevBlocks;
		
		CTrustKey() { SetNull(); }
		CTrustKey(std::vector<unsigned char> vchPubKeyIn, uint1024 hashBlockIn, uint512 hashTxIn, unsigned int nTimeIn)
		{
			SetNull();
			
			nVersion               = 1;
			vchPubKey              = vchPubKeyIn;
			hashGenesisBlock       = hashBlockIn;
			hashGenesisTx          = hashTxIn;
			nGenesisTime           = nTimeIn;
		}
		
		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			
			READWRITE(vchPubKey);
			READWRITE(hashGenesisBlock);
			READWRITE(hashGenesisTx);
			READWRITE(nGenesisTime);
		)
		
		
		/** Set the Data structure to Null. **/
		void SetNull() 
		{ 
			nVersion             = 1;
			hashGenesisBlock     = 0;
			hashGenesisTx        = 0;
			nGenesisTime         = 0;
			
			vchPubKey.clear();
		}
		
		/** Hash of a Trust Key to Verify the Key's Root. **/
		uint512 GetHash() const { return SK512(vchPubKey, BEGIN(hashGenesisBlock), END(nGenesisTime)); }
		
		/** Determine how old the Trust Key is From Timestamp. **/
		uint64 Age(unsigned int nTime) const;
		
		/** Time Since last Trust Block. **/
		uint64 BlockAge(unsigned int nTime) const;
		
		/** Flag to Determine if Class is Empty and Null. **/
		bool IsNull()  const { return (hashGenesisBlock == 0 || hashGenesisTx == 0 || nGenesisTime == 0 || vchPubKey.empty()); }
		bool Expired(unsigned int nTime) const;
		bool CheckGenesis(CBlock cBlock) const;
		
		void Print()
		{
			uint576 cKey;
			cKey.SetBytes(vchPubKey);
			
			printf("CTrustKey(Hash = %s, Key = %s, Genesis = %s, Tx = %s, Time = %u, Age = %"PRIu64", BlockAge = %"PRIu64", Expired = %s)\n", GetHash().ToString().c_str(), cKey.ToString().c_str(), hashGenesisBlock.ToString().c_str(), hashGenesisTx.ToString().c_str(), nGenesisTime, Age(GetUnifiedTimestamp()), BlockAge(GetUnifiedTimestamp()), Expired(GetUnifiedTimestamp()) ? "TRUE" : "FALSE");
		}
	};
	
	/** Holding Class Structure to contain the Trust Keys. **/
	class CTrustPool
	{
	private:
		mutable CCriticalSection cs;
		mutable std::map<uint576, CTrustKey> mapTrustKeys;

	public:
		/** The Trust Key Owned By Current Node. **/
		std::vector<unsigned char>   vchTrustKey;
		
		/** Helper Function to Find Trust Key. **/
		bool HasTrustKey(unsigned int nTime);
		
		bool Check(CBlock cBlock);
		bool Accept(CBlock cBlock, bool fInit = false);
		bool Remove(CBlock cBlock);
		
		bool Exists(uint576 cKey)    const { return mapTrustKeys.count(cKey); }
		bool IsGenesis(uint576 cKey) const { return mapTrustKeys[cKey].hashPrevBlocks.empty(); }
		
		double InterestRate(uint576 cKey, unsigned int nTime) const;
		
		CTrustKey Find(uint576 cKey) const { return mapTrustKeys[cKey]; }
	};

	extern CTrustPool cTrustPool;
	

	/** The basic transaction that is broadcasted on the network and contained in
	 * blocks.  A transaction can contain multiple inputs and outputs.
	 */
	class CTransaction
	{
	public:
		int nVersion;
		unsigned int nTime;
		std::vector<CTxIn> vin;
		std::vector<CTxOut> vout;
		unsigned int nLockTime;

		// Denial-of-service detection:
		mutable int nDoS;
		bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

		CTransaction()
		{
			SetNull();
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(nTime);
			READWRITE(vin);
			READWRITE(vout);
			READWRITE(nLockTime);
		)

		void SetNull()
		{
			nVersion = 1;
			nTime = GetUnifiedTimestamp();
			vin.clear();
			vout.clear();
			nLockTime = 0;
			nDoS = 0;  // Denial-of-service prevention
		}

		bool IsNull() const
		{
			return (vin.empty() && vout.empty());
		}

		uint512 GetHash() const
		{
			return SerializeHash(*this);
		}

		bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const
		{
			// Time based nLockTime implemented in 0.1.6
			if (nLockTime == 0)
				return true;
			if (nBlockHeight == 0)
				nBlockHeight = nBestHeight;
			if (nBlockTime == 0)
				nBlockTime = GetUnifiedTimestamp();
			if ((int64)nLockTime < ((int64)nLockTime < LOCKTIME_THRESHOLD ? (int64)nBlockHeight : nBlockTime))
				return true;
			BOOST_FOREACH(const CTxIn& txin, vin)
				if (!txin.IsFinal())
					return false;
			return true;
		}

		bool IsNewerThan(const CTransaction& old) const
		{
			if (vin.size() != old.vin.size())
				return false;
			for (unsigned int i = 0; i < vin.size(); i++)
				if (vin[i].prevout != old.vin[i].prevout)
					return false;

			bool fNewer = false;
			unsigned int nLowest = std::numeric_limits<unsigned int>::max();
			for (unsigned int i = 0; i < vin.size(); i++)
			{
				if (vin[i].nSequence != old.vin[i].nSequence)
				{
					if (vin[i].nSequence <= nLowest)
					{
						fNewer = false;
						nLowest = vin[i].nSequence;
					}
					if (old.vin[i].nSequence < nLowest)
					{
						fNewer = true;
						nLowest = old.vin[i].nSequence;
					}
				}
			}
			return fNewer;
		}

		/** Coinbase Transaction Rules: **/
		bool IsCoinBase() const
		{
			/** A] Input Size must be 1. **/
			if(vin.size() != 1)
				return false;
				
			/** B] First Input must be Empty. **/
			if(!vin[0].prevout.IsNull())
				return false;
				
			/** C] Outputs Count must Exceed 1. **/
			if(vout.size() < 1)
				return false;
				
			return true;
		}

		/** Coinstake Transaction Rules: **/
		bool IsCoinStake() const
		{
			/** A] Must have more than one Input. **/
			if(vin.size() <= 1)
				return false;
				
			/** B] First Input Script Signature must be 8 Bytes. **/
			if(vin[0].scriptSig.size() != 8)
				return false;
				
			/** C] First Input Script Signature must Contain Fibanacci Byte Series. **/
			if(!vin[0].IsStakeSig())
				return false;
				
			/** D] All Remaining Previous Inputs must not be Empty. **/
			for(int nIndex = 1; nIndex < vin.size(); nIndex++)
				if(vin[nIndex].prevout.IsNull())
					return false;
				
			/** E] Must Contain only 1 Outputs. **/
			if(vout.size() != 1)
				return false;
				
			return true;
		}
		
		/** Flag to determine if the transaction is a genesis transaction. **/
		bool IsGenesis() const
		{
			/** A] Genesis Transaction must be Coin Stake. **/
			if(!IsCoinStake())
				return false;
				
			/** B] First Input Previous Transaction must be Empty. **/
			if(!vin[0].prevout.IsNull())
				return false;
				
			return true;
		}
		
		
		/** Flag to determine if the transaction is a Trust Transaction. **/
		bool IsTrust() const
		{
			/** A] Genesis Transaction must be Coin Stake. **/
			if(!IsCoinStake())
				return false;
				
			/** B] First Input Previous Transaction must not be Empty. **/
			if(vin[0].prevout.IsNull())
				return false;
				
			/** C] First Input Previous Transaction Hash must not be 0. **/
			if(vin[0].prevout.hash == 0)
				return false;
				
			/** D] First Input Previous Transaction Index must be 0. **/
			if(vin[0].prevout.n != 0)
				return false;
				
			return true;
		}

		/** Check for standard transaction types
			@return True if all outputs (scriptPubKeys) use only standard transaction forms
		*/
		bool IsStandard() const;

		/** Check for standard transaction types
			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return True if all inputs (scriptSigs) use only standard transaction forms
			@see CTransaction::FetchInputs
		*/
		bool AreInputsStandard(const MapPrevTx& mapInputs) const;

		/** Count ECDSA signature operations the old-fashioned (pre-0.6) way
			@return number of sigops this transaction's outputs will produce when spent
			@see CTransaction::FetchInputs
		*/
		unsigned int GetLegacySigOpCount() const;

		/** Count ECDSA signature operations in pay-to-script-hash inputs.

			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return maximum number of sigops required to validate this transaction's inputs
			@see CTransaction::FetchInputs
		 */
		unsigned int TotalSigOps(const MapPrevTx& mapInputs) const;

		/** Amount of Coins spent by this transaction.
			@return sum of all outputs (note: does not include fees)
		 */
		int64 GetValueOut() const
		{
			int64 nValueOut = 0;
			BOOST_FOREACH(const CTxOut& txout, vout)
			{
				nValueOut += txout.nValue;
				if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
					throw std::runtime_error("CTransaction::GetValueOut() : value out of range");
			}
			return nValueOut;
		}

		/** Amount of Coins coming in to this transaction
			Note that lightweight clients may not know anything besides the hash of previous transactions,
			so may not be able to calculate this.

			@param[in] mapInputs	Map of previous transactions that have outputs we're spending
			@return	Sum of value of all inputs (scriptSigs)
			@see CTransaction::FetchInputs
		 */
		int64 GetValueIn(const MapPrevTx& mapInputs) const;

		static bool AllowFree(double dPriority)
		{
			// Large (in bytes) low-priority (new, small-coin) transactions
			// need a fee.
			return dPriority > COIN * 144 / 250;
		}

		int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=false, enum GetMinFee_mode mode=GMF_BLOCK) const
		{
			if(nVersion >= 2)
				return 0;
				
			// Base fee is either MIN_TX_FEE or MIN_RELAY_TX_FEE
			int64 nBaseFee = (mode == GMF_RELAY) ? MIN_RELAY_TX_FEE : MIN_TX_FEE;

			unsigned int nBytes = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
			unsigned int nNewBlockSize = nBlockSize + nBytes;
			int64 nMinFee = (1 + (int64)nBytes / 1000) * nBaseFee;

			if (fAllowFree)
			{
				if (nBlockSize == 1)
				{
					// Transactions under 10K are free
					// (about 4500bc if made of 50bc inputs)
					if (nBytes < 10000)
						nMinFee = 0;
				}
				else
				{
					// Free transaction area
					if (nNewBlockSize < 27000)
						nMinFee = 0;
				}
			}

			// To limit dust spam, require MIN_TX_FEE/MIN_RELAY_TX_FEE if any output is less than 0.01
			if (nMinFee < nBaseFee)
			{
				BOOST_FOREACH(const CTxOut& txout, vout)
					if (txout.nValue < CENT)
						nMinFee = nBaseFee;
			}

			// Raise the price as the block approaches full
			if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
			{
				if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
					return MAX_TXOUT_AMOUNT;
				nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
			}

			if (!MoneyRange(nMinFee))
				nMinFee = MAX_TXOUT_AMOUNT;
				
			return nMinFee;
		}


		bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
		{
			CAutoFile filein = CAutoFile(OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb"), SER_DISK, DATABASE_VERSION);
			if (!filein)
				return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");

			// Read transaction
			if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
				return error("CTransaction::ReadFromDisk() : fseek failed");

			try {
				filein >> *this;
			}
			catch (std::exception &e) {
				return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
			}

			// Return file pointer
			if (pfileRet)
			{
				if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
					return error("CTransaction::ReadFromDisk() : second fseek failed");
				*pfileRet = filein.release();
			}
			return true;
		}

		friend bool operator==(const CTransaction& a, const CTransaction& b)
		{
			return (a.nVersion  == b.nVersion &&
					a.nTime     == b.nTime &&
					a.vin       == b.vin &&
					a.vout      == b.vout &&
					a.nLockTime == b.nLockTime);
		}

		friend bool operator!=(const CTransaction& a, const CTransaction& b)
		{
			return !(a == b);
		}


		std::string ToStringShort() const
		{
			std::string str;
			str += strprintf("%s %s", GetHash().ToString().c_str(), IsCoinBase()? "base" : (IsCoinStake()? "stake" : "user"));
			return str;
		}

		std::string ToString() const
		{
			std::string str;
			str += IsCoinBase() ? "Coinbase" : (IsGenesis() ? "Genesis" : (IsTrust() ? "Trust" : "Transaction"));
			str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%d, vout.size=%d, nLockTime=%d)\n",
				GetHash().ToString().substr(0,10).c_str(),
				nTime,
				nVersion,
				vin.size(),
				vout.size(),
				nLockTime);
			for (unsigned int i = 0; i < vin.size(); i++)
				str += "    " + vin[i].ToString() + "\n";
			for (unsigned int i = 0; i < vout.size(); i++)
				str += "    " + vout[i].ToString() + "\n";
			return str;
		}

		void print() const
		{
			printf("%s", ToString().c_str());
		}


		bool GetCoinstakeInterest(LLD::CIndexDB& txdb, int64& nInterest) const;
		bool GetCoinstakeAge(LLD::CIndexDB& txdb, uint64& nAge) const;

		
		bool ReadFromDisk(LLD::CIndexDB& indexdb, COutPoint prevout, CTxIndex& txindexRet);
		bool ReadFromDisk(LLD::CIndexDB& indexdb, COutPoint prevout);
		bool ReadFromDisk(COutPoint prevout);
		bool DisconnectInputs(LLD::CIndexDB& indexdb);

		/** Fetch from memory and/or disk. inputsRet keys are transaction hashes.

		 @param[in] txdb	Transaction database
		 @param[in] mapTestPool	List of pending changes to the transaction index database
		 @param[in] fBlock	True if being called to add a new best-block to the chain
		 @param[in] fMiner	True if being called by CreateNewBlock
		 @param[out] inputsRet	Pointers to this transaction's inputs
		 @param[out] fInvalid	returns true if transaction is invalid
		 @return	Returns true if all inputs are in txdb or mapTestPool
		 */
		bool FetchInputs(LLD::CIndexDB& indexdb, const std::map<uint512, CTxIndex>& mapTestPool,
						 bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);

		/** Sanity check previous transactions, then, if all checks succeed,
			mark them as spent by this transaction.

			@param[in] inputs	Previous transactions (from FetchInputs)
			@param[out] mapTestPool	Keeps track of inputs that need to be updated on disk
			@param[in] posThisTx	Position of this transaction on disk
			@param[in] pindexBlock
			@param[in] fBlock	true if called from ConnectBlock
			@param[in] fMiner	true if called from CreateNewBlock
			@param[in] fStrictPayToScriptHash	true if fully validating p2sh transactions
			@return Returns true if all checks succeed
		 */
		bool ConnectInputs(LLD::CIndexDB& indexdb, MapPrevTx inputs,
						   std::map<uint512, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
						   const CBlockIndex* pindexBlock, bool fBlock, bool fMiner);
		bool ClientConnectInputs();
		bool CheckTransaction() const;
		bool AcceptToMemoryPool(LLD::CIndexDB& indexdb, bool fCheckInputs=true, bool* pfMissingInputs=NULL);
	

	protected:
		const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
	};


	/** A transaction with a merkle branch linking it to the block chain. */
	class CMerkleTx : public CTransaction
	{
	public:
		uint1024 hashBlock;
		std::vector<uint512> vMerkleBranch;
		int nIndex;

		// memory only
		mutable bool fMerkleVerified;


		CMerkleTx()
		{
			Init();
		}

		CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
		{
			Init();
		}

		void Init()
		{
			hashBlock = 0;
			nIndex = -1;
			fMerkleVerified = false;
		}


		IMPLEMENT_SERIALIZE
		(
			nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
			nVersion = this->nVersion;
			READWRITE(hashBlock);
			READWRITE(vMerkleBranch);
			READWRITE(nIndex);
		)


		int SetMerkleBranch(const CBlock* pblock=NULL);
		int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
		int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
		bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
		int GetBlocksToMaturity() const;
		bool AcceptToMemoryPool(LLD::CIndexDB& indexdb, bool fCheckInputs=true);
		bool AcceptToMemoryPool();
	};




	/**  A txdb record that contains the disk location of a transaction and the
	 * locations of transactions that spend its outputs.  vSpent is really only
	 * used as a flag, but having the location is very helpful for debugging.
	 */
	class CTxIndex
	{
	public:
		CDiskTxPos pos;
		std::vector<CDiskTxPos> vSpent;

		CTxIndex()
		{
			SetNull();
		}

		CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
		{
			pos = posIn;
			vSpent.resize(nOutputs);
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);
			READWRITE(pos);
			READWRITE(vSpent);
		)

		void SetNull()
		{
			pos.SetNull();
			vSpent.clear();
		}

		bool IsNull()
		{
			return pos.IsNull();
		}

		friend bool operator==(const CTxIndex& a, const CTxIndex& b)
		{
			return (a.pos    == b.pos &&
					a.vSpent == b.vSpent);
		}

		friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
		{
			return !(a == b);
		}
		int GetDepthInMainChain() const;
	 
	};





	/** Nodes collect new transactions into a block, hash them into a hash tree,
	 * and scan through nonce values to make the block's hash satisfy proof-of-work
	 * requirements.  When they solve the proof-of-work, they broadcast the block
	 * to everyone and the block is added to the block chain.  The first transaction
	 * in the block is a special one that creates a new coin owned by the creator
	 * of the block.
	 *
	 * Blocks are appended to blk0001.dat files on disk.  Their location on disk
	 * is indexed by CBlockIndex objects in memory.
	 */
	class CBlock
	{
	public:
		
		unsigned int nVersion;
		uint1024 hashPrevBlock;
		uint512 hashMerkleRoot;
		unsigned int nChannel;
		unsigned int nHeight;
		unsigned int nBits;
		uint64 nNonce;
		
		unsigned int nTime;

		// network and disk
		std::vector<CTransaction> vtx;

		/** Nexus: block signature - signed by coinbase / coinstake txout[0]'s owner.
			Seals the nTime and nNonce [For CPU Miners and nPoS] into block **/
		std::vector<unsigned char> vchBlockSig;

		// memory only
		mutable std::vector<uint512> vMerkleTree;

		// Denial-of-service detection:
		mutable int nDoS;
		bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

		CBlock()
		{
			SetNull();
		}
		
		CBlock(unsigned int nVersionIn, uint1024 hashPrevBlockIn, unsigned int nChannelIn, unsigned int nHeightIn)
		{
			SetNull();
			
			nVersion = nVersionIn;
			hashPrevBlock = hashPrevBlockIn;
			nChannel = nChannelIn;
			nHeight  = nHeightIn;
		}

		IMPLEMENT_SERIALIZE
		(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(hashPrevBlock);
			READWRITE(hashMerkleRoot);
			READWRITE(nChannel);
			READWRITE(nHeight);
			READWRITE(nBits);
			READWRITE(nNonce);
			READWRITE(nTime);

			// ConnectBlock depends on vtx following header to generate CDiskTxPos
			if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
			{
				READWRITE(vtx);
				READWRITE(vchBlockSig);
			}
			else if (fRead)
			{
				const_cast<CBlock*>(this)->vtx.clear();
				const_cast<CBlock*>(this)->vchBlockSig.clear();
			}
		)

		void SetNull()
		{
			nVersion = 3;
			hashPrevBlock = 0;
			hashMerkleRoot = 0;
			nChannel = 0;
			nHeight = 0;
			nBits = 0;
			nNonce = 0;
			
			nTime = 0;

			vtx.clear();
			vchBlockSig.clear();
			vMerkleTree.clear();
			nDoS = 0;
		}
		
		void SetChannel(unsigned int nNewChannel)
		{
			nChannel = nNewChannel;
		}
		
		int GetChannel() const
		{
			return nChannel;
		}

		bool IsNull() const
		{
			return (nBits == 0);
		}
		
		CBigNum GetPrime() const
		{
			return CBigNum(GetHash() + nNonce);
		}

		/** Generate a Hash For the Block from the Header. **/
		uint1024 GetHash() const
		{
			/** Hashing template for CPU miners uses nVersion to nBits **/
			if(GetChannel() == 1)
				return SK1024(BEGIN(nVersion), END(nBits));
				
			/** Hashing template for GPU uses nVersion to nNonce **/
			return SK1024(BEGIN(nVersion), END(nNonce));
		}
		
		
		/** Generate the Signature Hash Required After Block completes Proof of Work / Stake.
			This is to seal the Block Timestamp / nNonce [For CPU Channel] into the Block Signature while allowing it
			to be set independent of proof of work searching. This prevents this data from being tampered with after broadcast. **/
		uint1024 SignatureHash() const { return SK1024(BEGIN(nVersion), END(nTime)); }

		
		/** Return the Block's current Timestamp. **/
		int64 GetBlockTime() const
		{
			return (int64)nTime;
		}

		void UpdateTime();


		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}

		bool IsProofOfWork() const
		{
			return (nChannel == 1 || nChannel == 2);
		}


		uint512 BuildMerkleTree() const
		{
			vMerkleTree.clear();
			BOOST_FOREACH(const CTransaction& tx, vtx)
				vMerkleTree.push_back(tx.GetHash());
			int j = 0;
			for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				for (int i = 0; i < nSize; i += 2)
				{
					int i2 = std::min(i+1, nSize-1);
					vMerkleTree.push_back(SK512(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
											    BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
				}
				j += nSize;
			}
			return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
		}

		std::vector<uint512> GetMerkleBranch(int nIndex) const
		{
			if (vMerkleTree.empty())
				BuildMerkleTree();
			std::vector<uint512> vMerkleBranch;
			int j = 0;
			for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
			{
				int i = std::min(nIndex^1, nSize-1);
				vMerkleBranch.push_back(vMerkleTree[j+i]);
				nIndex >>= 1;
				j += nSize;
			}
			return vMerkleBranch;
		}

		static uint512 CheckMerkleBranch(uint512 hash, const std::vector<uint512>& vMerkleBranch, int nIndex)
		{
			if (nIndex == -1)
				return 0;
			BOOST_FOREACH(const uint512& otherside, vMerkleBranch)
			{
				if (nIndex & 1)
					hash = SK512(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
				else
					hash = SK512(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
				nIndex >>= 1;
			}
			return hash;
		}


		bool WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet)
		{
			// Open history file to append
			CAutoFile fileout = CAutoFile(AppendBlockFile(nFileRet), SER_DISK, DATABASE_VERSION);
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
			nBlockPosRet = fileOutPos;
			fileout << *this;

			// Flush stdio buffers and commit to disk before returning
			fflush(fileout);
			if (!IsInitialBlockDownload() || (nBestHeight+1) % 500 == 0)
			{
	#ifdef WIN32
				_commit(_fileno(fileout));
	#else
				fsync(fileno(fileout));
	#endif
			}

			return true;
		}

		bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions=true)
		{
			SetNull();

			// Open history file to read
			CAutoFile filein = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb"), SER_DISK, DATABASE_VERSION);
			if (!filein)
				return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
			if (!fReadTransactions)
				filein.nType |= SER_BLOCKHEADERONLY;

			// Read block
			try {
				filein >> *this;
			}
			catch (std::exception &e) {
				return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
			}

			return true;
		}



		void print() const
		{
			printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nChannel = %u, nHeight = %u, nNonce=%"PRIu64", vtx=%d, vchBlockSig=%s)\n",
				GetHash().ToString().substr(0,20).c_str(),
				nVersion,
				hashPrevBlock.ToString().substr(0,20).c_str(),
				hashMerkleRoot.ToString().substr(0,10).c_str(),
				nTime, nBits, nChannel, nHeight, nNonce,
				vtx.size(),
				HexStr(vchBlockSig.begin(), vchBlockSig.end()).c_str());
			for (unsigned int i = 0; i < vtx.size(); i++)
			{
				printf("  ");
				vtx[i].print();
			}
			printf("  vMerkleTree: ");
			for (unsigned int i = 0; i < vMerkleTree.size(); i++)
				printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
			printf("\n");
		}


		bool DisconnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		bool ConnectBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindex);
		bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);
		bool SetBestChain(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew);
		bool AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos);
		bool CheckBlock() const;
		
		bool VerifyWork() const;
		bool VerifyStake() const;
		
		bool AcceptBlock();
		bool StakeWeight();
		bool SignBlock(const Wallet::CKeyStore& keystore);
		bool CheckBlockSignature() const;
	};



	/** The block chain is a tree shaped structure starting with the
	 * genesis block at the root, with each block potentially having multiple
	 * candidates to be the next block.  pprev and pnext link a path through the
	 * main/longest chain.  A blockindex may have multiple pprev pointing back
	 * to it, but pnext will only point forward to the longest branch, or will
	 * be null if the block is not part of the longest chain.
	 */
	class CBlockIndex
	{
	public:
		const uint1024* phashBlock;
		CBlockIndex* pprev;
		CBlockIndex* pnext;
		unsigned int nFile;
		unsigned int nBlockPos;
		
		uint64 nChainTrust; // Nexus: trust score of block chain
		int64  nMint;
		int64  nMoneySupply;
		int64  nChannelHeight;
		int64  nReleasedReserve[4];
		int64  nCoinbaseRewards[3];
		
				
		/** Used to store the pending Checkpoint in Blockchain. 
			This is also another proof that this block is descendant 
			of most recent Pending Checkpoint. This helps Nexus
			deal with reorganizations of a Pending Checkpoint **/
		std::pair<unsigned int, uint1024> PendingCheckpoint;
		

		unsigned int nFlags;  // Nexus: block index flags
		enum  
		{
			BLOCK_PROOF_OF_STAKE = (1 << 0), // is proof-of-stake block
			BLOCK_STAKE_ENTROPY  = (1 << 1), // entropy bit for stake modifier
			BLOCK_STAKE_MODIFIER = (1 << 2), // regenerated stake modifier
		};

		uint64 nStakeModifier;

		// block header
		unsigned int nVersion;
		uint512 hashMerkleRoot;
		unsigned int nChannel;
		unsigned int nHeight;
		unsigned int nBits;
		uint64 nNonce;

		unsigned int nTime;

		CBlockIndex()
		{
			phashBlock = NULL;
			pprev = NULL;
			pnext = NULL;
			nFile = 0;
			nBlockPos = 0;
			
			nChainTrust = 0;
			nMint = 0;
			nMoneySupply = 0;
			nFlags = 0;
			nStakeModifier = 0;
			
			nCoinbaseRewards[0] = 0;
			nCoinbaseRewards[1] = 0;
			nCoinbaseRewards[2] = 0;

			nVersion       = 0;
			hashMerkleRoot = 0;
			nHeight        = 0;
			nBits          = 0;
			nNonce         = 0;
			
			nTime          = 0;
		}

		CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock& block)
		{
			phashBlock = NULL;
			pprev = NULL;
			pnext = NULL;
			nFile = nFileIn;
			nBlockPos = nBlockPosIn;
			nChainTrust = 0;
			nMint = 0;
			nMoneySupply = 0;
			nStakeModifier = 0;
			nFlags = 0;
			
			nCoinbaseRewards[0] = 0;
			nCoinbaseRewards[1] = 0;
			nCoinbaseRewards[2] = 0;
				
			if (block.IsProofOfWork() && block.nHeight > 0)
			{
				unsigned int nSize = block.vtx[0].vout.size();
				for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
					nCoinbaseRewards[0] += block.vtx[0].vout[nIndex].nValue;
						
				nCoinbaseRewards[1] = block.vtx[0].vout[nSize - 2].nValue;
				nCoinbaseRewards[2] = block.vtx[0].vout[nSize - 1].nValue;
			}

			nVersion       = block.nVersion;
			hashMerkleRoot = block.hashMerkleRoot;
			nChannel       = block.nChannel;
			nHeight        = block.nHeight;
			nBits          = block.nBits;
			nNonce         = block.nNonce;
			
			nTime          = block.nTime;
		}

		CBlock GetBlockHeader() const
		{
			CBlock block;
			block.nVersion       = nVersion;
			if (pprev)
				block.hashPrevBlock = pprev->GetBlockHash();
			block.hashMerkleRoot = hashMerkleRoot;
			block.nChannel       = nChannel;
			block.nHeight        = nHeight;
			block.nBits          = nBits;
			block.nNonce         = nNonce;
			
			block.nTime          = nTime;
			
			return block;
		}
		
		int GetChannel() const
		{
			return nChannel;
		}

		uint1024 GetBlockHash() const
		{
			return *phashBlock;
		}

		int64 GetBlockTime() const
		{
			return (int64)nTime;
		}

		uint64 GetBlockTrust() const
		{
				
			/** Give higher block trust if last block was of different channel **/
			if(pprev && pprev->GetChannel() != GetChannel())
				return 3;
			
			/** Normal Block Trust Increment. **/
			return 1;
		}

		bool IsInMainChain() const
		{
			return (pnext || this == pindexBest);
		}

		bool CheckIndex() const
		{
			return true;//IsProofOfWork() ? CheckProofOfWork(GetBlockHash(), nBits) : true;
		}

		bool EraseBlockFromDisk()
		{
			// Open history file
			CAutoFile fileout = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb+"), SER_DISK, DATABASE_VERSION);
			if (!fileout)
				return false;

			// Overwrite with empty null block
			CBlock block;
			block.SetNull();
			fileout << block;

			return true;
		}

		bool IsProofOfWork() const
		{
			return (nChannel > 0);
		}

		bool IsProofOfStake() const
		{
			return (nChannel == 0);
		}

		std::string ToString() const
		{
			return strprintf("CBlockIndex(nprev=%08x, pnext=%08x, nFile=%d, nBlockPos=%-6d nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s), merkle=%s, hashBlock=%s)",
				pprev, pnext, nFile, nBlockPos, nHeight,
				FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
				IsProofOfStake() ? "PoS" : "PoW",
				hashMerkleRoot.ToString().substr(0,10).c_str(),
				GetBlockHash().ToString().substr(0,20).c_str());
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};



	/** Used to marshal pointers into hashes for db storage. */
	class CDiskBlockIndex : public CBlockIndex
	{
	public:
		uint1024 hashPrev;
		uint1024 hashNext;

		CDiskBlockIndex()
		{
			hashPrev = 0;
			hashNext = 0;
		}

		explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
		{
			hashPrev = (pprev ? pprev->GetBlockHash() : 0);
			hashNext = (pnext ? pnext->GetBlockHash() : 0);
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);

			if (!(nType & SER_LLD)) 
			{
				READWRITE(hashNext);
				READWRITE(nFile);
				READWRITE(nBlockPos);
				READWRITE(nMint);
				READWRITE(nMoneySupply);
				READWRITE(nFlags);
				READWRITE(nStakeModifier);
			}
			else
			{
				READWRITE(hashNext);
				READWRITE(nFile);
				READWRITE(nBlockPos);
				READWRITE(nMint);
				READWRITE(nMoneySupply);
				READWRITE(nChannelHeight);
				READWRITE(nChainTrust);
				
				READWRITE(nCoinbaseRewards[0]);
				READWRITE(nCoinbaseRewards[1]);
				READWRITE(nCoinbaseRewards[2]);
				READWRITE(nReleasedReserve[0]);
				READWRITE(nReleasedReserve[1]);
				READWRITE(nReleasedReserve[2]);
				
				//READWRITE(PendingCheckpoint);
			}

			// block header
			READWRITE(this->nVersion);
			READWRITE(hashPrev);
			READWRITE(hashMerkleRoot);
			READWRITE(nChannel);
			READWRITE(nHeight);
			READWRITE(nBits);
			READWRITE(nNonce);
			READWRITE(nTime);
			
		)

		uint1024 GetBlockHash() const
		{
			CBlock block;
			block.nVersion        = nVersion;
			block.hashPrevBlock   = hashPrev;
			block.hashMerkleRoot  = hashMerkleRoot;
			block.nChannel        = nChannel;
			block.nHeight         = nHeight;
			block.nBits           = nBits;
			block.nNonce          = nNonce;
			
			return block.GetHash();
		}


		std::string ToString() const
		{
			std::string str = "CDiskBlockIndex(";
			str += CBlockIndex::ToString();
			str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashNext=%s)",
				GetBlockHash().ToString().c_str(),
				hashPrev.ToString().substr(0,20).c_str(),
				hashNext.ToString().substr(0,20).c_str());
			return str;
		}

		void print() const
		{
			printf("%s\n", ToString().c_str());
		}
	};




	/** Describes a place in the block chain to another node such that if the
	 * other node doesn't have the same branch, it can find a recent common trunk.
	 * The further back it is, the further before the fork it may be.
	 */
	class CBlockLocator
	{
	protected:
		std::vector<uint1024> vHave;
	public:

		CBlockLocator()
		{
		}

		explicit CBlockLocator(const CBlockIndex* pindex)
		{
			Set(pindex);
		}

		explicit CBlockLocator(uint1024 hashBlock)
		{
			std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
			if (mi != mapBlockIndex.end())
				Set((*mi).second);
		}

		CBlockLocator(const std::vector<uint1024>& vHaveIn)
		{
			vHave = vHaveIn;
		}

		IMPLEMENT_SERIALIZE
		(
			if (!(nType & SER_GETHASH))
				READWRITE(nVersion);
			READWRITE(vHave);
		)

		void SetNull()
		{
			vHave.clear();
		}

		bool IsNull()
		{
			return vHave.empty();
		}

		void Set(const CBlockIndex* pindex)
		{
			vHave.clear();
			int nStep = 1;
			while (pindex)
			{
				vHave.push_back(pindex->GetBlockHash());

				// Exponentially larger steps back
				for (int i = 0; pindex && i < nStep; i++)
					pindex = pindex->pprev;
				if (vHave.size() > 10)
					nStep *= 2;
			}
			vHave.push_back(hashGenesisBlock);
		}

		int GetDistanceBack()
		{
			// Retrace how far back it was in the sender's branch
			int nDistance = 0;
			int nStep = 1;
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return nDistance;
				}
				nDistance += nStep;
				if (nDistance > 10)
					nStep *= 2;
			}
			return nDistance;
		}

		CBlockIndex* GetBlockIndex()
		{
			// Find the first block the caller has in the main chain
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return pindex;
				}
			}
			return pindexGenesisBlock;
		}

		uint1024 GetBlockHash()
		{
			// Find the first block the caller has in the main chain
			BOOST_FOREACH(const uint1024& hash, vHave)
			{
				std::map<uint1024, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
				if (mi != mapBlockIndex.end())
				{
					CBlockIndex* pindex = (*mi).second;
					if (pindex->IsInMainChain())
						return hash;
				}
			}
			return hashGenesisBlock;
		}

		int GetHeight()
		{
			CBlockIndex* pindex = GetBlockIndex();
			if (!pindex)
				return 0;
			return pindex->nHeight;
		}
	};



	class CTxMemPool
	{
	public:
		mutable CCriticalSection cs;
		std::map<uint512, CTransaction> mapTx;
		std::map<COutPoint, CInPoint> mapNextTx;

		bool accept(LLD::CIndexDB& indexdb, CTransaction &tx,
					bool fCheckInputs, bool* pfMissingInputs);
		bool addUnchecked(CTransaction &tx);
		bool remove(CTransaction &tx);
		void queryHashes(std::vector<uint512>& vtxid);

		unsigned long size()
		{
			LOCK(cs);
			return mapTx.size();
		}

		bool exists(uint512 hash)
		{
			return (mapTx.count(hash) != 0);
		}

		CTransaction& lookup(uint512 hash)
		{
			return mapTx[hash];
		}
	};

	extern CTxMemPool mempool;

}
#endif
