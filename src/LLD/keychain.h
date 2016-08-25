#ifndef LOWER_LEVEL_LIBRARY_LLD_KEYCHAIN
#define LOWER_LEVEL_LIBRARY_LLD_KEYCHAIN

#include "key.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
	/** Handle the Registry of Shared Keychain Pointer Objects. **/
	std::map<std::string, KeyDatabase*> mapKeychainRegistry;
	
	/** Handle the Key Registry. **/
	boost::mutext REGISTRY_MUTEX;
	
	/** Handle the Registrying of Keychains for LLD Sectors. **/
	void RegisterKeychain(std::string strRegistryName, std::string::strBaseName)
	{
		REGISTRY_MUTEX.lock();
		
		/** Create the New Keychain Database. **/
		KeyDatabase* SectorKeys = new KeyDatabase(GetDefaultDataDir().string() + "//keychain//", strBaseName);
		SectorKeys->Initialize();
		
		/** Log the new Keychain into the Memeory Map. **/
		mapKeychainRegistry[strRegistryName] = SectorKeys;
		
		REGISTRY_MUTEX.unlock();
	}
	
	/** Return the Keychain Pointer Object. **/
	KeyDatabase* GetKeychain(std::string strRegistryName) {
		if(!mapKeychainRegistry.count(strRegistryName))
			return NULL;
		
		return mapKeychainRegistry[strRegistryName];
	}
	
	/** TODO:: Break Keychain Registry into another Database that stores the Keychain Registry and States on Disk.
		This can then be used to remove all memory requirements of the Database if so Desired. **/
}

#endif