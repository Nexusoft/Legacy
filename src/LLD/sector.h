#ifndef LOWER_LEVEL_LIBRARY_LLD_SECTOR
#define LOWER_LEVEL_LIBRARY_LLD_SECTOR

#include "keychain.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
	
	
	/** Transactino Class to hold the data that is stored in Binary. **/
	class SectorTransaction
	{
	public:
	
		/** The hash for the Transaction to be saved under. **/
		uint64 TransactionID;
		
		/** Only let one operation happen on the transaction at one time. **/
		boost::mutex TX_MUTEX;
		
		/** New Data to be Added. **/
		std::map< std::vector<unsigned char>, std::vector<unsigned char> > mapTransactions;
		
		/** Original Data that is retained when new one is added. **/
		std::map< std::vector<unsigned char>, std::vector<unsigned char> > mapOriginalData;
		
		/** Vector to hold the keys of transactions to be erased. **/
		std::map< std::vector<unsigned char>, unsigned int > mapEraseData;
		
		/** Basic Constructor. **/
		SectorTransaction(){ }
		
		/** Add a new Transaction to the Memory Map. **/
		bool AddTransaction(std::vector<unsigned char> vKey, std::vector<unsigned char> vData,
							std::vector<unsigned char> vOriginalData)
		{
			MUTEX_LOCK(TX_MUTEX);
			
			if(mapEraseData.count(vKey))
				return false;
			
			mapTransactions[vKey] = vData;
			//mapOriginalData[vKey] = vOriginalData;
			
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
		
	};

	/** Base Template Class for a Sector Database. 
		Processes main Lower Level Disk Communications.
		A Sector Database Is a Fixed Width Data Storage Medium.
		
		It is ideal for data structures to be stored that do not
		change in their size. This allows the greatest efficiency
		in fixed data storage (structs, class, etc.).
		
		It is not ideal for data structures that may vary in size
		over their lifetimes. The Dynamic Database will allow that.
		
		Key Type can be of any type. Data lengths are attributed to
		each key type. Keys are assigned sectors and stored in the
		key storage file. Sector files are broken into maximum of 1 GB
		for stability on all systems, key files are determined the same.
		
		Multiple Keys can point back to the same sector to allow multiple
		access levels of the sector. This specific class handles the lower
		level disk communications for the sector database.
		
		If each sector was allowed to be varying sizes it would remove the
		ability to use free space that becomes available upon an erase of a
		record. Use this Database purely for fixed size structures. Overflow
		attempts will trigger an error code.
		
		TODO:: Add Transactions for Database
		
		TransactionStart();
		TransactionCommit();
		TransactionAbort();
		
		A. Use a Memory map of Keys and Value Pairs to start Transaction.
		B. Use previous memory map to keep the original Keys and Value pairs.
		
		C. Once Transaction is Commited, iterate the memory map and write each one to disk.
		D. If there is any error in the writing across these, original key and value pairs needs to be rewritten.
		
		TransactionStart() -> Erase both Memory maps.
		TransactionCommit() -> Do the Writing. Erase map on success.
		TransactionAbort() -> Erase both Memeory maps.
		
		
		TODO:: Add in the Database File Searching from Sector Keys. Allow Multiple Files.
		
	**/
	class SectorDatabase
	{
	protected:
		/** Mutex for Thread Synchronization. 
			TODO: Lock Mutex based on Read / Writes on a per Sector Basis. 
			Will allow higher efficiency for thread concurrency. **/
		boost::mutex SECTOR_MUTEX;
		
		/** The String to hold the Disk Location of Database File. 
			Each Database File Acts as a New Table as in Conventional Design.
			Key can be any Type, which is how the Database Records are Accessed. **/
		std::string strBaseName;
		
		
		/** Location of the Directory to host Database File System. 
			Main File Components are Derived from Base Name.
			Contains Key and Cache Databases. **/
		std::string strBaseLocation;
		
		/** Keychain Registry:
			The nameof the Keychain Registry. **/
		std::string strKeychainRegistry;
		
		/** Memory Structure to Track the Database Sizes. **/
		std::vector<unsigned int> vDatabaseSizes;
		
		/** Read only Flag for Sectors. **/
		bool fReadOnly = false;
		
		/** Class to handle Transaction Data. **/
		SectorTransaction* pTransaction;
	public:
		/** The Database Constructor. To determine file location and the Bytes per Record. **/
		SectorDatabase(std::string strName, std::string strKeychain, const char* pszMode="r+")
		{
			strKeychainRegistry = strKeychain;
			strBaseLocation = GetDefaultDataDir().string() + "\\datachain\\"; 
			strBaseName = strName;
			
			/** Read only flag when instantiating new database. **/
			fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
			
			Initialize();
		}
		~SectorDatabase() { }
		
		/** Initialize Sector Database. **/
		void Initialize()
		{
			/** Create the Sector Database Directories. **/
			boost::filesystem::path dir(strBaseLocation);
			boost::filesystem::create_directory(dir);
			
			/** TODO: Make a worker or thread to check sizes of files and automatically create new file.
				Keep independent of reads and writes for efficiency. **/
			std::fstream fIncoming(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), 0).c_str(), std::ios::in | std::ios::binary);
			if(!fIncoming) {
				std::ofstream cStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), 0).c_str(), std::ios::binary);
				cStream.close();
			}
			
			pTransaction = NULL;
		}
		
		template<typename Key>
		bool Exists(const Key& key)
		{
			/** Serialize Key into Bytes. **/
			CDataStream ssKey(SER_LLD, DATABASE_VERSION);
			ssKey.reserve(GetSerializeSize(key, SER_LLD, DATABASE_VERSION));
			ssKey << key;
			std::vector<unsigned char> vKey(ssKey.begin(), ssKey.end());
			
			/** Read a Record from Binary Data. **/
			KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
			if(!SectorKeys)
				return error("Exists() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());
			
			/** Return the Key existance in the Keychain Database. **/
			return SectorKeys->HasKey(vKey);
		}
		
		template<typename Key>
		bool Erase(const Key& key)
		{
			/** Serialize Key into Bytes. **/
			CDataStream ssKey(SER_LLD, DATABASE_VERSION);
			ssKey.reserve(GetSerializeSize(key, SER_LLD, DATABASE_VERSION));
			ssKey << key;
			std::vector<unsigned char> vKey(ssKey.begin(), ssKey.end());
			
			if(pTransaction){
				pTransaction->EraseTransaction(vKey);
				
				return true;
			}
		
			/** Read a Record from Binary Data. **/
			KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
			if(!SectorKeys)
				return error("Erase() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());
			
			/** Return the Key existance in the Keychain Database. **/
			return SectorKeys->Erase(vKey);
		}
		
		template<typename Key, typename Type>
		bool Read(const Key& key, Type& value)
		{
			/** Serialize Key into Bytes. **/
			CDataStream ssKey(SER_LLD, DATABASE_VERSION);
			ssKey.reserve(GetSerializeSize(key, SER_LLD, DATABASE_VERSION));
			ssKey << key;
			std::vector<unsigned char> vKey(ssKey.begin(), ssKey.end());
			
			/** Get the Data from Sector Database. **/
			std::vector<unsigned char> vData;
			if(!Get(vKey, vData))
				return false;

			/** Deserialize Value. **/
			try {
				CDataStream ssValue(vData, SER_LLD, DATABASE_VERSION);
				ssValue >> value;
			}
			catch (std::exception &e) {
				return false;
			}

			return true;
		}

		template<typename Key, typename Type>
		bool Write(const Key& key, const Type& value)
		{
			if (fReadOnly)
				assert(!"Write called on database in read-only mode");

			/** Serialize the Key. **/
			CDataStream ssKey(SER_LLD, DATABASE_VERSION);
			ssKey.reserve(GetSerializeSize(key, SER_LLD, DATABASE_VERSION));
			ssKey << key;
			std::vector<unsigned char> vKey(ssKey.begin(), ssKey.end());

			/** Serialize the Value **/
			CDataStream ssValue(SER_LLD, DATABASE_VERSION);
			ssValue.reserve(value.GetSerializeSize(SER_LLD, DATABASE_VERSION));
			ssValue << value;
			std::vector<unsigned char> vData(ssValue.begin(), ssValue.end());

			/** Commit to the Database. **/
			if(pTransaction)
			{
				if(fDebug)
					printf("Write() : Adding Transaction to Sector Chains.\n");
				
				std::vector<unsigned char> vOriginalData;
				//Get(vKey, vOriginalData);
				
				return pTransaction->AddTransaction(vKey, vData, vOriginalData);
			}
			
			return Put(vKey, vData);
		}
		
		/** Get a Record from the Database with Given Key. **/
		bool Get(std::vector<unsigned char> vKey, std::vector<unsigned char>& vData)
		{
			MUTEX_LOCK(SECTOR_MUTEX);
			
			/** Read a Record from Binary Data. **/
			KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
			if(!SectorKeys)
				return error("Get() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());
			
			if(SectorKeys->HasKey(vKey))
			{	
				/** Read the Sector Key from Keychain. **/
				SectorKey cKey;
				if(!SectorKeys->Get(vKey, cKey))
					return false;
				
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), cKey.nSectorFile).c_str(), std::ios::in | std::ios::binary);

				/** Seek to the Sector Position on Disk. **/
				fStream.seekg(cKey.nSectorStart);
			
				/** Read the State and Size of Sector Header. **/
				vData.resize(cKey.nSectorSize);
				fStream.read((char*) &vData[0], vData.size());
				fStream.close();
				
				/** Check the Data Integrity of the Sector by comparing the Checksums. **/
				uint64 nChecksum = SK64(vData);
				if(cKey.nChecksum != nChecksum)
					return error("Sector Get() : Checksums don't match data. Corrupted Sector.");
				
				if(GetArg("-verbose", 0) >= 3)
					printf("SECTOR GET:%s\n", HexStr(vData.begin(), vData.end()).c_str());
				
				return true;
			}
			
			return false;
		}
		
		
		/** Add / Update A Record in the Database **/
		bool Put(std::vector<unsigned char> vKey, std::vector<unsigned char> vData)
		{
			MUTEX_LOCK(SECTOR_MUTEX);
			
			KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
			if(!SectorKeys)
				return error("Put() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());
			
			/** Write Header if First Update. **/
			if(!SectorKeys->HasKey(vKey))
			{
				/** TODO:: Assign a Sector File based on Database Sizes. **/
				unsigned short nSectorFile = 0;
				
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
				
				/** Create a new Sector Key. **/
				SectorKey cKey(WRITE, vKey, nSectorFile, 0, vData.size()); 
				
				/** Assign the Key to Keychain. **/
				SectorKeys->Put(cKey);
				
				/** If it is a New Sector, Assign a Binary Position. 
					TODO: Track Sector Database File Sizes. **/
				unsigned int nBegin = fStream.tellg();
				fStream.seekg (0, std::ios::end);
				
				unsigned int nStart = (unsigned int) fStream.tellg() - nBegin;
				fStream.seekp(nStart, std::ios::beg);
				
				fStream.write((char*) &vData[0], vData.size());
				fStream.close();
				
				/** Assign New Data to the Sector Key. **/
				cKey.nState       = READY;
				cKey.nSectorSize  = vData.size();
				cKey.nSectorStart = nStart;
				
				/** Check the Data Integrity of the Sector by comparing the Checksums. **/
				cKey.nChecksum    = SK64(vData);
				
				/** Assign the Key to Keychain. **/
				SectorKeys->Put(cKey);
			}
			else
			{
				/** Get the Sector Key from the Keychain. **/
				SectorKey cKey;
				if(!SectorKeys->Get(vKey, cKey)) {
					SectorKeys->Erase(vKey);
					
					return false;
				}
					
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), cKey.nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
				
				/** Locate the Sector Data from Sector Key. 
					TODO: Make Paging more Efficient in Keys by breaking data into different locations in Database. **/
				fStream.seekp(cKey.nSectorStart, std::ios::beg);
				if(vData.size() > cKey.nSectorSize){
					fStream.close();
					printf("ERROR PUT (TOO LARGE) NO TRUNCATING ALLOWED (Old %u :: New %u):%s\n", cKey.nSectorSize, vData.size(), HexStr(vData.begin(), vData.end()).c_str());
					
					return false;
				}
				
				/** Assign the Writing State for Sector. **/
				cKey.nState = WRITE;
				SectorKeys->Put(cKey);
				
				fStream.write((char*) &vData[0], vData.size());
				fStream.close();
				
				cKey.nState    = READY;
				cKey.nChecksum = SK64(vData);
				
				SectorKeys->Put(cKey);
			}
			
			if(GetArg("-verbose", 0) >= 3)
				printf("SECTOR PUT:%s\n", HexStr(vData.begin(), vData.end()).c_str());
			
			return true;
		}
		
		/** Start a New Database Transaction. 
			This will put all the database changes into pending state.
			If any of the database updates fail in procewss it will roll the database back to its previous state. **/
		void TransactionStart()
		{
			/** Delete a previous database transaction pointer if applicable. **/
			if(pTransaction)
				delete pTransaction;
			
			/** Create the new Database Transaction Object. **/
			pTransaction = new SectorTransaction();
			
			if(GetArg("-verbose", 0) >= 4)
				printf("TransactionStart() : New Sector Transaction Started.\n");
		}
		
		/** Abort the current transaction that is pending in the transaction chain. **/
		void TransactionAbort()
		{
			/** Delete the previous transaction pointer if applicable. **/
			if(pTransaction)
				delete pTransaction;
			
			/** Set the transaction pointer to null also acting like a flag **/
			pTransaction = NULL;
		}
		
		/** Return the database state back to its original state before transactions are commited. **/
		bool RollbackTransactions()
		{
				/** Iterate the original data memory map to reset the database to its previous state. **/
			for(typename std::map< std::vector<unsigned char>, std::vector<unsigned char> >::iterator nIterator = pTransaction->mapOriginalData.begin(); nIterator != pTransaction->mapOriginalData.end(); nIterator++ )
				if(!Put(nIterator->first, nIterator->second))
					return false;
				
			return true;
		}
		
		/** Commit the Data in the Transaction Object to the Database Disk.
			TODO: Handle the Transaction Rollbacks with a new Transaction Keychain and Sector Database. 
			Make it temporary and named after the unique identity of the sector database. 
			Fingerprint is SK64 hash of unified time and the sector database name along with some other data 
			To be determined... **/
		bool TransactionCommit()
		{
			MUTEX_LOCK(SECTOR_MUTEX);
			
			if(GetArg("-verbose", 0) >= 4)
				printf("TransactionCommit() : Commiting Transactin to Datachain.\n");
			
			/** Check that there is a valid transaction to apply to the database. **/
			if(!pTransaction)
				return error("TransactionCommit() : No Transaction data to Commit.");
			
			/** Get the Keychain from the Sector Database. **/
			KeyDatabase* SectorKeys = GetKeychain(strKeychainRegistry);
			if(!SectorKeys)
				return error("TransactionCommit() : Sector Keys not Registered for Name %s\n", strKeychainRegistry.c_str());
			
			/** Habdle setting the sector key flags so the database knows if the transaction was completed properly. **/
			if(GetArg("-verbose", 0) >= 4)
				printf("TransactionCommit() : Commiting Keys to Keychain.\n");
			
			for(typename std::map< std::vector<unsigned char>, std::vector<unsigned char> >::iterator nIterator = pTransaction->mapTransactions.begin(); nIterator != pTransaction->mapTransactions.end(); nIterator++ )
			{
				SectorKey cKey;
				if(SectorKeys->HasKey(nIterator->first)) {
					if(!SectorKeys->Get(nIterator->first, cKey))
						return error("CommitTransaction() : Couldn't get the Active Sector Key.");
					
					cKey.nState = TRANSACTION;
					SectorKeys->Put(cKey);
				}
			}
			
			
			/** Commit the Sector Data to the Database. **/
			if(GetArg("-verbose", 0) >= 4)
				printf("TransactionCommit() : Commit Data to Datachain Sector Database.\n");
			
			for(typename std::map< std::vector<unsigned char>, std::vector<unsigned char> >::iterator nIterator = pTransaction->mapTransactions.begin(); nIterator != pTransaction->mapTransactions.end(); nIterator++ )
			{
				/** Declare the Key and Data for easier reference. **/
				std::vector<unsigned char> vKey  = nIterator->first;
				std::vector<unsigned char> vData = nIterator->second;
				
				/** Write Header if First Update. **/
				if(!SectorKeys->HasKey(vKey))
				{
					/** TODO:: Assign a Sector File based on Database Sizes. **/
					unsigned short nSectorFile = 0;
					
					/** Open the Stream to Read the data from Sector on File. **/
					std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
					
					/** Create a new Sector Key. **/
					SectorKey cKey(TRANSACTION, vKey, nSectorFile, 0, vData.size());
					
					/** If it is a New Sector, Assign a Binary Position. 
						TODO: Track Sector Database File Sizes. **/
					unsigned int nBegin = fStream.tellg();
					fStream.seekg (0, std::ios::end);
					
					/** Find the Binary Starting Location. **/
					unsigned int nStart = (unsigned int) fStream.tellg() - nBegin;
					
					/** Assign the Key to Keychain. **/
					cKey.nSectorStart = nStart;
					SectorKeys->Put(cKey);
					
					/** Write the Data to the Sector. **/
					fStream.seekp(nStart, std::ios::beg);
					fStream.write((char*) &vData[0], vData.size());
					fStream.close();
				}
				else
				{
					/** Get the Sector Key from the Keychain. **/
					SectorKey cKey;
					if(!SectorKeys->Get(vKey, cKey)) {
						SectorKeys->Erase(vKey);
						
						return false;
					}
						
					/** Open the Stream to Read the data from Sector on File. **/
					std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), cKey.nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
					
					/** Locate the Sector Data from Sector Key. 
						TODO: Make Paging more Efficient in Keys by breaking data into different locations in Database. **/
					fStream.seekp(cKey.nSectorStart, std::ios::beg);
					if(vData.size() > cKey.nSectorSize){
						fStream.close();
						printf("ERROR PUT (TOO LARGE) NO TRUNCATING ALLOWED (Old %u :: New %u):%s\n", cKey.nSectorSize, vData.size(), HexStr(vData.begin(), vData.end()).c_str());
						
						return false;
					}
					
					/** Assign the Writing State for Sector. **/
					fStream.write((char*) &vData[0], vData.size());
					fStream.close();
				}
			}
			
			/** Update the Keychain with Checksums and READY Flag letting sectors know they were written successfully. **/
			if(GetArg("-verbose", 0) >= 4)
				printf("TransactionCommit() : Commiting Key Valid States to Keychain.\n");
			
			for(typename std::map< std::vector<unsigned char>, std::vector<unsigned char> >::iterator nIterator = pTransaction->mapTransactions.begin(); nIterator != pTransaction->mapTransactions.end(); nIterator++ )
			{
				/** Assign the Writing State for Sector. **/
				SectorKey cKey;
				if(!SectorKeys->Get(nIterator->first, cKey))
					return error("CommitTransaction() : Failed to Get Key from Keychain.");
				
				/** Set the Sector states back to Active. **/
				cKey.nState    = READY;
				cKey.nChecksum = SK64(nIterator->second);
				
				/** Commit the Keys to Keychain Database. **/
				if(!SectorKeys->Put(cKey))
					return error("CommitTransaction() : Failed to Commit Key to Keychain.");
			}
			
			/** Clean up the Sector Transaction Key. 
				TODO: Delete the Sector and Keychain for Current Transaction Commit ID. **/
			delete pTransaction;
			pTransaction = NULL;
			
			return true;
		}
	};
}

#endif