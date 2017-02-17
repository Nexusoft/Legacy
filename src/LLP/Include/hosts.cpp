/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "protocol.h"
#include "../../LLU/util.h"

#ifndef WIN32
#include <sys/fcntl.h>
#endif

#include "../LLU/strlcpy.h"

namespace LLP
{
	static const unsigned char pchIPv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
		
	/** DNS Query of Domain Names Associated with Seed Nodes **/
	vector<LLP::CAddress> DNS_Lookup(const char* DNS_Seed[])
	{
		vector<LLP::CAddress> vNodes;
		for (unsigned int seed = 0; seed < (fTestNet ? 1 : 41); seed++)
		{
			printf("%u Host: %s\n", seed, DNS_Seed[seed]);
			vector<LLP::CNetAddr> vaddr;
			if (LLP::LookupHost(DNS_Seed[seed], vaddr))
			{
					BOOST_FOREACH(LLP::CNetAddr& ip, vaddr)
					{
						LLP::CAddress addr = LLP::CAddress(LLP::CService(ip, LLP::GetDefaultPort()));
						vNodes.push_back(addr);
					
					printf("DNS Seed: %s\n", addr.ToStringIP().c_str());
					
					Net::addrman.Add(addr, ip, true);
					}
			}
		}
		
		return vNodes;
	}
	
	bool static LookupIntern(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions, bool fAllowLookup)
	{
		vIP.clear();
		struct addrinfo aiHint;
		memset(&aiHint, 0, sizeof(struct addrinfo));

		aiHint.ai_socktype = SOCK_STREAM;
		aiHint.ai_protocol = IPPROTO_TCP;
	#ifdef WIN32
	#  ifdef USE_IPV6
		aiHint.ai_family = AF_UNSPEC;
		aiHint.ai_flags = fAllowLookup ? 0 : AI_NUMERICHOST;
	#  else
		aiHint.ai_family = AF_INET;
		aiHint.ai_flags = fAllowLookup ? 0 : AI_NUMERICHOST;
	#  endif
	#else
	#  ifdef USE_IPV6
		aiHint.ai_family = AF_UNSPEC;
		aiHint.ai_flags = AI_ADDRCONFIG | (fAllowLookup ? 0 : AI_NUMERICHOST);
	#  else
		aiHint.ai_family = AF_INET;
		aiHint.ai_flags = AI_ADDRCONFIG | (fAllowLookup ? 0 : AI_NUMERICHOST);
	#  endif
	#endif
		struct addrinfo *aiRes = NULL;
		int nErr = getaddrinfo(pszName, NULL, &aiHint, &aiRes);
		if (nErr)
			return false;

		struct addrinfo *aiTrav = aiRes;
		while (aiTrav != NULL && (nMaxSolutions == 0 || vIP.size() < nMaxSolutions))
		{
			if (aiTrav->ai_family == AF_INET)
			{
				assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in));
				vIP.push_back(CNetAddr(((struct sockaddr_in*)(aiTrav->ai_addr))->sin_addr));
			}

	#ifdef USE_IPV6
			if (aiTrav->ai_family == AF_INET6)
			{
				assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in6));
				vIP.push_back(CNetAddr(((struct sockaddr_in6*)(aiTrav->ai_addr))->sin6_addr));
			}
	#endif

			aiTrav = aiTrav->ai_next;
		}

		freeaddrinfo(aiRes);

		return (vIP.size() > 0);
	}
	
	bool LookupHost(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions, bool fAllowLookup)
	{
		if (pszName[0] == 0)
			return false;
		char psz[256];
		char *pszHost = psz;
		strlcpy(psz, pszName, sizeof(psz));
		if (psz[0] == '[' && psz[strlen(psz)-1] == ']')
		{
			pszHost = psz+1;
			psz[strlen(psz)-1] = 0;
		}

		return LookupIntern(pszHost, vIP, nMaxSolutions, fAllowLookup);
	}
}

