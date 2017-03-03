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
	
	
	/* ______________________________________________________________________________________________________________________________  */
	
	
	
	const char* DNS_SeedNodes[] = 
	{
		"node1.nexusearth.com",
		"node1.mercuryminer.com",
		"node1.nexusminingpool.com",
		"node1.nxs.efficienthash.com",
		"node2.nexusearth.com",
		"node2.mercuryminer.com",
		"node2.nexusminingpool.com",
		"node2.nxs.efficienthash.com",
		"node3.nexusearth.com",
		"node3.mercuryminer.com",
		"node3.nxs.efficienthash.com",
		"node4.nexusearth.com",
		"node4.mercuryminer.com",
		"node4.nxs.efficienthash.com",
		"node5.nexusearth.com",
		"node5.mercuryminer.com",
		"node5.nxs.efficienthash.com",
		"node6.nexusearth.com",
		"node6.mercuryminer.com",
		"node6.nxs.efficienthash.com",
		"node7.nexusearth.com",
		"node7.mercuryminer.com",
		"node7.nxs.efficienthash.com",
		"node8.nexusearth.com",
		"node8.mercuryminer.com",
		"node8.nxs.efficienthash.com",
		"node9.nexusearth.com",
		"node9.mercuryminer.com",
		"node9.nxs.efficienthash.com",
		"node10.nexusearth.com",
		"node10.mercuryminer.com",
		"node10.nxs.efficienthash.com",
		"node11.nexusearth.com",
		"node11.mercuryminer.com",
		"node11.nxs.efficienthash.com",
		"node12.nexusearth.com",
		"node12.mercuryminer.com",
		"node12.nxs.efficienthash.com",
		"node13.nexusearth.com",
		"node13.mercuryminer.com",
		"node13.nxs.efficienthash.com",
	};

	
	const char* DNS_SeedNodes_Testnet[] = 
	{
		"test1.nexusoft.io"
	};	
	
	
	
	/* ______________________________________________________________________________________________________________________________  */
	
	
	
	/* The DNS Lookup Routine to find the Nodes that are set as DNS seeds. */
	std::vector<CAddress> DNS_Lookup(const char* DNS_Seed[]);
	
	
	/* Standard Wrapper Function to Interact with cstdlib DNS functions. */
	bool LookupHost(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0, bool fAllowLookup = true);
	
	
}

#endif
