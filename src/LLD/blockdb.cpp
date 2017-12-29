/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#include "include/blockdb.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
    
    /* Read the Block State Object from disk */
    bool CBlockDB::ReadBlockState(uint1024 hashBlock, Core::CBlockState& blkState)
    {
        //TODO:: Handle poolCache (if exists in cache return that)
        
        //ELSE
        return Read(hashBlock, blkState);
    }
    
    /* Write the block state object to disk through an LLD instance. */
    bool CBlockDB::WriteBlockState(Core::CBlockState blkState)
    {
        //TODO:: Handle poolCache (if not exists in pool cache set to write state)
        
        //ELSE
        return Write(blkState.GetHash(), blkState);
    }
    
    /* Helper function to return just a CBlock from a CBlockState written in Database. */
    bool CBlockDB::ReadBlock(uint1024 hashBlock, Core::CBlock& blk)
    {
        //TODO:: Handle poolCache (if exists in cache return that)
        
        //ELSE
        Core::CBlockState blkState;
        if(!Read(hashBlock, blkState))
            return false;
        
        blk = blkState.GetBlock();
        return true;
    }
}
