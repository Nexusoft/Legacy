#include "index.h"
#include "trustkeys.h"

#include "../main.h"
#include "../core/core.h"

#include "../util/ui_interface.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
    using namespace std;

    uint1024 hashCorruptedNext = 0;

    bool CIndexDB::ReadTxIndex(uint512 hash, Core::CTxIndex& txindex)
    {
        txindex.SetNull();
        return Read(make_pair(string("tx"), hash), txindex);
    }

    bool CIndexDB::UpdateTxIndex(uint512 hash, const Core::CTxIndex& txindex)
    {
        return Write(make_pair(string("tx"), hash), txindex);
    }

    bool CIndexDB::EraseTxIndex(const Core::CTransaction& tx)
    {
        assert(!Net::fClient);
        uint512 hash = tx.GetHash();

        return Erase(make_pair(string("tx"), hash));
    }

    bool CIndexDB::AddTxIndex(const Core::CTransaction& tx, const Core::CDiskTxPos& pos, int nHeight)
    {
        // Add to tx index
        uint512 hash = tx.GetHash();
        Core::CTxIndex txindex(pos, tx.vout.size());
        return Write(make_pair(string("tx"), hash), txindex);
    }

    bool CIndexDB::ContainsTx(uint512 hash)
    {
        assert(!Net::fClient);
        return Exists(make_pair(string("tx"), hash));
    }

    bool CIndexDB::ReadDiskTx(uint512 hash, Core::CTransaction& tx, Core::CTxIndex& txindex)
    {
        assert(!Net::fClient);
        tx.SetNull();
        if (!ReadTxIndex(hash, txindex))
            return false;
        return (tx.ReadFromDisk(txindex.pos));
    }

    bool CIndexDB::ReadDiskTx(uint512 hash, Core::CTransaction& tx)
    {
        Core::CTxIndex txindex;
        return ReadDiskTx(hash, tx, txindex);
    }

    bool CIndexDB::ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx, Core::CTxIndex& txindex)
    {
        return ReadDiskTx(outpoint.hash, tx, txindex);
    }

    bool CIndexDB::ReadDiskTx(Core::COutPoint outpoint, Core::CTransaction& tx)
    {
        Core::CTxIndex txindex;
        return ReadDiskTx(outpoint.hash, tx, txindex);
    }

    bool CIndexDB::WriteBlockIndex(const Core::CDiskBlockIndex& blockindex)
    {
        return Write(make_pair(string("blockindex"), blockindex.GetBlockHash()), blockindex);
    }

    bool CIndexDB::ReadBlockIndex(const uint1024 hashBlock, Core::CBlockIndex* pindexNew)
    {
        Core::CDiskBlockIndex diskindex;
        if(!Read(make_pair(string("blockindex"), hashBlock), diskindex))
            return false;

        pindexNew                 = new Core::CBlockIndex();
        pindexNew->phashBlock     = &hashBlock;
        //pindexNew->pprev        = InsertBlockIndex(diskindex.hashPrev);
        //pindexNew->pnext        = InsertBlockIndex(diskindex.hashNext);
        pindexNew->nFile          = diskindex.nFile;
        pindexNew->nBlockPos      = diskindex.nBlockPos;
        pindexNew->nMint          = diskindex.nMint;
        pindexNew->nMoneySupply   = diskindex.nMoneySupply;
        pindexNew->nChannelHeight = diskindex.nChannelHeight;
        pindexNew->nChainTrust    = diskindex.nChainTrust;

        /* Handle the Reserves. */
        pindexNew->nCoinbaseRewards[0] = diskindex.nCoinbaseRewards[0];
        pindexNew->nCoinbaseRewards[1] = diskindex.nCoinbaseRewards[1];
        pindexNew->nCoinbaseRewards[2] = diskindex.nCoinbaseRewards[2];
        pindexNew->nReleasedReserve[0] = diskindex.nReleasedReserve[0];
        pindexNew->nReleasedReserve[1] = diskindex.nReleasedReserve[1];
        pindexNew->nReleasedReserve[2] = diskindex.nReleasedReserve[2];

        /* Handle the Block Headers. */
        pindexNew->nVersion       = diskindex.nVersion;
        pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
        pindexNew->nChannel       = diskindex.nChannel;
        pindexNew->nHeight        = diskindex.nHeight;
        pindexNew->nBits          = diskindex.nBits;
        pindexNew->nNonce         = diskindex.nNonce;
        pindexNew->nTime          = diskindex.nTime;

        return true;
    }

    bool CIndexDB::ReadHashBestChain(uint1024& hashBestChain)
    {
        return Read(string("hashBestChain"), hashBestChain);
    }

    bool CIndexDB::WriteHashBestChain(uint1024 hashBestChain)
    {
        return Write(string("hashBestChain"), hashBestChain);
    }

    bool CIndexDB::WriteTrustKey(uint576 hashTrustKey, Core::CTrustKey cTrustKey)
    {
        return Write(make_pair(string("trustKey"), hashTrustKey), cTrustKey);
    }

    bool CIndexDB::HasTrustKey(uint576 hashTrustKey)
    {
        return Exists(make_pair(string("trustKey"), hashTrustKey));
    }

    //this will be very slow. Think about optimizing it further.
    bool CIndexDB::GetTrustKeys(std::vector<uint576>& vTrustKeys)
    {
        KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
        if(!SectorKeys)
            return error("Get() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());

        /* Get the keys from the keychain. */
        std::vector< std::vector<unsigned char> > vKeys = SectorKeys->GetKeys();

        /* Loop all the keys to check. */
        for(auto vKey : vKeys)
        {
            CDataStream ssKey(vKey, SER_LLD, DATABASE_VERSION);
            std::string strKey;
            ssKey >> strKey;

            if(strKey == "trustKey")
            {
                uint576 key;
                ssKey >> key;

                vTrustKeys.push_back(key);
            }
        }

        return vTrustKeys.size() > 0;
    }

    bool CIndexDB::ReadTrustKey(uint576 hashTrustKey, Core::CTrustKey& cTrustKey)
    {
        return Read(make_pair(string("trustKey"), hashTrustKey), cTrustKey);
    }

    bool CIndexDB::EraseTrustKey(uint576 hashTrustKey)
    {
        return Erase(make_pair(string("trustKey"), hashTrustKey));
    }

    bool CIndexDB::Bootstrapped()
    {
        uint256 hash;
        return Read(string("bootstrapped"), hash);
    }

    bool CIndexDB::Bootstrap()
    {
        uint256 hash = 0;
        return Write(string("bootstrapped"), hash);
    }


    Core::CBlockIndex static * InsertBlockIndex(uint1024 hash)
    {
        if (hash == 0)
            return NULL;

        // Return existing
        map<uint1024, Core::CBlockIndex*>::iterator mi = Core::mapBlockIndex.find(hash);
        if (mi != Core::mapBlockIndex.end())
            return (*mi).second;

        // Create new
        Core::CBlockIndex* pindexNew = new Core::CBlockIndex();
        if (!pindexNew)
            throw runtime_error("LoadBlockIndex() : new Core::CBlockIndex failed");

        mi = Core::mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        return pindexNew;
    }

#ifdef USE_LLD
    bool CIndexDB::LoadBlockIndex()
    {
        if(!ReadHashBestChain(Core::hashBestChain))
            return error("No Hash Best Chain in Index Database.");


        unsigned int nTotalBlocks = 0;
        Core::CDiskBlockIndex diskindexBest;
        if(Read(make_pair(string("blockindex"), Core::hashBestChain), diskindexBest))
            nTotalBlocks = diskindexBest.nHeight;


        uint1024 hashBlock = Core::hashGenesisBlock;
        std::vector<uint576> vTrustKeys;

        bool fBootstrap = !Bootstrapped();
        std::map<uint576, unsigned int> mapLastBlocks;
        while(!fRequestShutdown)
        {
            Core::CDiskBlockIndex diskindex;
            if(!Read(make_pair(string("blockindex"), hashBlock), diskindex))
            {
                printf("Failed to Read Block %s Height %u\n", hashBlock.ToString().c_str(), diskindex.nHeight);

                break;
            }

            Core::CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
            pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
            pindexNew->pnext          = InsertBlockIndex(diskindex.hashNext);
            pindexNew->nFile          = diskindex.nFile;
            pindexNew->nBlockPos      = diskindex.nBlockPos;
            pindexNew->nMint          = diskindex.nMint;
            pindexNew->nMoneySupply   = diskindex.nMoneySupply;
            pindexNew->nChannelHeight = diskindex.nChannelHeight;
            pindexNew->nChainTrust    = diskindex.nChainTrust;

            /** Handle the Reserves. **/
            pindexNew->nCoinbaseRewards[0] = diskindex.nCoinbaseRewards[0];
            pindexNew->nCoinbaseRewards[1] = diskindex.nCoinbaseRewards[1];
            pindexNew->nCoinbaseRewards[2] = diskindex.nCoinbaseRewards[2];
            pindexNew->nReleasedReserve[0] = diskindex.nReleasedReserve[0];
            pindexNew->nReleasedReserve[1] = diskindex.nReleasedReserve[1];
            pindexNew->nReleasedReserve[2] = diskindex.nReleasedReserve[2];

            /** Handle the Block Headers. **/
            pindexNew->nVersion       = diskindex.nVersion;
            pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
            pindexNew->nChannel       = diskindex.nChannel;
            pindexNew->nHeight        = diskindex.nHeight;
            pindexNew->nBits          = diskindex.nBits;
            pindexNew->nNonce         = diskindex.nNonce;
            pindexNew->nTime          = diskindex.nTime;

            /** Detect the Genesis Block on Loading. */
            if(hashBlock == Core::hashGenesisBlock)
                Core::pindexGenesisBlock = pindexNew;
            else
            {
                //TODO: Optimize Correctly
                if(GetBoolArg("-richlist", false))
                {
                    Core::CBlock block;
                    if(!block.ReadFromDisk(pindexNew))
                        continue;

                    /** Grab the transactions for the block and set the address balances. **/
                    for(int nTx = 0; nTx < block.vtx.size(); nTx++)
                    {
                        for(int nOut = 0; nOut < block.vtx[nTx].vout.size(); nOut++)
                        {
                            Wallet::NexusAddress cAddress;
                            if(!ExtractAddress(block.vtx[nTx].vout[nOut].scriptPubKey, cAddress))
                                continue;

                            Core::mapAddressTransactions[cAddress.GetHash256()] += block.vtx[nTx].vout[nOut].nValue;

                            if(!Core::mapRichList.count(cAddress.GetHash256()))
                                Core::mapRichList[cAddress.GetHash256()] = { std::make_pair(block.vtx[nTx].IsCoinBase(), block.vtx[nTx].GetHash()) };
                            else
                                Core::mapRichList[cAddress.GetHash256()].push_back(std::make_pair(block.vtx[nTx].IsCoinBase(), block.vtx[nTx].GetHash()));
                        }

                        if(!block.vtx[nTx].IsCoinBase())
                        {
                            for(auto txin : block.vtx[nTx].vin)
                            {
                                if(txin.IsStakeSig())
                                    continue;

                                Core::CTransaction tx;
                                Core::CTxIndex txind;

                                if(!ReadTxIndex(txin.prevout.hash, txind))
                                    continue;

                                if(!tx.ReadFromDisk(txind.pos))
                                    continue;

                                Wallet::NexusAddress cAddress;
                                if(!ExtractAddress(tx.vout[txin.prevout.n].scriptPubKey, cAddress))
                                    continue;

                                Core::mapAddressTransactions[cAddress.GetHash256()] = std::max((uint64)0, Core::mapAddressTransactions[cAddress.GetHash256()] - tx.vout[txin.prevout.n].nValue);

                                if(!Core::mapRichList.count(cAddress.GetHash256()))
                                    Core::mapRichList[cAddress.GetHash256()] = { std::make_pair(block.vtx[nTx].IsCoinBase(), tx.GetHash()) };
                                else
                                    Core::mapRichList[cAddress.GetHash256()].push_back(std::make_pair(block.vtx[nTx].IsCoinBase(), tx.GetHash()));
                            }
                        }
                    }
                }

                if(fBootstrap && pindexNew->IsProofOfStake() && pindexNew->nVersion < 5)
                {
                    Core::CBlock block;
                    if(!block.ReadFromDisk(pindexNew))
                        return error("CTxDB::LoadBlockIndex() : Failed to Read Block");

                    std::vector<unsigned char> vTrustKey;
                    if(!block.TrustKey(vTrustKey))
                        return error("CTxDb::LoadBlockIndex() : Failed to extract new trust key");

                    uint576 cKey;
                    cKey.SetBytes(vTrustKey);

                    //log the time of the last block
                    mapLastBlocks[cKey] = block.nTime;

                    Core::CTrustKey trustKey;
                    if(!ReadTrustKey(cKey, trustKey))
                    {
                        trustKey = Core::CTrustKey(vTrustKey, block.GetHash(), block.vtx[0].GetHash(), block.nTime);
                        trustKey.hashLastBlock = block.GetHash();
                        WriteTrustKey(cKey, trustKey);

                        vTrustKeys.push_back(cKey);

                        //printf("CTxDb::LoadBlockIndex() : Writing Genesis %u Key to Disk %s\n", block.nHeight, cKey.ToString().substr(0, 20).c_str());
                    }
                    else
                    {
                        trustKey.hashLastBlock = block.GetHash();
                        WriteTrustKey(cKey, trustKey);
                    }
                }

                if(nTotalBlocks > 0)
                {
                    if(fBootstrap && pindexNew->nHeight % 1000 == 0)
                    {
                        std::string str = strprintf("Reindexing %.2f %% Complete... Do Not Close.", (pindexNew->nHeight * 100.0 / nTotalBlocks));
                        InitMessage(str.c_str());
                    }
                    else if(pindexNew->nHeight % 1000 == 0)
                    {
                        std::string str = strprintf("Index Loading %.2f %% Complete...", (pindexNew->nHeight * 100.0 / nTotalBlocks));
                        InitMessage(str.c_str());
                    }
                }
            }

            /** Add the Pending Checkpoint into the Blockchain. **/
            if(!pindexNew->pprev || Core::HardenCheckpoint(pindexNew, true))
                pindexNew->PendingCheckpoint = make_pair(pindexNew->nHeight, pindexNew->GetBlockHash());
            else
                pindexNew->PendingCheckpoint = pindexNew->pprev->PendingCheckpoint;

            Core::pindexBest  = pindexNew;
            hashBlock = diskindex.hashNext;
        }


        /* Check for my trust key. */
        if(fBootstrap)
        {
            InitMessage("Cleaning up Expired Trust Keys...");
            Bootstrap();

            LLD::CTrustDB trustdb("r+");
            Core::CTrustKey myTrustKey;
            uint576 myKey;

            /* If one has their trust key. */
            bool fHasKey = trustdb.ReadMyKey(myTrustKey);
            if(fHasKey)
                myKey.SetBytes(myTrustKey.vchPubKey);

            /* Erase expired trust keys. */
            for(auto key : vTrustKeys)
            {
                if(mapLastBlocks.count(key) && mapLastBlocks[key] + Core::TRUST_KEY_EXPIRE < Core::pindexBest->GetBlockTime())
                {
                    EraseTrustKey(key);

                    printf("Erasing Expired Trust Key %s\n", HexStr(key.begin(), key.end()).c_str());

                    if(fHasKey && key == myKey)
                        trustdb.EraseMyKey();

                    continue;
                }
            }
        }

        if(fRequestShutdown)
            return false;

        Core::nBestHeight     = Core::pindexBest->nHeight;
        Core::nBestChainTrust = Core::pindexBest->nChainTrust;
        Core::hashBestChain   = Core::pindexBest->GetBlockHash();
        printf("[DATABASE] Indexes Loaded. Height %u Hash %s\n", Core::nBestHeight, Core::pindexBest->GetBlockHash().ToString().substr(0, 20).c_str());

        /* Handle corrupted database. */
        if(Core::pindexBest->pnext)
        {
            /* Get the hash of the next block. */
            hashCorruptedNext = Core::pindexBest->pnext->GetBlockHash();

            /* Debug output. */
            printf("[DATABASE] Found corrupted pnext %s... Resolving\n", hashCorruptedNext.ToString().substr(0, 20).c_str());

            /* Set the memory index to 0 */
            Core::pindexBest->pnext = 0;

            /* Set the disk index back to 0. */
            Core::CDiskBlockIndex blockindex(Core::pindexBest);
            blockindex.hashNext = 0;
            if (!WriteBlockIndex(blockindex))
                return error("LoadBlockIndex() : WriteBlockIndex failed");

            /* Erase mapblockindex if found. */
            if(Core::mapBlockIndex.count(hashCorruptedNext))
                Core::mapBlockIndex.erase(hashCorruptedNext);
        }

        /** Verify the Blocks in the Best Chain To Last Checkpoint. **/
        int nCheckLevel = GetArg("-checklevel", 6);
        int nCheckDepth = GetArg("-checkblocks", 10);
        if (nCheckDepth == 0)
            nCheckDepth = 1000000000;

        if (nCheckDepth > Core::nBestHeight)
            nCheckDepth = Core::nBestHeight;
        printf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
        Core::CBlockIndex* pindexFork = NULL;

        /* Allow Forking out an old chain. */
        unsigned int nFork = GetArg("-forkblocks", 0);
        if(nFork > 0 && Core::pindexBest)
        {
            pindexFork = Core::pindexBest;
            for(int nIndex = 0; nIndex < nFork; nIndex++)
            {
                if(!pindexFork->pprev)
                    break;

                pindexFork = pindexFork->pprev;
            }
        }


        map<pair<unsigned int, unsigned int>, Core::CBlockIndex*> mapBlockPos;
        for (Core::CBlockIndex* pindex = Core::pindexBest; pindex && pindex->pprev && nCheckDepth > 0; pindex = pindex->pprev)
        {
            if (pindex->nHeight <= Core::nBestHeight - nCheckDepth)
                break;

            Core::CBlock block;
            if (!block.ReadFromDisk(pindex))
                return error("LoadBlockIndex() : block.ReadFromDisk failed");

            block.print();

            if (nCheckLevel > 0 && !block.CheckBlock())
            {
                printf("LoadBlockIndex() : *** found bad block at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str());
                pindexFork = pindex->pprev;
            }

            // check level 2: verify transaction index validity
            if (nCheckLevel>1)
            {
                pair<unsigned int, unsigned int> pos = make_pair(pindex->nFile, pindex->nBlockPos);
                mapBlockPos[pos] = pindex;
                BOOST_FOREACH(const Core::CTransaction &tx, block.vtx)
                {
                    uint512 hashTx = tx.GetHash();
                    Core::CTxIndex txindex;
                    if (ReadTxIndex(hashTx, txindex))
                    {

                        // check level 3: checker transaction hashes
                        if (nCheckLevel>2 || pindex->nFile != txindex.pos.nFile || pindex->nBlockPos != txindex.pos.nBlockPos)
                        {
                            // either an error or a duplicate transaction
                            Core::CTransaction txFound;
                            if (!txFound.ReadFromDisk(txindex.pos))
                            {
                                printf("LoadBlockIndex() : *** cannot read mislocated transaction %s\n", hashTx.ToString().c_str());
                                pindexFork = pindex->pprev;
                            }
                            else
                                if (txFound.GetHash() != hashTx) // not a duplicate tx
                                {
                                    printf("LoadBlockIndex(): *** invalid tx position for %s\n", hashTx.ToString().c_str());
                                    pindexFork = pindex->pprev;
                                }
                        }
                        // check level 4: check whether spent txouts were spent within the main chain
                        unsigned int nOutput = 0;
                        if (nCheckLevel>3)
                        {
                            BOOST_FOREACH(const Core::CDiskTxPos &txpos, txindex.vSpent)
                            {
                                if (!txpos.IsNull())
                                {
                                    pair<unsigned int, unsigned int> posFind = make_pair(txpos.nFile, txpos.nBlockPos);
                                    if (!mapBlockPos.count(posFind))
                                    {
                                        printf("LoadBlockIndex(): *** found bad spend at %d, hashBlock=%s, hashTx=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str(), hashTx.ToString().c_str());
                                        pindexFork = pindex->pprev;
                                    }
                                    // check level 6: check whether spent txouts were spent by a valid transaction that consume them
                                    if (nCheckLevel>5)
                                    {
                                        Core::CTransaction txSpend;
                                        if (!txSpend.ReadFromDisk(txpos))
                                        {
                                            printf("LoadBlockIndex(): *** cannot read spending transaction of %s:%i from disk\n", hashTx.ToString().c_str(), nOutput);
                                            pindexFork = pindex->pprev;
                                        }
                                        else if (!txSpend.CheckTransaction())
                                        {
                                            printf("LoadBlockIndex(): *** spending transaction of %s:%i is invalid\n", hashTx.ToString().c_str(), nOutput);
                                            pindexFork = pindex->pprev;
                                        }
                                        else
                                        {
                                            bool fFound = false;
                                            BOOST_FOREACH(const Core::CTxIn &txin, txSpend.vin)
                                                if (txin.prevout.hash == hashTx && txin.prevout.n == nOutput)
                                                    fFound = true;
                                            if (!fFound)
                                            {
                                                printf("LoadBlockIndex(): *** spending transaction of %s:%i does not spend it\n", hashTx.ToString().c_str(), nOutput);
                                                pindexFork = pindex->pprev;
                                            }
                                        }
                                    }
                                }
                                nOutput++;
                            }
                        }
                    }

                    // check level 5: check whether all prevouts are marked spent
                    if (nCheckLevel>4)
                    {
                        BOOST_FOREACH(const Core::CTxIn &txin, tx.vin)
                        {
                            Core::CTxIndex txindex;
                            if (ReadTxIndex(txin.prevout.hash, txindex))
                                if (txindex.vSpent.size()-1 < txin.prevout.n || txindex.vSpent[txin.prevout.n].IsNull())
                                {
                                    printf("LoadBlockIndex(): *** found unspent prevout %s:%i in %s\n", txin.prevout.hash.ToString().c_str(), txin.prevout.n, hashTx.ToString().c_str());
                                    pindexFork = pindex->pprev;
                                }
                        }
                    }
                }
            }
        }
        if (pindexFork)
        {
            // Reorg back to the fork
            printf("LoadBlockIndex() : *** moving best chain pointer back to block %d\n", pindexFork->nHeight);
            Core::CBlock block;
            if (!block.ReadFromDisk(pindexFork))
                return error("LoadBlockIndex() : block.ReadFromDisk failed");

            CIndexDB txdb;
            block.SetBestChain(txdb, pindexFork);
        }

        return true;
    }
#else
    bool CIndexDB::LoadBlockIndex()
    {

        Dbc* pcursor = GetCursor();
        if (!pcursor)
            return false;


        unsigned int fFlags = DB_SET_RANGE;
        loop() {
            // Read next record
            CDataStream ssKey(SER_DISK, DATABASE_VERSION);
            if (fFlags == DB_SET_RANGE)
                ssKey << make_pair(string("blockindex"), uint1024(0));
            CDataStream ssValue(SER_DISK, DATABASE_VERSION);
            int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
            fFlags = DB_NEXT;
            if (ret == DB_NOTFOUND)
                break;
            else if (ret != 0)
                return false;

            // Unserialize

            try {
                string strType;
                ssKey >> strType;
                if (strType == "blockindex" && !fRequestShutdown)
                {
                    Core::CDiskBlockIndex diskindex;
                    ssValue >> diskindex;

                    // Construct block index object
                    Core::CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
                    pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
                    pindexNew->pnext          = InsertBlockIndex(diskindex.hashNext);
                    pindexNew->nFile          = diskindex.nFile;
                    pindexNew->nBlockPos      = diskindex.nBlockPos;
                    pindexNew->nMint          = diskindex.nMint;
                    pindexNew->nMoneySupply   = diskindex.nMoneySupply;
                    pindexNew->nFlags         = diskindex.nFlags;
                    pindexNew->nVersion       = diskindex.nVersion;
                    pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                    pindexNew->nChannel       = diskindex.nChannel;
                    pindexNew->nHeight        = diskindex.nHeight;
                    pindexNew->nBits          = diskindex.nBits;
                    pindexNew->nNonce         = diskindex.nNonce;
                    pindexNew->nTime          = diskindex.nTime;

                    // Watch for genesis block
                    if (Core::pindexGenesisBlock == NULL && diskindex.GetBlockHash() == Core::hashGenesisBlock)
                        Core::pindexGenesisBlock = pindexNew;

                    if (!pindexNew->CheckIndex())
                        return error("LoadBlockIndex() : CheckIndex failed at %d", pindexNew->nHeight);

                }
                else
                {
                    break; // if shutdown requested or finished loading block index
                }
            }    // try
            catch (std::exception &e) {
                return error("%s() : deserialize error", __PRETTY_FUNCTION__);
            }
        }
        pcursor->close();

        if (fRequestShutdown)
            return true;

        /** Calculate the Chain Trust. **/
        if (!ReadHashBestChain(Core::hashBestChain))
        {
            if (Core::pindexGenesisBlock == NULL)
                return true;
            return error("CTxDB::LoadBlockIndex() : hashBestChain not loaded");
        }

        if (!Core::mapBlockIndex.count(Core::hashBestChain))
            return error("CTxDB::LoadBlockIndex() : hashBestChain not found in the block index");
        Core::pindexBest = Core::mapBlockIndex[Core::hashBestChain];
        Core::nBestHeight = Core::pindexBest->nHeight;

        Core::CBlockIndex* pindex = Core::pindexGenesisBlock;
        loop() {

            /** Get the Coinbase Transaction Rewards. **/
            if(pindex->pprev)
            {
                Core::CBlock block;
                if (!block.ReadFromDisk(pindex))
                    break;

                if(pindex->IsProofOfWork())
                {
                    unsigned int nSize = block.vtx[0].vout.size();
                    pindex->nCoinbaseRewards[0] = 0;
                    for(int nIndex = 0; nIndex < nSize - 2; nIndex++)
                        pindex->nCoinbaseRewards[0] += block.vtx[0].vout[nIndex].nValue;

                    pindex->nCoinbaseRewards[1] = block.vtx[0].vout[nSize - 2].nValue;
                    pindex->nCoinbaseRewards[2] = block.vtx[0].vout[nSize - 1].nValue;
                }

                //TODO: Trust key accept with no cBlock (CBlockIndex instead)
                else
                {
                    if(!Core::cTrustPool.Accept(block, true))
                        return error("CTxDB::LoadBlockIndex() : Failed To Accept Trust Key Block.");

                    if(!Core::cTrustPool.Connect(block, true))
                        return error("CTxDB::LoadBlockIndex() : Failed To Connect Trust Key Block.");
                }

                /** Grab the transactions for the block and set the address balances. **/
                if(GetBoolArg("-richlist", false))
                {
                    for(int nTx = 0; nTx < block.vtx.size(); nTx++)
                    {
                        for(int nOut = 0; nOut < block.vtx[nTx].vout.size(); nOut++)
                        {
                            Wallet::NexusAddress cAddress;
                            if(!ExtractAddress(block.vtx[nTx].vout[nOut].scriptPubKey, cAddress))
                                continue;

                            Core::mapAddressTransactions[cAddress.GetHash256()] += block.vtx[nTx].vout[nOut].nValue;

                            if(!Core::mapRichList.count(cAddress.GetHash256()))
                                Core::mapRichList[cAddress.GetHash256()] = { std::make_pair(block.vtx[nTx].IsCoinBase(), block.vtx[nTx].GetHash()) };
                            else
                                Core::mapRichList[cAddress.GetHash256()].push_back(std::make_pair(block.vtx[nTx].IsCoinBase(), block.vtx[nTx].GetHash()));
                        }

                        if(!block.vtx[nTx].IsCoinBase())
                        {
                            BOOST_FOREACH(const Core::CTxIn& txin, block.vtx[nTx].vin)
                            {
                                if(txin.IsStakeSig())
                                    continue;

                                Core::CTransaction tx;
                                Core::CTxIndex txind;

                                if(!ReadTxIndex(txin.prevout.hash, txind))
                                    continue;

                                if(!tx.ReadFromDisk(txind.pos))
                                    continue;

                                Wallet::NexusAddress cAddress;
                                if(!ExtractAddress(tx.vout[txin.prevout.n].scriptPubKey, cAddress))
                                    continue;

                                Core::mapAddressTransactions[cAddress.GetHash256()] -= tx.vout[txin.prevout.n].nValue;

                                if(!Core::mapRichList.count(cAddress.GetHash256()))
                                    Core::mapRichList[cAddress.GetHash256()] = { std::make_pair(block.vtx[nTx].IsCoinBase(), tx.GetHash()) };
                                else
                                    Core::mapRichList[cAddress.GetHash256()].push_back(std::make_pair(block.vtx[nTx].IsCoinBase(), tx.GetHash()));
                            }
                        }
                    }
                }
            }
            else
            {

                pindex->nCoinbaseRewards[0] = 0;
                pindex->nCoinbaseRewards[1] = 0;
                pindex->nCoinbaseRewards[2] = 0;
            }


            /** Calculate the Chain Trust. **/
            pindex->nChainTrust = (pindex->pprev ? pindex->pprev->nChainTrust : 0) + pindex->GetBlockTrust();


            /** Release the Nexus Rewards into the Blockchain. **/
            const Core::CBlockIndex* pindexPrev = GetLastChannelIndex(pindex->pprev, pindex->GetChannel());
            pindex->nChannelHeight = (pindexPrev ? pindexPrev->nChannelHeight : 0) + 1;


            /** Compute the Released Reserves. **/
            for(int nType = 0; nType < 3; nType++)
            {
                if(pindex->IsProofOfWork() && pindexPrev)
                {
                    int64 nReserve = GetReleasedReserve(pindex, pindex->GetChannel(), nType);
                    pindex->nReleasedReserve[nType] = pindexPrev->nReleasedReserve[nType] + nReserve - pindex->nCoinbaseRewards[nType];
                }
                else
                    pindex->nReleasedReserve[nType] = 0;

            }


            /** Add the Pending Checkpoint into the Blockchain. **/
            if(!pindex->pprev || Core::HardenCheckpoint(pindex, true))
                pindex->PendingCheckpoint = make_pair(pindex->nHeight, pindex->GetBlockHash());
            else
                pindex->PendingCheckpoint = pindex->pprev->PendingCheckpoint;

            /** Exit the Loop on the Best Block. **/
            if(pindex->GetBlockHash() == Core::hashBestChain)
            {
                printf("LoadBlockIndex(): hashBestChain=%s  height=%d  trust=%"PRIu64"\n", Core::hashBestChain.ToString().substr(0,20).c_str(), Core::nBestHeight, Core::nBestChainTrust);
                break;
            }


            pindex = pindex->pnext;
        }

        /** Verify the Blocks in the Best Chain To Last Checkpoint. **/
        int nCheckLevel = GetArg("-checklevel", 1);
        int nCheckDepth = GetArg( "-checkblocks", 10);
        if (nCheckDepth == 0)
            nCheckDepth = 1000000000;

        if (nCheckDepth > Core::nBestHeight)
            nCheckDepth = Core::nBestHeight;
        printf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
        Core::CBlockIndex* pindexFork = NULL;

        /* Allow Forking out an old chain. */
        unsigned int nFork = GetArg("-forkblocks", 0);
        if(nFork > 0 && Core::pindexBest)
        {
            pindexFork = Core::pindexBest;
            for(int nIndex = 0; nIndex < nFork; nIndex++)
            {
                if(!pindexFork->pprev)
                    break;

                pindexFork = pindexFork->pprev;
            }
        }


        map<pair<unsigned int, unsigned int>, Core::CBlockIndex*> mapBlockPos;
        for (Core::CBlockIndex* pindex = Core::pindexBest; pindex && pindex->pprev && nCheckDepth > 0; pindex = pindex->pprev)
        {
            if (pindex->nHeight < Core::nBestHeight - nCheckDepth)
                break;

            Core::CBlock block;
            if (!block.ReadFromDisk(pindex))
                return error("LoadBlockIndex() : block.ReadFromDisk failed");

            if (nCheckLevel > 0 && !block.CheckBlock())
            {
                printf("LoadBlockIndex() : *** found bad block at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str());
                pindexFork = pindex->pprev;
            }

            // check level 2: verify transaction index validity
            if (nCheckLevel>1)
            {
                pair<unsigned int, unsigned int> pos = make_pair(pindex->nFile, pindex->nBlockPos);
                mapBlockPos[pos] = pindex;
                BOOST_FOREACH(const Core::CTransaction &tx, block.vtx)
                {
                    uint512 hashTx = tx.GetHash();
                    Core::CTxIndex txindex;
                    if (ReadTxIndex(hashTx, txindex))
                    {
                        // check level 3: checker transaction hashes
                        if (nCheckLevel>2 || pindex->nFile != txindex.pos.nFile || pindex->nBlockPos != txindex.pos.nBlockPos)
                        {
                            // either an error or a duplicate transaction
                            Core::CTransaction txFound;
                            if (!txFound.ReadFromDisk(txindex.pos))
                            {
                                printf("LoadBlockIndex() : *** cannot read mislocated transaction %s\n", hashTx.ToString().c_str());
                                pindexFork = pindex->pprev;
                            }
                            else
                                if (txFound.GetHash() != hashTx) // not a duplicate tx
                                {
                                    printf("LoadBlockIndex(): *** invalid tx position for %s\n", hashTx.ToString().c_str());
                                    pindexFork = pindex->pprev;
                                }
                        }
                        // check level 4: check whether spent txouts were spent within the main chain
                        unsigned int nOutput = 0;
                        if (nCheckLevel>3)
                        {
                            BOOST_FOREACH(const Core::CDiskTxPos &txpos, txindex.vSpent)
                            {
                                if (!txpos.IsNull())
                                {
                                    pair<unsigned int, unsigned int> posFind = make_pair(txpos.nFile, txpos.nBlockPos);
                                    if (!mapBlockPos.count(posFind))
                                    {
                                        printf("LoadBlockIndex(): *** found bad spend at %d, hashBlock=%s, hashTx=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str(), hashTx.ToString().c_str());
                                        pindexFork = pindex->pprev;
                                    }
                                    // check level 6: check whether spent txouts were spent by a valid transaction that consume them
                                    if (nCheckLevel>5)
                                    {
                                        Core::CTransaction txSpend;
                                        if (!txSpend.ReadFromDisk(txpos))
                                        {
                                            printf("LoadBlockIndex(): *** cannot read spending transaction of %s:%i from disk\n", hashTx.ToString().c_str(), nOutput);
                                            pindexFork = pindex->pprev;
                                        }
                                        else if (!txSpend.CheckTransaction())
                                        {
                                            printf("LoadBlockIndex(): *** spending transaction of %s:%i is invalid\n", hashTx.ToString().c_str(), nOutput);
                                            pindexFork = pindex->pprev;
                                        }
                                        else
                                        {
                                            bool fFound = false;
                                            BOOST_FOREACH(const Core::CTxIn &txin, txSpend.vin)
                                                if (txin.prevout.hash == hashTx && txin.prevout.n == nOutput)
                                                    fFound = true;
                                            if (!fFound)
                                            {
                                                printf("LoadBlockIndex(): *** spending transaction of %s:%i does not spend it\n", hashTx.ToString().c_str(), nOutput);
                                                pindexFork = pindex->pprev;
                                            }
                                        }
                                    }
                                }
                                nOutput++;
                            }
                        }
                    }
                    // check level 5: check whether all prevouts are marked spent
                    if (nCheckLevel>4)
                    {
                        BOOST_FOREACH(const Core::CTxIn &txin, tx.vin)
                        {
                            Core::CTxIndex txindex;
                            if (ReadTxIndex(txin.prevout.hash, txindex))
                                if (txindex.vSpent.size()-1 < txin.prevout.n || txindex.vSpent[txin.prevout.n].IsNull())
                                {
                                    printf("LoadBlockIndex(): *** found unspent prevout %s:%i in %s\n", txin.prevout.hash.ToString().c_str(), txin.prevout.n, hashTx.ToString().c_str());
                                    pindexFork = pindex->pprev;
                                }
                        }
                    }
                }
            }
        }
        if (pindexFork)
        {
            // Reorg back to the fork
            printf("LoadBlockIndex() : *** moving best chain pointer back to block %d\n", pindexFork->nHeight);
            Core::CBlock block;
            if (!block.ReadFromDisk(pindexFork))
                return error("LoadBlockIndex() : block.ReadFromDisk failed");
            CIndexDB txdb;
            block.SetBestChain(txdb, pindexFork);
        }

        return true;
    }
#endif
}
