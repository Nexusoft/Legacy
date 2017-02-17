/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_PROTOCOL_H
#define NEXUS_LLP_INCLUDE_PROTOCOL_H

#include "../../LLU/serialize.h"
#include "../../LLU/compat.h"
#include "../../LLT/uint1024.h"

#include <string>
#include <vector>

#ifdef WIN32
// In MSVC, this is defined as a macro, undefine it to prevent a compile and link error
#undef SetPort
#endif

#define NEXUS_PORT  9323
#define NEXUS_CORE_LLP_PORT 9324
#define NEXUS_MINING_LLP_PORT 9325

#define RPC_PORT     9336
#define TESTNET_RPC_PORT 9336

#define TESTNET_PORT 8313
#define TESTNET_CORE_LLP_PORT 8329
#define TESTNET_MINING_LLP_PORT 8325

extern bool fTestNet;
namespace LLP
{
	
	/** Declarations for the DNS Seed Nodes. **/
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

	
	/** Declarations for the DNS Seed Nodes. **/
	const char* DNS_SeedNodes_Testnet[] = 
	{
		"test1.nexusoft.io"
	};	
	
	
	/** Services flags */
	enum
	{
		NODE_NETWORK = (1 << 0),
	};
	
	
	/* Standard Wrapper Function to Interact with cstdlib DNS functions. */
	bool LookupHost(const char *pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0, bool fAllowLookup = true);
	
	
	/* The DNS Lookup Routine to find the Nodes that are set as DNS seeds. */
	std::vector<CAddress> DNS_Lookup(const char* DNS_Seed[]);
	
	
	/* Get the Main Message LLP Port for Nexus. */
	static inline unsigned short GetDefaultPort(const bool testnet = fTestNet){ return testnet ? TESTNET_PORT : NEXUS_PORT; }
	
	
	/* Proxy Settings fro Nexus Core. */
	int fUseProxy = false;
	CService addrProxy("127.0.0.1",9050);
	
	
	/** inv message data */
	class CInv
	{
		public:
			CInv();
			CInv(int typeIn, const uint1024& hashIn);
			CInv(const std::string& strType, const uint1024& hashIn);

			IMPLEMENT_SERIALIZE
			(
				READWRITE(type);
				READWRITE(hash);
			)

			friend bool operator<(const CInv& a, const CInv& b);

			bool IsKnownType() const;
			const char* GetCommand() const;
			std::string ToString() const;
			void print() const;

		// TODO: make private (improves encapsulation)
		public:
			int type;
			uint1024 hash;
	};

	
	/** A CService with information about it as peer */
	class CAddress : public CService
	{
		public:
			CAddress();
			explicit CAddress(CService ipIn, uint64 nServicesIn = NODE_NETWORK);

			void Init();

			IMPLEMENT_SERIALIZE
				(
				 CAddress* pthis = const_cast<CAddress*>(this);
				 CService* pip = (CService*)pthis;
				 if (fRead)
					 pthis->Init();
				 if (nType & SER_DISK)
					 READWRITE(nVersion);
				 if ((nType & SER_DISK) || (!(nType & SER_GETHASH)))
					 READWRITE(nTime);
				 READWRITE(nServices);
				 READWRITE(*pip);
				)

			void print() const;

		// TODO: make private (improves encapsulation)
		public:
			uint64 nServices;

			// disk and network only
			unsigned int nTime;

			// memory only
			int64 nLastTry;
	};


	/** IP address (IPv6, or IPv4 using mapped IPv6 range (::FFFF:0:0/96)) */
	class CNetAddr
	{
		protected:
			unsigned char ip[16]; // in network byte order

		public:
			CNetAddr();
			CNetAddr(const struct in_addr& ipv4Addr);
			explicit CNetAddr(const char *pszIp, bool fAllowLookup = false);
			explicit CNetAddr(const std::string &strIp, bool fAllowLookup = false);
			void Init();
			void SetIP(const CNetAddr& ip);
			bool IsIPv4() const;    // IPv4 mapped address (::FFFF:0:0/96, 0.0.0.0/0)
			bool IsRFC1918() const; // IPv4 private networks (10.0.0.0/8, 192.168.0.0/16, 172.16.0.0/12)
			bool IsRFC3849() const; // IPv6 documentation address (2001:0DB8::/32)
			bool IsRFC3927() const; // IPv4 autoconfig (169.254.0.0/16)
			bool IsRFC3964() const; // IPv6 6to4 tunneling (2002::/16)
			bool IsRFC4193() const; // IPv6 unique local (FC00::/15)
			bool IsRFC4380() const; // IPv6 Teredo tunneling (2001::/32)
			bool IsRFC4843() const; // IPv6 ORCHID (2001:10::/28)
			bool IsRFC4862() const; // IPv6 autoconfig (FE80::/64)
			bool IsRFC6052() const; // IPv6 well-known prefix (64:FF9B::/96)
			bool IsRFC6145() const; // IPv6 IPv4-translated address (::FFFF:0:0:0/96)
			bool IsLocal() const;
			bool IsRoutable() const;
			bool IsValid() const;
			bool IsMulticast() const;
			std::string ToString() const;
			std::string ToStringIP() const;
			int GetByte(int n) const;
			uint64 GetHash() const;
			bool GetInAddr(struct in_addr* pipv4Addr) const;
			std::vector<unsigned char> GetGroup() const;
			void print() const;

	#ifdef USE_IPV6
			CNetAddr(const struct in6_addr& pipv6Addr);
			bool GetIn6Addr(struct in6_addr* pipv6Addr) const;
	#endif

			friend bool operator==(const CNetAddr& a, const CNetAddr& b);
			friend bool operator!=(const CNetAddr& a, const CNetAddr& b);
			friend bool operator<(const CNetAddr& a, const CNetAddr& b);

			IMPLEMENT_SERIALIZE
				(
				 READWRITE(FLATDATA(ip));
				)
	};

	/** A combination of a network address (CNetAddr) and a (TCP) port */
	class CService : public CNetAddr
	{
		protected:
			unsigned short port; // host order

		public:
			CService();
			CService(const CNetAddr& ip, unsigned short port);
			CService(const struct in_addr& ipv4Addr, unsigned short port);
			CService(const struct sockaddr_in& addr);
			explicit CService(const char *pszIpPort, int portDefault, bool fAllowLookup = false);
			explicit CService(const char *pszIpPort, bool fAllowLookup = false);
			explicit CService(const std::string& strIpPort, int portDefault, bool fAllowLookup = false);
			explicit CService(const std::string& strIpPort, bool fAllowLookup = false);
			void Init();
			void SetPort(unsigned short portIn);
			unsigned short GetPort() const;
			bool GetSockAddr(struct sockaddr_in* paddr) const;
			friend bool operator==(const CService& a, const CService& b);
			friend bool operator!=(const CService& a, const CService& b);
			friend bool operator<(const CService& a, const CService& b);
			std::vector<unsigned char> GetKey() const;
			std::string ToString() const;
			std::string ToStringPort() const;
			std::string ToStringIPPort() const;
			void print() const;

	#ifdef USE_IPV6
			CService(const struct in6_addr& ipv6Addr, unsigned short port);
			bool GetSockAddr6(struct sockaddr_in6* paddr) const;
			CService(const struct sockaddr_in6& addr);
	#endif

			IMPLEMENT_SERIALIZE
			(
				 CService* pthis = const_cast<CService*>(this);
				 READWRITE(FLATDATA(ip));
				 unsigned short portN = htons(port);
				 READWRITE(portN);
				 if (fRead)
					 pthis->port = ntohs(portN);
			)
	};

}

#endif // __INCLUDED_PROTOCOL_H__
