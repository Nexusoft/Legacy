#ifndef LOWER_LEVEL_LIBRARY_LLD_TYPES
#define LOWER_LEVEL_LIBRARY_LLD_TYPES

#include "../LLU/util.h"
#include <boost/thread.hpp>
#include <fstream>

#define MUTEX_LOCK(a) boost::lock_guard<boost::mutex> lock(a)

/** Lower Level Database Name Space. **/
namespace LLD
{
	/* Handle to automatically create new files if the file size is exceeded. 
	 * TODO: Make this a configurable option
	 * 
	 * NOTE: A larger max file size should be seen within constraints of OS. Keep under 1 GB for MAx Compatability. 
	 */
	const unsigned int MAX_FILE_SIZE = 1024 * 1024;
	
	/* Handle to automatically append sectors of a given size. 
	 * TODO: Make this a configured option
	 * 
	 * NOTE: A larger APPEND_SIZE will make the O(n) operate faster over larger datq sizes and appends.
	 */
	const unsigned int MAX_APPEND_SIZE = 1024 * 1024;
    
	/* Enumeration for each State.
		Allows better thread concurrency
		Allow Reads on READY and READ. 
		
		TODO: Only Flush to Database if not Cached*/
	enum 
	{
		EMPTY 			= 0x00, //Keep track of empty sector keys to be used later
		READ  			= 0x01, //Keep track of read operations on the sector key
		WRITE 			= 0x02, //Keep track of if the sector is being written or not
		APPEND          = 0x04  //Keep track of whether there is any extra data that can be written in sectors without creating a new key/sector location
	};
	
    
	/**	Used to locate key sectors and pieces
	 * 	Each Key Location is stored in the keychain filesystems.
	 * 	This is for storage and navigation of the keychain keys
	 * 	to track the Sector positions and piece them together
	 * .
	 * 	TODO: Assign route positions to each key position. */
	class KeyBase
	{
	public:
		unsigned int   nKeyPosiition;
		unsigned short nKeyFile;
		
		/* Serialization Macro. */
		IMPLEMENT_SERIALIZE
		(
			READWRITE(nKeyPosition);
			READWRITE(nKeyFile);
		)
		
		/* Key Base Constructors. */
		KeyBase() { SetNull(); }
		KeyBase(unsigned int nPosIn) : nKeyPosition(nPosIn), nKeyFile(0) {}
		KeyBase(unsigned int nPosIn, unsigned short nFileIn) : nKeyPosition(nPosIn), nKeyFile(nFileIn) {}
		
		/* Set the validity of this key base loaction. */
		void SetNull() { nKeyPosiition = 0; nKeyFile = 0; }
		
		/* Null check indicator to determine validity of this keybase. */
		bool IsNull(){ return (nKeyFile == 0); }
        
		/* Size of the Key Base class. Fixed width, but for expandability in the future. */
		int  Size()  { return 6; }
	};
	
    
	/*  Sector Location Class
	 * 
	 *  Handles the storing of the data associated with each sector position
	 *  This allows the sectors to be pieced together to establish dynamic record allocation.
	 * 
	 *  This is Indexed by the Key Location class to be read from the location filesystems.
	 *  
	 *  TODO: Think of optized algorithm and structuring, so far is O(3 +- 2)
	 *  Sector Location handles the reading and writing of files instead of the need for a sepearate class initialization.
	 */
	class KeyChain
	{
		mutable boost::mutex MUTEX;
        
	public:
		/* These three hold the location of 
			Sector in the Sector Database of 
			Given Sector Key. */
		unsigned short            nSectorFile;
		unsigned short            nSectorSize;
		unsigned int              nSectorStart;
        
		/* Key Base Location for Next Key in the Chain. */
		KeyBase cNext;
		
		KeyChain() { SetNull(); }
		KeyChain(unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn) : nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) { }
		KeyChain(unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn, unsigned short nNextFile, unsigned int nNextPosition) : nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) 
		{ 
			cNext.nKeyFile     = nNextFile;
			cNext.nKeyPosition = nNextPosition;
		}
		
		/* Serialization Macro. */
		IMPLEMENT_SERIALIZE
		(
			READWRITE(nSectorFile);
			READWRITE(nSectorSize);
			READWRITE(nSectorStart);
			READWRITE(cNext);
		)
		
		/* Automatic assignment of NULL value. */
		void SetNull()
		{
				nSectorFile  = 0;
				nSectorSize  = 0;
				nSectorStart = 0;
				cNext.SetNull();
		}
		
		/* Flags to know whether this location is active. */
		bool IsNull()
		{
			return (nSectorFile == 0 && nSectorSize == 0 && nSectorStart == 0 && cNext.IsNull());
		}
		
		/* Fixed width size for accuracy in size projections. */
		int Size() { return 8 + cNext.Size(); }
		
		/* Indicator if this is the last key in the keychain. */
		bool Last() { return cNext.IsNull(); }
        
		/* Check for appending data.
		 * 
		 * TODO: This checks how much free data is available in the last sector.
		 * Dynamic sectors will need to be allocated at a fixed sector size.
		 * This will only come into play if it is set as a dynamic record.
		 * This is controlled by a record being appended when it is already written.
		 * This will set an auto flag, and begin an auto growth in the sector data. 
		 * 
		 * Return value of (-1) indicates it can't be determined because current iterator is not on last sector.
		 */
		int Append()
		{
			if(!cNext.IsNull())
				return -1;
			
			/* Send fail code if there is a problem with the sector sizes. */
			if(nSectorSize < cNext.nKeyPosition)
				return -1;
            
			/* Free Append data is determined by the Next Keybase nKeyPosition
			 * This becomes the substitue for the total used space in sector. 
			 */
			return (nSectorSize - cNext.nKeyPosition);
		}
		
		/* Read the Sector Location data from a Key Location Index. */
		bool Read(KeyBase cKeyLocation, std::string strDirectory)
		{
			std::fstream fStream(strprintf("%s/keychain.%04u", strDirectory.c_str(), nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
			if(!fStream) //read operation should not be done if the file doesn't exist
				return false;
			
			fStream.seekg(nSectorStart, std::ios::beg);
			std::vector<unsigned char> vSector(Size(), 0);
			fStream.read((char*) &vSector[0], vSector.size());
			
			CDataStream ssValue(vSector, SER_LLD, DATABASE_VERSION);
			ssValue >> *this;
            
			return false;
		}
        
		/* Write the Sector Location data from a Key Location Index. */
		bool Write(KeyBase cKeyLocation, std::string strDirectory)
		{
			std::fstream fStream(strprintf("%s/keychain.%04u", strDirectory.c_str(), nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
			if(!fStream) //write operation should not be done if the file doesn't exist
				return false;
			
			fStream.seekp(nSectorStart, std::ios::beg);
			CDataStream ssValue(vSector, SER_LLD, DATABASE_VERSION);
			ssValue << *this;
            
			std::vector<unsigned char> vSector(ssValue.begin(), ssValue.end());
			fStream.write((char*) &vSector[0], vSector.size());
			
			return false;
		}
		
		/* Operator Overloading for Read Ability. */
		friend bool operator==(const SectorLocation& a, const SectorLocation& b)  { return (a.nSectorFile == b.nSectorFile && a.nSectorSize == b.nSectorSize && a.nSectorStart == b.nSectorStart); }
	};
    
	
	/** Key Class to Hold the Location of Sectors it is referencing. 
		This Indexes the Sector Database. **/
	class SectorKey
	{
	public:
		
		/** The Key Header:
			Byte 0: nState
			Byte 1 - 3: nLength (The Size of the Key)
		**/
		unsigned char   		   	nState;
		unsigned short 			   nLength;
		
		/* Checksum of Original Data to ensure no database corrupted sectors. 
			TODO: Consider the Original Data from a checksum.
			When transactions implemented have transactions stored in a sector Database.
			Ensure Original keychain and new keychain are stored on disk.
			If there is failure here before it is commited to the main database it will only
			corrupt the database backups. This will ensure the core database never gets corrupted.
			On startup ensure that the checksums match to ensure that the database was not stopped
			in the middle of a write. */
		unsigned int nChecksum;
        
        /* TODO: Timestamp of last data write. Useful for knowing last time sector was modified and do
         * key sorting algorithms giving priority to newer keys, or once the memory layers and keychain
         * structures are stored in a binary tree from root memory branches using the index and file positions,
         * this will help make deeper layered items be the older records in the database. */
        //unsigned int nTimestamp;
		
		/* The Kye Locations. */
		mutable KeyChain cKeychain;
		
		/* The binary data of the Sector key. */
		std::vector<unsigned char> vKey;
		
		/** Serialization Macro. **/
		IMPLEMENT_SERIALIZE
		(
			READWRITE(nState);
			READWRITE(nLength);
			
			if (!(nType & SER_LLD_KEY_HEADER))
			{
				READWRITE(nChecksum);
				READWRITE(cKeychain);
			}
		)
		
		/** Constructors. **/
		SectorKey() : nState(0), nLength(0), nSectorStart() { }
		SectorKey(unsigned char nStateIn, std::vector<unsigned char> vKeyIn) : 
			nState(nStateIn)
		{
			nLength = vKeyIn.size();
			vKey    = vKeyIn;
		}
		
		/** Return the Size of the Key Sector on Disk. **/
		int Size() { return (7 + cKeychain.Size() + nLength); }
		
		/* A standard validating Empty check for sector data. */
		bool Empty() { return (nState == EMPTY); }
		
		/* A standard validated Ready check for sector. */
		bool Ready() { return (nState == READY || nState == APPEND); }
		
		/* Append Sector means that there is data available in the sector chains to be added without new key. */
		bool Append(){ return (nState == APPEND); }
		
		/* Check if there is more data in the keychain. */
		bool HasNext(){ return !cKeyNext.IsNull(); } 
		
	};
}

#endif
