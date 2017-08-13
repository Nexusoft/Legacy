/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_HOSTS_H
#define NEXUS_LLP_INCLUDE_HOSTS_H

#include <string>
#include <vector>

namespace LLP
{
	
	class CAddress;
	class CNetAddr;
	class CService;
	
	
	/* The DNS Lookup Routine to find the Nodes that are set as DNS seeds. */
	std::vector<CAddress> DNS_Lookup(std::vector<std::string> DNS_Seed);
	
	
	/* Standard Wrapper Function to Interact with cstdlib DNS functions. */
	bool LookupHost(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0, bool fAllowLookup = true);
	bool LookupHostNumeric(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0);
	bool Lookup(const char *pszName, CService& addr, int portDefault = 0, bool fAllowLookup = true);
	bool Lookup(const char *pszName, std::vector<CService>& vAddr, int portDefault = 0, bool fAllowLookup = true, unsigned int nMaxSolutions = 0);
	bool LookupNumeric(const char *pszName, CService& addr, int portDefault = 0);
}

#endif
