/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_NETWORK_H
#define NEXUS_LLP_INCLUDE_NETWORK_H

#include <stdint.h>

#if defined(MAC_OSX) || defined(WIN32)
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif

#ifdef WIN32
// In MSVC, this is defined as a macro, undefine it to prevent a compile and link error
#undef SetPort
#endif

namespace LLP
{
	
	
	/** Services flags */
	enum
	{
		NODE_NETWORK = (1 << 0),
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
	
	
	/** Extended statistics about a CAddress */
	class CAddrInfo : public CAddress
	{
	private:
		/* Who Gave us this Address. */
		CNetAddr source;

		
		/* The last time this connection was seen. */
		unsigned int nLastSuccess;

		
		/* The last time this node was tried. */
		unsigned int nLastAttempt;

		
		/* Number of attempts to connect since last try. */
		unsigned int nAttempts;

		

	public:

		IMPLEMENT_SERIALIZE(
			CAddress* pthis = (CAddress*)(this);
			READWRITE(*pthis);
			READWRITE(source);
			READWRITE(nLastSuccess);
			READWRITE(nAttempts);
		)

		void Init()
		{
			nLastSuccess = 0;
			nLastTry = 0;
			nAttempts = 0;
			nRefCount = 0;
			fInTried = false;
			nRandomPos = -1;
		}

		CAddrInfo(const CAddress &addrIn, const CNetAddr &addrSource) : CAddress(addrIn), source(addrSource)
		{
			Init();
		}

		CAddrInfo() : CAddress(), source()
		{
			Init();
		}
	};
	
	
	/* Proxy Settings for Nexus Core. */
	int fUseProxy = false;
	CService addrProxy("127.0.0.1", 9050);
}

#endif
