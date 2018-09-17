/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "core.h"
#include "../hash/uint1024.h"
#include "../wallet/db.h"

namespace Core
{
    /** Hardened Checkpoints. **/
    std::map<unsigned int, uint1024> mapCheckpoints;
    unsigned int CHECKPOINT_TIMESPAN = 30, MAX_CHECKPOINTS_SEARCH = 1;


    /** Check Checkpoint Timespan. **/
    bool IsNewTimespan(CBlockIndex* pindex)
    {
        if(mapCheckpoints.empty() || !pindex->pprev)
            return true;

        int nFirstMinutes = floor((pindex->GetBlockTime() - mapBlockIndex[pindex->PendingCheckpoint.second]->GetBlockTime()) / 60.0);
        int nLastMinutes =  floor((pindex->pprev->GetBlockTime() - mapBlockIndex[pindex->PendingCheckpoint.second]->GetBlockTime()) / 60.0);

        return (nFirstMinutes != nLastMinutes && nFirstMinutes >= CHECKPOINT_TIMESPAN);
    }


    /** Checks whether given block index is a descendant of last hardened checkpoint. **/
    bool IsDescendant(CBlockIndex* pindex)
    {
        if(mapCheckpoints.empty() || pindex->nHeight <= 1)
            return true;

        /** Ensure that the block is made after last hardened Checkpoint. **/
        unsigned int nTotalCheckpoints = 0;

        /** Check The Block Hash **/
        while(pindex && pindex->pprev)
        {
            if(mapCheckpoints.count(pindex->pprev->nHeight))
            {
                if(pindex->pprev->GetBlockHash() == mapCheckpoints[pindex->pprev->nHeight])
                    return true;

                if(nTotalCheckpoints >= MAX_CHECKPOINTS_SEARCH)
                    return false;

                nTotalCheckpoints ++;
            }

            pindex = pindex->pprev;
        }

        return false;
    }


    /** Hardens the Pending Checkpoint on the Blockchain, determined by a new block creating a new Timespan.
        The blockchain from genesis to new hardened checkpoint will then be fixed into place. **/
    bool HardenCheckpoint(CBlockIndex* pcheckpoint, bool fInit)
    {

        /** Only Harden New Checkpoint if it Fits new Timestamp. **/
        if(!IsNewTimespan(pcheckpoint->pprev))
            return false;


        /** Only Harden a New Checkpoint if it isn't already hardened. **/
        if(mapCheckpoints.count(pcheckpoint->pprev->PendingCheckpoint.first))
            return true;


        /** Update the Checkpoints into Memory. **/
        mapCheckpoints[pcheckpoint->pprev->PendingCheckpoint.first] = pcheckpoint->pprev->PendingCheckpoint.second;


        /** Dump the Checkpoint if not Initializing. **/
        if(!fInit)
            printg("===== Hardened Checkpoint %s Height = %u\n",
            pcheckpoint->pprev->PendingCheckpoint.second.ToString().substr(0, 20).c_str(),
            pcheckpoint->pprev->PendingCheckpoint.first);

        return true;
    }
}
