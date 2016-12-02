/*******************************************************************************************
 * 
 * "Doubt is poison, cast it out" - Videlicet
 * 
 * http://www.opensource.org/licenses/mit-license.php
 * 
 ******************************************************************************************/

#ifndef LOWER_LEVEL_LIBRARY_LLD_JOURNAL
#define LOWER_LEVEL_LIBRARY_LLD_JOURNAL

#include "sector.h"
#include "core.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
    /* Journal Database keeps in non-volatile memory the transaction record for rollback.
     * This will be triggered if the checksums don't match with sector data and the keychain 
     * TODO: Figure out the best Key system to correlate a transaction to the data in the journal
     * This should be seen by the sector as well, which means that the keychain should keep a list
     * of the keys that were changed.
     */
	class CJournalDB : public SectorDatabase
	{
	public:
		/** The Database Constructor. To determine file location and the Bytes per Record. **/
		CJournalDB(const char* pszMode="r+", std::string strID) : SectorDatabase(strID, strID, pszMode) {}
		
		/* TODO: Delete the Journal Database if it is deconstructed. 
         *       This should only happen when the database commits the transaction */
		~CJournalDB()
        {
            
        }
		
		bool AddTransaction(uint512 hash, Core::CTxIndex& txindex);
		bool RemoveTransaction(uint512 hash, const Core::CTxIndex& txindex);
	};
}

#endif
