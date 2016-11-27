#ifndef LOWER_LEVEL_LIBRARY_LLD_LOCATIONS
#define LOWER_LEVEL_LIBRARY_LLD_LOCATIONS

#include <boost/thread.hpp>
#include <fstream>
#include "types.h"

#define MUTEX_LOCK(a) boost::lock_guard<boost::mutex> lock(a)

/** Lower Level Database Name Space. **/
namespace LLD
{

	/** Location Database:
	 * 
	 * + Stores the Pieces of Location data to determine Sector Positions.
	 * + TODO: Optionally Cache Location Data in Memory when done
	**/
	class LocationDatabase
	{
	protected:
		
		/* The String to hold the Disk Location of Database File. 
			Each Database File Acts as a New Table as in Conventional Design.
			Key can be any Type, which is how the Database Records are Accessed. */
		std::string strBaseLocation;
		std::string strLocation;
		
		/** The Database Constructor. To determine file location and the Bytes per Record. **/
		LocationDatabase(std::string strBaseLocationIn, std::string strDatabaseNameIn) : strBaseLocation(strBaseLocationIn) { strLocation = strBaseLocation + strDatabaseNameIn + "/"}
		
		
		/** Add / Update a Record in the Locations Database **/
		bool Add(KeyLocation cLocation) const
		{
			/** Establish the Outgoing Stream. **/
			std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			if(!fStream)
			{
				boost::filesystem::path dir(strBaseLocation);
				boost::filesystem::create_directory(dir);
			
				std::ofstream cStream(strLocation.c_str(), std::ios::binary);
				cStream.close();	
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
		
		/** Simple Erase for now, not efficient in Data Usage of HD but quick to get erase function working. **/
		bool Erase(const std::vector<unsigned char> vKey)
		{
			/** Check for the Key. **/
			if(!mapKeys.count(vKey))
				return error("Key Erase() : Key doesn't Exist");
			
			/** Establish the Outgoing Stream. **/
			std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			fStream.seekp(mapKeys[vKey], std::ios::beg);
			
			/** Establish the Sector State as Empty. **/
			std::vector<unsigned char> vData(1, EMPTY);
			fStream.write((char*) &vData[0], vData.size());
			fStream.close();
				
			/** Remove the Sector Key from the Memory Map. **/
			mapKeys.erase(vKey);
			
			return true;
		}
		
		/** Get a Record from the Database with Given Key. **/
		bool Get(const std::vector<unsigned char> vKey, SectorKey& cKey)
		{
			MUTEX_LOCK(KEY_MUTEX);
			
			/** Cache Handler. **/
			if(mapKeysCache.count(vKey) && fMemoryCaching)
			{
				cKey = mapKeysCache[vKey];
				cKey.vKey = vKey;
				
				return true;
			}
			
			/** Read a Record from Binary Data. **/
			if(mapKeys.count(vKey))
			{
				/** Open the Stream Object. **/
				std::fstream fStream(strLocation.c_str(), std::ios::in | std::ios::binary);

				/** Seek to the Sector Position on Disk. **/
				fStream.seekg(mapKeys[vKey], std::ios::beg);
			
				/** Read the State and Size of Sector Header. **/
				std::vector<unsigned char> vData(19, 0);
				fStream.read((char*) &vData[0], 19);
				
				/** De-serialize the Header. **/
				CDataStream ssHeader(vData, SER_LLD, DATABASE_VERSION);
				ssHeader >> cKey;
				
				/** Debug Output of Sector Key Information. **/
				if(GetArg("-verbose", 0) >= 4)
					printf("KEY::Get(): State: %s Length: %u Location: %u Sector File: %u Sector Size: %u Sector Start: %u\n Key: %s\n", cKey.nState == READY ? "Valid" : "Invalid", cKey.nLength, mapKeys[cKey.vKey], cKey.nSectorFile, cKey.nSectorSize, cKey.nSectorStart, HexStr(cKey.vKey.begin(), cKey.vKey.end()).c_str());
						
				/** Skip Empty Sectors for Now. (TODO: Expand to Reads / Writes) **/
				if(cKey.Ready() || cKey.IsTxn()) {
				
					/** Read the Key Data. **/
					std::vector<unsigned char> vKeyIn(cKey.nLength, 0);
					fStream.read((char*) &vKeyIn[0], vKeyIn.size());
					fStream.close();
					
					/** Check the Keys Match Properly. **/
					if(vKeyIn != vKey)
						return error("Key Mistmatch: DB:: %s  MEM %s\n", HexStr(vKeyIn.begin(), vKeyIn.end()).c_str(), HexStr(vKey.begin(), vKey.end()).c_str());
					
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
