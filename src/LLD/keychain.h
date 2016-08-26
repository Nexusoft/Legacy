#ifndef LOWER_LEVEL_LIBRARY_LLD_KEYCHAIN
#define LOWER_LEVEL_LIBRARY_LLD_KEYCHAIN

#include "key.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
	/** Handle the Registry of Shared Keychain Pointer Objects. **/
	extern std::map<std::string, KeyDatabase*> mapKeychainRegistry;
	
	/** Handle the Key Registry. **/
	extern boost::mutex REGISTRY_MUTEX;
	
	/** Handle the Registrying of Keychains for LLD Sectors. **/
	void RegisterKeychain(std::string strRegistryName, std::string strBaseName);
	
	/** Return the Keychain Pointer Object. **/
	KeyDatabase* GetKeychain(std::string strRegistryName);
}

#endif