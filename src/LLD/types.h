#ifndef LOWER_LEVEL_LIBRARY_LLD_TYPES
#define LOWER_LEVEL_LIBRARY_LLD_TYPES

#include "../LLU/util.h"

/* TODO: Move this helper to LLU
	 * Helps to parse out the individual bits from a byte to use multiple flags in a 0xff range. */
	bool GetBit(unsigned char byte, int position) // position in range 0-7
	{
		return (byte >> position) & 0x1;
	}

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
		EMPTY 			= 0x00,
		READ  			= 0x01,
		WRITE 			= 0x02,
		TRANSACTION    = 0x03
	}
	
	/** Sector Location Class
	 * 
	 *  Handles the storing of the data associated with each sector position
	 *  This allows the sectors to be pieced together to establish dynamic record allocation.
	 */
	class SectorLocation
	{
	public:
		
		/** These three hold the location of 
			Sector in the Sector Database of 
			Given Sector Key. **/
		unsigned short 			   nSectorFile;
		unsigned short   		   	nSectorSize;
		unsigned int   			   nSectorStart;
		
		SectorLocation() { SetNull(); }
		SectorLocation(unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn) : nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) { }
		
		/* Automatic assignment of NULL value. */
		void SetNull()
		{
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
		unsigned int Size() { return 8; }
		
		/* Operator Overloading for Read Ability. */
		friend bool operator==(const SectorLocation& a, const SectorLocation& b)  { return (a.nSectorFile == b.nSectorFile && a.nSectorSize == b.nSectorSize && a.nSectorStart == b.nSectorStart); }
	};
	
	/** Sector Position Class:
	 * 
	 *  Determines the location in the chain of sector data to piece
	 *  together all the LLD sector data.
	 **/
	class SectorPosition : public SectorLocation
	{
	public:
		
		/* Reference to the location of the next sector data space. */
		SectorLocation cSectorNext;
		
		/** Serialization Macro. **/
		IMPLEMENT_SERIALIZE
		(
			READWRITE(nSectorFile);
			READWRITE(nSectorSize);
			READWRITE(nSectorStart);
			
			if (!(nType & SER_LLD_KEY_HEADER))
			{
				READWRITE(cSectorNext);
			}
		)
		
		/* Basic Constructor. */
		SectorPosition() 
		{ 
			SetNull();
			cSectorNext.SetNull(); 
		}
		
		SectorPosition(unsigned short nFileIn, unsigned short nSizeIn, unsigned int nStartIn) : 
			nSectorFile(nFileIn), nSectorSize(nSizeIn), nSectorStart(nStartIn) { cSectorNext.SetNull(); }

		bool HasNext() { return cSectorNext.IsNull(); }
		unsigned int Size() { return 16; }
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
		
		/** Checksum of Original Data to ensure no database corrupted sectors. 
			TODO: Consider the Original Data from a checksum.
			When transactions implemented have transactions stored in a sector Database.
			Ensure Original keychain and new keychain are stored on disk.
			If there is failure here before it is commited to the main database it will only
			corrupt the database backups. This will ensure the core database never gets corrupted.
			On startup ensure that the checksums match to ensure that the database was not stopped
			in the middle of a write. **/
		unsigned int nChecksum;
		
		/* Location of the First Sector for the Key. */
		SectorPosition cSectorStart;
		
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
				READWRITE(cSectorStart);
			}
		)
		
		/** Constructors. **/
		SectorKey() : nState(0), nLength(0), nSectorStart() { }
		SectorKey(unsigned char nStateIn, std::vector<unsigned char> vKeyIn, unsigned short nSectorFileIn, unsigned int nSectorStartIn, unsigned short nSectorSizeIn) : 
			nState(nStateIn)
		{ 
			cSectorStart.nSectorFile  = nSectorFileIn;
			cSectorStart.nSectorSize  = nSectorSizeIn;
			cSectorStart.nSectorStart = nSectorStartIn;
			
			nLength = vKeyIn.size();
			vKey    = vKeyIn;
		}
		
		/** Return the Size of the Key Sector on Disk. **/
		unsigned int Size() { return (7 + cSectorStart.Size() + nLength); }
		
		/** Check for Key Activity on Sector. **/
		bool Empty() { return (nState == EMPTY); }
		bool Ready() { return (nState == READY); }
		bool IsTxn() { return (nState == TRANSACTION); }
		
	};
	
#endif
