/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#include "include/manager.h"

#include "../LLP/include/hosts.h"
#include "../LLC/include/random.h"

namespace Core
{
	
	Manager* pManager;
	
	
	/* NODE MANAGER NOTE:
	 * 
	 * The Node Manager handles all the connections associated with a specifric node and rates them based on their interactions and trust they have built in th network
	 * 
	 * It is also intelligent to distinguish the best nodes to connect with and also process blocks in a single location so that there are not more processes that need happen
	 * and that everything can be done in the order it was inteneded.
	 */
	
	void Manager::TimestampManager()
	{

	}
	
	void Manager::ConnectionManager()
	{
		while(!fShutdown)
		{
			if(vNew.size() == 0 && vTried.size() == 0){
				Sleep(1000);
				
				continue;
			}

			//TODO: Make this tied to port macros
			if(!AddConnection(vNew[0].ToStringIP(), "9323"))
				vTried.push_back(vNew[0]);
				
			vNew.erase(vNew.begin());
			Sleep(5000);
		}
	}
		
		
	/* Blocks are checked in the order they are recieved. */
	void Manager::BlockProcessor()
	{

	}
		
		
	/* Add address to the Queue. */
	void Manager::AddAddress(LLP::CAddress cAddress)
	{
		vNew.push_back(cAddress);
	}
		
		
	/* Get a random address from the active connections in the manager. */
	LLP::CAddress Manager::GetRandAddress(bool fNew)
	{
		std::vector<LLP::CAddress> vSelect;
		if(fNew)
			vSelect.insert(vSelect.begin(), vNew.begin(), vNew.end());
		
	}
	
	
	/* Start up the Node Manager. */
	void Manager::Start()
	{
		std::vector<LLP::CAddress> vSeeds = LLP::DNS_Lookup(fTestNet ? LLP::DNS_SeedNodes_Testnet : LLP::DNS_SeedNodes);
		for(int nIndex = 0; nIndex < vSeeds.size(); nIndex++)
			AddAddress(vSeeds[nIndex]);
		
	}
}
