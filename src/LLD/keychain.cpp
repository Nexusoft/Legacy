#include "key.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
	/* 	The Keychain Database Structures and Filesystems is as follows:
	 * 	
	 * 	Keychain Database Registry -> Holds Key Locations in Key Database.
	 * 		I. Stored in Memory on Keychain map for quick accass and low rewauirements.
	 * 
	 * 	Keychain Database Keys File -> Main keychain file located in 'keychain/[registry].keys'
	 * 	Keychain Key Locations File -> Main keychain locations files in 'keychain/[registry]/locations.001 ....'
	 * 	
	 * 	Keychain Sector Locations   -> Main keychain sector binary locations in 'keychain/[registry]/sectors.001 ....'
	 * 
	 * 	Datachain File Locations    -> Main datachain sector binary locations in 'datachain/[registry].001 ....'
	 * 
	 * 
	 * 	Database Keychain Logic Sequences
	 * 
	 * 	Read:
	 * 		1. Obtain Key location from Keychain Registry
	 * 		2. Read Key from Keychain dat file
	 * 			For Dynamic Sectors with no NULL cNext Record (Slower)
	 * 			I.   Read first Sector Location, add first record to Vector
	 * 			II.  Iterate the cNext Location, read Sector Location, add to Vector
	 * 			III. Iterate until cNext is NULL, return Key Locations Vector
	 * 
	 * 		3. Read Sectors from Datachain
	 * 		4. Return Deserialized and concatenated master sector from pieces (If Dynamic)
	 * 
	 * 	Write:
	 * 		1. Obtain Key location from Keychain Registry
	 * 		2. Read Key from Keychain dat file
	 * 			For Dynamic Sectors with no NULL cNext Record (Slower)
	 * 			I.   Read first Sector Location, add first record to Vector
	 * 			II.  Iterate the cNext Location, read Sector Location, add to Vector
	 * 			III. Iterate until cNext is NULL, return Key Locations Vector
	 * 		
	 * 		3. Piece out the Master Sector into Pieces based on keychain location allocations
	 * 		4. If there is remaining data after Pieces, add a new sector location to last keychain location. Write last record.
	 * 
	 * */
	
	/* Handle the Registry of Shared Keychain Pointer Objects. */
	std::map<std::string, KeyDatabase*> mapKeychainRegistry;
	
	/* Handle the Key Registry. */
	boost::mutex REGISTRY_MUTEX;
	
	/* Handle the Registrying of Keychains for LLD Sectors. */
	void RegisterKeychain(std::string strRegistryName, std::string strBaseName)
	{
		MUTEX_LOCK(REGISTRY_MUTEX);
		
		/* Create the New Keychain Database. */
		KeyDatabase* SectorKeys = new KeyDatabase(GetDefaultDataDir().string() + "/keychain/", strBaseName);
		SectorKeys->Initialize();
		
		/* Log the new Keychain into the Memeory Map. */
		mapKeychainRegistry[strRegistryName] = SectorKeys;
		
		/* Debug Output for Keychain Database Initialization. */
		printf("[KEYCHAIN] Registered Keychain For Database %s\n", strRegistryName.c_str());
	}
	
	/* Return the Keychain Pointer Object. */
	KeyDatabase* GetKeychain(std::string strRegistryName) {
		if(!mapKeychainRegistry.count(strRegistryName))
			return NULL;
		
		return mapKeychainRegistry[strRegistryName];
	}
	
	/* TODO:: Break Keychain Registry into another Database that stores the Keychain Registry and States on Disk.
		This can then be used to remove all memory requirements of the Database if so Desired. */
}
