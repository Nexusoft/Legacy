#ifndef LOWER_LEVEL_LIBRARY_LLD_TRANSACTION
#define LOWER_LEVEL_LIBRARY_LLD_TRANSACTION

#include <boost/thread.hpp>
#include <map>
#include <vector>

#define MUTEX_LOCK(a) boost::lock_guard<boost::mutex> lock(a)

/** Lower Level Database Name Space. **/
namespace LLD
{
		/** ACID Transactions:
	
		Atomicity - All or nothing. The full transaction must complete. Transaction Record in Memory is held until commited to disk. If any part fails it needs
		To Roll Back to all the original data.
		I.   If any transaction fails, roll back to original data
		II.  This can happen if it fails due to a power failure or system or application crash
		III. When rebooted the LLD sector database will check its instance for its original data.
		
		Consistency - The database must be brought from one valid state to the next. If any of the transaction sequences fail due to a violation of the rules
		of the database, the whole transaction must fail.
		I.  SectorTransaction class to handle the storing of all sequences of the transaction.
		II. If the commit fails, and the system is still operating, roll back all the transaction data to original states.
		
		Isolation -All Transactions must execute in the sequence in which they were created in order to have the data changed in the order it was flagged
		To change.
		I. Use a Vector storing the order with a pair of byte vectors for the data type.
		II. Retain this ordering while commiting the transaction to disk.
		
		Durability - The data in a transaction must be commited to non volatile memory. This will make sure that the transaction will retain independent of a crash
		or power loss even in the middle of the transaction. This is done in three ways:
		I. Flagging of the keychain to set the state of the sector as part of a transaction sequence. 
		II. The data checksum is stored in the keychain that defines the state of the sector data to ensure that there was not a crash
		or power loss in the middle of the database write. 
		II.Return of the sector keys to a valid state stating that the sectors were written successfully. This will only happen after the sector has been written.
		
		If there is a power loss before the transaction successfully commits, the original data is backed up in a separate temporary keychain/sector database 
		so that on reboot of the node or database, it checks the data for consistency. This will roll back to original data if any of the transaction sequences fail.
		
		Use a checksum as the transaction original key. If the data fails to read due to invalid state that was never reset, search the database for the original data.
		
	**/
	
	/** Transaction Class to hold the data that is stored in Binary. **/
	class SectorTransaction
	{
	public:
	
		/** Only let one operation happen on the transaction at one time. **/
		boost::mutex TX_MUTEX;
		
		/** New Data to be Added. **/
		std::map< std::vector<unsigned char>, std::vector<unsigned char> > mapTransactions;
		
		/** Original Data that is retained when new one is added. **/
		std::map< std::vector<unsigned char>, std::vector<unsigned char> > mapOriginalData;
		
		/** Vector to hold the keys of transactions to be erased. **/
		std::map< std::vector<unsigned char>, unsigned int > mapEraseData;
        
        /** Flag for Transaction. **/
        bool fCommit = false;
		
		/** Basic Constructor. **/
		SectorTransaction(){ }
		
		/** Add a new Transaction to the Memory Map. **/
		bool AddTransaction(std::vector<unsigned char> vKey, std::vector<unsigned char> vData,
							std::vector<unsigned char> vOriginalData)
		{
			MUTEX_LOCK(TX_MUTEX);
			
			mapTransactions[vKey] = vData;
			mapOriginalData[vKey] = vOriginalData;
            
            if(mapEraseData.count(vKey))
                mapEraseData.erase(vKey);
			
			return true;
		}
		
		/** Function to Erase a Key from the Keychain. **/
		bool EraseTransaction(std::vector<unsigned char> vKey)
		{
			MUTEX_LOCK(TX_MUTEX);
			
			mapEraseData[vKey] = 0;
			if(mapTransactions.count(vKey))
				mapTransactions.erase(vKey);
			
			if(mapOriginalData.count(vKey))
				mapOriginalData.erase(vKey);
			
			return true;
		}	
		
		//TODO: Add Journal Flush to Disk Here
		//OF the following serialization format
		/* ------> Start Header
         * unsigned int nTotalRecords;
         * 
         * ------> Start Records
         * unsigned short nKeyLength;
         * std::vector<unsigned char> vKey;
         * 
         * unsigned short nDataLength
         * std::vector<unsigned char> vData;
         * 
         * unsigned short nOriginalLength
         * std::vector<unsigned char> vOriginalData;
         * ------> Next Record in Sequence
         */
	};
}

#endif
