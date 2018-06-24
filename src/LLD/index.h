#ifndef LOWER_LEVEL_LIBRARY_LLD_INDEX
#define LOWER_LEVEL_LIBRARY_LLD_INDEX

#include "sector.h"
#include "../core/core.h"
#include "../wallet/db.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
#ifdef USE_LLD
	class CIndexDB : public SectorDatabase
	{
	public:
		/** The Database Constructor. To determine file location and the Bytes per Record. **/
		CIndexDB(const char* pszMode="r+") : SectorDatabase("blkindex", "blkindex", pszMode) {}
		
		bool ReadTxIndex(uint512 hash, Core::CTxIndex& txindex);
		bool UpdateTxIndex(uint512 hash, const Core::CTxIndex& txindex);
		bool AddTxIndex(const Core::CTransaction& tx, const Core::CDiskTxPos& pos, int nHeight);
		bool EraseTxIndex(const Core::CTransaction& tx);
		bool ContainsTx(uint512 hash);
		bool ReadDiskTx(uint512 hash, Core::CTransaction& tx, Core::CTxIndex& txindex);
		bool ReadDiskTx(uint512 hash, Core::CTransaction& tx);
		bool ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx, Core::CTxIndex& txindex);
		bool ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx);
		bool WriteBlockIndex(const Core::CDiskBlockIndex& blockindex);
        bool ReadBlockIndex(const uint1024 hashBlock, Core::CBlockIndex* pindexNew);
		bool ReadHashBestChain(uint1024& hashBestChain);
		bool WriteHashBestChain(uint1024 hashBestChain);
		bool LoadBlockIndex();
		
		bool WriteTrustKey(uint512 hashTrustKey, Core::CTrustKey cTrustKey);
		bool ReadTrustKey(uint512 hashTrustKey, Core::CTrustKey& cTrustKey);
		bool AddTrustBlock(uint512 hashTrustKey, uint1024 hashTrustBlock);
		bool RemoveTrustBlock(uint1024 hashTrustBlock);
	};
#else
	class CIndexDB : public Wallet::CDB
	{
	public:
		CIndexDB(const char* pszMode="r+") : Wallet::CDB("blkindex.dat", pszMode) { }
		
		bool ReadTxIndex(uint512 hash, Core::CTxIndex& txindex);
		bool UpdateTxIndex(uint512 hash, const Core::CTxIndex& txindex);
		bool AddTxIndex(const Core::CTransaction& tx, const Core::CDiskTxPos& pos, int nHeight);
		bool EraseTxIndex(const Core::CTransaction& tx);
		bool ContainsTx(uint512 hash);
		bool ReadDiskTx(uint512 hash, Core::CTransaction& tx, Core::CTxIndex& txindex);
		bool ReadDiskTx(uint512 hash, Core::CTransaction& tx);
		bool ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx, Core::CTxIndex& txindex);
		bool ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx);
		bool WriteBlockIndex(const Core::CDiskBlockIndex& blockindex);
        bool ReadBlockIndex(const uint1024 hashBlock, Core::CBlockIndex* pindexNew);
		bool ReadHashBestChain(uint1024& hashBestChain);
		bool WriteHashBestChain(uint1024 hashBestChain);
		bool LoadBlockIndex();
		
		bool WriteTrustKey(uint512 hashTrustKey, Core::CTrustKey cTrustKey);
		bool ReadTrustKey(uint512 hashTrustKey, Core::CTrustKey& cTrustKey);
		bool AddTrustBlock(uint512 hashTrustKey, uint1024 hashTrustBlock);
		bool RemoveTrustBlock(uint1024 hashTrustBlock);
	};
#endif
}

#endif
