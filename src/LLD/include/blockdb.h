/*__________________________________________________________________________________________
 
 (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
 
 (c) Copyright The Nexus Developers 2014 - 2017
 
 Distributed under the MIT software license, see the accompanying
 file COPYING or http://www.opensource.org/licenses/mit-license.php.
 
 "fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
 
 ____________________________________________________________________________________________*/

#ifndef NEXUS_LLD_INCLUDE_BLOCKS_H
#define NEXUS_LLD_INCLUDE_BLOCKS_H

#include "../templates/key.h"
#include "../templates/sector.h"
#include "../../Core/types/include/block.h"

namespace LLD
{
    
    //TODO: Combine this in a correlated LLD instance with LLP::CHoldingPool as the cache
    //allows the block to exist in memory or disk and the LLD to handle the switching
    class CBlockDB : public SectorDatabase
    {
        //LLP::CHoldingPool<uint1024, CBlockState> poolCache;
        
    public:
        /* The Database Constructor. To determine file location and the Bytes per Record. */
        CBlockDB(const char* pszMode="r+") : SectorDatabase("blocks-", "blocks-", pszMode) {}
        
        /* Read the Block State Object from disk */
        bool ReadBlockState(uint1024 hashBlock, Core::CBlockState& blkState);
        
        /* Write the block state object to disk through an LLD instance. */
        bool WriteBlockState(Core::CBlockState blkState);
        
        /* Helper function to return just a CBlock from a CBlockState written in Database. */
        bool ReadBlock(uint1024 hashBlock, Core::CBlock& blk);
    };
}

#endif
