#ifndef LOWER_LEVEL_LIBRARY_LLD_SECTOR
#define LOWER_LEVEL_LIBRARY_LLD_SECTOR

#include "keychain.h"

/** Lower Level Database Name Space. **/
namespace LLD
{

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
			printf("[DATABASE] Initializing Sector Database.\n");
			
			/** Create the Sector Database Directories. **/
			boost::filesystem::path dir(strBaseLocation);
			boost::filesystem::create_directory(dir);
			
			/** TODO: Make a worker or thread to check sizes of files and automatically create new file.
				Keep independent of reads and writes for efficiency. **/
			std::fstream fIncoming(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), 0).c_str(), std::ios::in | std::ios::binary);
			if(!fIncoming) {
				std::ofstream cStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), 0).c_str(), std::ios::binary);
				cStream.close();
				
				printf("[DATABASE] Creating New Sector Database.\n");
			}
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
			return Put(vKey, vData);
		}
		
		/** Get a Record from the Database with Given Key. **/
		bool Get(std::vector<unsigned char> vKey, std::vector<unsigned char>& vData)
		{
			MUTEX_LOCK(SECTOR_MUTEX);
			
			/** Read a Record from Binary Data. **/
			if(GetKeychain(strKeychainRegistry)->HasKey(vKey))
			{
				/** Read the Sector Key from Keychain. **/
				SectorKey cKey;
				if(!GetKeychain(strKeychainRegistry)->Get(vKey, cKey))
					return false;
				
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), cKey.nSectorFile).c_str(), std::ios::in | std::ios::binary);

				/** Seek to the Sector Position on Disk. **/
				fStream.seekg(cKey.nSectorStart);
			
				/** Read the State and Size of Sector Header. **/
				vData.resize(cKey.nSectorSize);
				fStream.read((char*) &vData[0], vData.size());
				fStream.close();
				
				//printf("SECTOR GET:%s\n", HexStr(vData.begin(), vData.end()).c_str());
				
				return true;
			}
			
			return false;
		}
		
		
		/** Add / Update A Record in the Database **/
		bool Put(std::vector<unsigned char> vKey, std::vector<unsigned char> vData)
		{
			MUTEX_LOCK(SECTOR_MUTEX);
			
			/** Write Header if First Update. **/
			if(!GetKeychain(strKeychainRegistry)->HasKey(vKey))
			{
				/** TODO:: Assign a Sector File based on Database Sizes. **/
				unsigned short nSectorFile = 0;
				
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
				
				/** If it is a New Sector, Assign a Binary Position. 
					TODO: Track Sector Database File Sizes. **/
				unsigned int nBegin = fStream.tellg();
				fStream.seekg (0, std::ios::end);
				
				unsigned int nStart = (unsigned int) fStream.tellg() - nBegin;
				fStream.seekp(nStart, std::ios::beg);
				
				fStream.write((char*) &vData[0], vData.size());
				fStream.close();
				
				/** Create a new Sector Key. **/
				SectorKey cKey(READY, vKey, nSectorFile, nStart, vData.size()); 
				
				/** Assign the Key to Keychain. **/
				GetKeychain(strKeychainRegistry)->Put(cKey);
			}
			else
			{
				/** Get the Sector Key from the Keychain. **/
				SectorKey cKey;
				if(!GetKeychain(strKeychainRegistry)->Get(vKey, cKey))
					return false;
					
				/** Open the Stream to Read the data from Sector on File. **/
				std::fstream fStream(strprintf("%s%s%u.dat", strBaseLocation.c_str(), strBaseName.c_str(), cKey.nSectorFile).c_str(), std::ios::in | std::ios::out | std::ios::binary);
				
				/** Locate the Sector Data from Sector Key. 
					TODO: Make Paging more Efficient in Keys by breaking data into different locations in Database. **/
				fStream.seekp(cKey.nSectorStart, std::ios::beg);
				if(vData.size() > cKey.nSectorSize){
					fStream.close();
					printf("ERROR PUT (TOO LARGE) NO TRUNCATING ALLOWED:%s\n", HexStr(vData.begin(), vData.end()).c_str());
					
					return false;
				}
				
				fStream.write((char*) &vData[0], vData.size());
				fStream.close();
			}
			
			//printf("SECTOR PUT:%s\n", HexStr(vData.begin(), vData.end()).c_str());
			
			return true;
		}
	};
}

#endif