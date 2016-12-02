/*******************************************************************************************
 * 
 * "Senses fail to bind what you see" - Videlicet
 * 
 * http://www.opensource.org/licenses/mit-license.php
 * 
 ******************************************************************************************/

#ifndef LOWER_LEVEL_LIBRARY_LLD_SECTOR_KEY
#define LOWER_LEVEL_LIBRARY_LLD_SECTOR_KEY

#include "types.h"

/** Lower Level Database Name Space. **/
namespace LLD
{

	/** Base Key Database Class.
		Stores and Contains the Sector Keys to Access the
		Sector Database.
	**/
	class KeyDatabase
	{
	protected:
		/* Mutex for Thread Synchronization. */
		mutable boost::mutex KEY_MUTEX;
		
		/* The String to hold the Disk Location of Database File. 
			Each Database File Acts as a New Table as in Conventional Design.
			Key can be any Type, which is how the Database Records are Accessed. */
		std::string strBaseLocation;
		std::string strDatabaseName;
		std::string strLocation;
		
	public:	
		/** Map to Contain the Binary Positions of Each Key.
			Used to Quickly Read the Database File at Given Position
			To Obtain the Record from its Database Key. This is Read
			Into Memory on Database Initialization. **/
		mutable typename std::map< std::vector<unsigned char>, unsigned int > mapKeys;
		
		/** Caching Memory Map. Keep within caching limits. Pop on and off like a stack
			using seperate caching class if over limits. **/
		mutable typename std::map< std::vector<unsigned char>, SectorKey > mapKeysCache;
		
		/** The Database Constructor. To determine file location and the Bytes per Record. **/
		KeyDatabase(std::string strBaseLocationIn, std::string strDatabaseNameIn) : strDatabaseName(strDatabaseNameIn), strBaseLocation(strBaseLocationIn) { strLocation = strBaseLocation + strDatabaseName + ".keys"; }
		
		/** Clean up Memory Usage. **/
		~KeyDatabase() {
			mapKeys.clear();
		}
		
		/** Return the Keys to the Records Held in the Database. **/
		std::vector< std::vector<unsigned char> > GetKeys()
		{
			std::vector< std::vector<unsigned char> > vKeys;
			for(typename std::map< std::vector<unsigned char>, unsigned int >::iterator nIterator = mapKeys.begin(); nIterator != mapKeys.end(); nIterator++ )
				vKeys.push_back(nIterator->first);
				
			return vKeys;
		}
		
		/** Return Whether a Key Exists in the Database. **/
		bool HasKey(std::vector<unsigned char> vKey) { return mapKeys.count(vKey); }
		
		
		/** Read the Database Keys and File Positions. **/
		void Initialize()
		{
			MUTEX_LOCK(KEY_MUTEX);
			
			/** Open the Stream to Read from the File Stream. **/
			std::fstream fIncoming(strLocation.c_str(), std::ios::in | std::ios::binary);
			if(!fIncoming)
			{
				/** Create the Sector Database Directories. **/
				boost::filesystem::path dir(strBaseLocation);
				boost::filesystem::create_directory(dir);
			
				std::ofstream cStream(strLocation.c_str(), std::ios::binary);
				cStream.close();
					
				return;
			}
			
			/* Basic Debug Information. */
			printf("[DATABASE] Initializing %s Keychain....\n", strDatabaseName.c_str());
			
			/** Get the Binary Size. **/
			fIncoming.seekg (0, std::ios::end);
			unsigned int nBinarySize = fIncoming.tellg();
			
			/** Iterator for Key Sectors. **/
			fIncoming.seekg (0, std::ios::beg);
			while(fIncoming.tellg() < nBinarySize)
			{
				/** Get the current file pointer position. **/
				unsigned int nPosition = fIncoming.tellg();
				
				std::vector<unsigned char> vHeader(3, 0);
				fIncoming.read((char*) &vHeader[0], vHeader.size());
				
				SectorKey cKey;
				CDataStream ssKey(vHeader, SER_LLD_KEY_HEADER, DATABASE_VERSION);
				ssKey >> cKey;
				
				
				/** Skip Empty Sectors for Now. 
					TODO: Handle any sector and keys gracfully here to ensure that the Sector is returned to a valid state from the transaction journal in case there was a failure reading and writing in the sector. This will most likely be held in the sector database code. **/
				if(cKey.Ready()) {
				
					/** Read the Key Data. **/
					std::vector<unsigned char> vKey(cKey.nLength, 0);
					fIncoming.seekg(nPosition + cKey.Size());
					fIncoming.read((char*) &vKey[0], vKey.size());
					
					/** Set the Key Data. **/
					mapKeys[vKey] = nPosition;
					
					/** Debug Output of Sector Key Information. **/
					if(GetArg("-verbose", 0) >= 5)
						printf("KeyDB::Load() : State: %u Length: %u Location: %u Key: %s\n", cKey.nState, cKey.nLength, mapKeys[vKey], HexStr(vKey.begin(), vKey.end()).c_str());
				}
				else {
					/** Move the Cursor to the Next Record. **/
					fIncoming.seekg(nPosition + cKey.Size(), std::ios::beg);
                    
                    /** Debug Output of Sector Key Information. **/
					if(GetArg("-verbose", 0) >= 3)
						printf("KeyDB::Load() : Skiping Sector State: %u Length: %u\n", cKey.nState, cKey.nLength);
                }
			}
			
			fIncoming.close();
		}
		
		/** Add / Update A Record in the Database **/
		bool Put(SectorKey cKey) const
		{
			MUTEX_LOCK(KEY_MUTEX);
			
			/** Establish the Outgoing Stream. **/
			std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			
			/** Write Header if First Update. **/
			if(!mapKeys.count(cKey.vKey))
			{
				/** If it is a New Sector, Assign a Binary Position. **/
				unsigned int nBegin = fStream.tellg();
				fStream.seekg (0, std::ios::end);

				mapKeys[cKey.vKey] = (unsigned int) fStream.tellg() - nBegin;
			}
			
			/** Seek File Pointer **/
			fStream.seekp(mapKeys[cKey.vKey], std::ios::beg);
				
			/** Handle the Sector Key Serialization. **/
			CDataStream ssKey(SER_LLD, DATABASE_VERSION);
			ssKey.reserve(cKey.Size());
			ssKey << cKey;
				
			/** Write to Disk. **/
			std::vector<unsigned char> vData(ssKey.begin(), ssKey.end());
			vData.insert(vData.end(), cKey.vKey.begin(), cKey.vKey.end());
			fStream.write((char*) &vData[0], vData.size());
			fStream.close();
				
			/** Debug Output of Sector Key Information. **/
			if(GetArg("-verbose", 0) >= 4)
				printf("KEY::Put(): State: %s | Length: %u | Location: %u | Sector File: %u | Sector Size: %u | Sector Start: %u\n Key: %s\n", cKey.nState == READY ? "Valid" : "Invalid", cKey.nLength, mapKeys[cKey.vKey], cKey.nSectorFile, cKey.nSectorSize, cKey.nSectorStart, HexStr(cKey.vKey.begin(), cKey.vKey.end()).c_str());
			
			return true;
		}
		
		/* Erase Functionality as follows TODO:
		 * 
		 *  1. Flag the Sector Key as EMPTY
		 *  2. Add the Sector Key binary position in a stored memory mapKeys
		 *  3. If this is a transaction data, stroe it in the transaction journal
		 *  
		 *  ON INSERT:
		 *  1. Use old sector locationd data first for HD usage efficiency
		 *  2. Piece together and see if specific key needs to be appended.
		 *  3. If sector is only at partial capacity whether dynamic or static, 
		 *     have the available sector space be logged in the sector location or the key memory map as 'empty space'
		 */
		bool Erase(const std::vector<unsigned char> vKey)
		{
			/* Check for the Key. **/
			if(!mapKeys.count(vKey))
				return error("Key Erase() : Key doesn't Exist");
			
			/* Establish the Outgoing Stream. **/
			std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			fStream.seekp(mapKeys[vKey], std::ios::beg);
			
			/* Establish the Sector State as Empty. **/
			std::vector<unsigned char> vData(1, EMPTY);
			fStream.write((char*) &vData[0], vData.size());
			fStream.close();
				
			/* Remove the Sector Key from the Memory Map. **/
			mapKeys.erase(vKey);
			
            /* TODO: Add the erased key to a memory map for later indexing. */
            
			return true;
		}
		
		/** Get a Record from the Database with Given Key. **/
		bool Get(const std::vector<unsigned char> vKey, SectorKey& cKey)
		{
			MUTEX_LOCK(KEY_MUTEX);
			
			/** Read a Record from Binary Data. **/
			if(mapKeys.count(vKey))
			{
				/** Open the Stream Object. **/
				std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::binary);

				/** Seek to the Sector Position on Disk. **/
				fStream.seekg(mapKeys[vKey], std::ios::beg);
			
				/** Read the State and Size of Sector Header. **/
				std::vector<unsigned char> vData(cKey.Size(), 0);
				fStream.read((char*) &vData[0], vData.size());
				
				/** De-serialize the Header. **/
				CDataStream ssHeader(vData, SER_LLD, DATABASE_VERSION);
				ssHeader >> cKey;
				
				/** Debug Output of Sector Key Information. **/
				if(GetArg("-verbose", 0) >= 4)
					printf("KEY::Get(): State: %s Length: %u Location: %u Sector File: %u Sector Size: %u Sector Start: %u\n Key: %s\n", cKey.nState == READY ? "Valid" : "Invalid", cKey.nLength, mapKeys[cKey.vKey], cKey.nSectorFile, cKey.nSectorSize, cKey.nSectorStart, HexStr(cKey.vKey.begin(), cKey.vKey.end()).c_str());
						
				/** Skip Empty Sectors for Now. (TODO: Expand to Reads / Writes) **/
				if(cKey.Ready() || cKey.Append()) {
				
					/** Read the Key Data. **/
					std::vector<unsigned char> vKeyIn(cKey.nLength, 0);
					fStream.read((char*) &vKeyIn[0], vKeyIn.size());
					fStream.close();
					
					/** Check the Keys Match Properly. **/
					if(vKeyIn != vKey)
						return error("Key Mistmatch::DB:: %s  MEM %s\n", HexStr(vKeyIn.begin(), vKeyIn.end()).c_str(), HexStr(vKey.begin(), vKey.end()).c_str());
					
					/** Assign Key to Sector. **/
					cKey.vKey = vKeyIn;
					
					return true;
				}
				
				/** Close the Stream. **/
				fStream.close();
			}
			
			return false;
		}
	};
}

#endif
