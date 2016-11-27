#ifndef LOWER_LEVEL_LIBRARY_LLD_TYPES
#define LOWER_LEVEL_LIBRARY_LLD_TYPES

#include "../LLU/util.h"

/** Lower Level Database Name Space. **/
namespace LLD
{

	/** Enumeration for each State.
		Allows better thread concurrency
		Allow Reads on READY and READ. 
		
		Only Flush to Database if not Cached. (TODO) *
		
		TODO: Add Static and Dynamic Sector Keys for Space Consideration. 
		This keyspace will be moved 
		
		Use Bit Masks to calculate the Enumeration. 
		TODO: Remove this enumeration into the proper bit masks handled in the keychaiun class */
	enum 
	{
		EMPTY 			= 0x00, //Keep track of empty sector keys to be used later
		READ  			= 0x01, //Keep track of read operations on the sector key
		WRITE 			= 0x02, //Keep track of if the sector is being written or not
		APPEND         = 0x04  //Keep track of whether there is any extra data that can be written in sectors without creating a new key/sector location
	};
	
	/**	Used to locate key sectors and pieces
	 * 	Each Key Location is stored in the keychain filesystems.
	 * 	This is for storage and navigation of the keychain keys
	 * 	to track the Sector positions and piece them together
	 * .
	 * 	TODO: Assign route positions to each key position. */
	class KeyLocation
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
		
		/* Constructors. */
		KeyLocation() { SetNull(); }
		KeyLocation(unsigned int nPosIn) : nKeyPosition(nPosIn), nKeyFile(0) {}
		KeyLocation(unsigned int nPosIn, unsigned short nFileIn) : nKeyPosition(nPosIn), nKeyFile(nFileIn) {}
		
		void SetNull() { nKeyPosiition = 0; nKeyFile = 0; }
		bool IsNull(){ return nKeyPosition == 0 && nKeyFile == 0; }
		int  Size()  { return 6; }
	};
	
	/** Sector Location Class
	 * 
	 *  Handles the storing of the data associated with each sector position
	 *  This allows the sectors to be pieced together to establish dynamic record allocation.
	 */
	class SectorLocation
	{
	public:
		
		/* These three hold the location of 
			Sector in the Sector Database of 
			Given Sector Key. */
		unsigned short					nSectorID;
		unsigned short 			   nSectorFile;
		unsigned short   		   	nSectorSize;
		unsigned int   			   nSectorStart;
		
		SectorLocation() { SetNull(); }
		SectorLocation(unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn) : nSectorID(0), nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) { }
		SectorLocation(unsigned short nIDIn, unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn) : nSectorID(nIDIn), nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) { }
		
		/* Serialization Macro. */
		IMPLEMENT_SERIALIZE
		(
			READWRITE(nSectorID);
			READWRITE(nSectorFile);
			READWRITE(nSectorSize);
			READWRITE(nSectorStart);
		)
		
		/* Automatic assignment of NULL value. */
		void SetNull()
		{
				nSectorID    = 0;
				nSectorFile  = 0;
				nSectorSize  = 0;
				nSectorStart = 0;
		}
		
		/* Flags to know whether this location is active. */
		bool IsNull()
		{
			return (nSectorFile == 0 && nSectorSize == 0 && nSectorStart == 0);
		}
		
		/* Fixed width size for accuracy in size projections. */
		unsigned int Size() { return 10; }
		
		/* Operator Overloading for Read Ability. */
		friend bool operator==(const SectorLocation& a, const SectorLocation& b)  { return (a.nSectorID == b.nSectorID && a.nSectorFile == b.nSectorFile && a.nSectorSize == b.nSectorSize && a.nSectorStart == b.nSectorStart); }
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
		
		/* The Kye Locations. */
		mutable KeyLocation cKeyStart;
		mutable KeyLocation cKeyNext;
		
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
				READWRITE(cKeyStart);
				READWRITE(cKeyNext);
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
		unsigned int Size() { return (7 + cKeyStart.Size() + cKeyNext.Size() + nLength); }
		
		/* A standard validating Empty check for sector data. */
		bool Empty() { return (nState == EMPTY); }
		
		/* A standard validated Ready check for sector. */
		bool Ready() { return (nState == READY || nState == APPEND); }
		
		/* Append Sector means that there is data available in the sector chains to be added without new key. */
		bool Append(){ return (nState == APPEND); }
		
		/* A static key is one that only has one sector data is pointing to. */
		bool Statci(){ return cKeyNext.IsNull(); } 
		
	};
}

#endif
