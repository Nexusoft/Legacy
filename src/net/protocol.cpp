/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "protocol.h"
#include "../LLU/util.h"
#include "netbase.h"

#ifndef WIN32
# include <arpa/inet.h>
#endif

namespace Net
{

	/** For Nexus the 4 bytes that make the message header will be Fibanacci Primes [5, 13, 89, 233] **/
	static unsigned char pchMessageStartNexusTest[4] = { 0xe9, 0x59, 0x0d, 0x05 };
	static unsigned char pchMessageStartNexus[4] =     { 0x05, 0x0d, 0x59, 0xe9 };

	void GetMessageStart(unsigned char pchMessageStart[])
	{
		if (fTestNet)
			memcpy(pchMessageStart, pchMessageStartNexusTest, sizeof(pchMessageStartNexusTest));
		else
			memcpy(pchMessageStart, pchMessageStartNexus, sizeof(pchMessageStartNexus));
	}

	static const char* ppszTypeName[] =
	{
		"ERROR",
		"tx",
		"block",
	};

	CMessageHeader::CMessageHeader()
	{
		GetMessageStart(pchMessageStart);
		memset(pchCommand, 0, sizeof(pchCommand));
		pchCommand[1] = 1;
		nMessageSize = -1;
		nChecksum = 0;
	}

	CMessageHeader::CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn)
	{
		GetMessageStart(pchMessageStart);
		strncpy(pchCommand, pszCommand, COMMAND_SIZE);
		nMessageSize = nMessageSizeIn;
		nChecksum = 0;
	}

	std::string CMessageHeader::GetCommand() const
	{
		if (pchCommand[COMMAND_SIZE-1] == 0)
			return std::string(pchCommand, pchCommand + strlen(pchCommand));
		else
			return std::string(pchCommand, pchCommand + COMMAND_SIZE);
	}

	bool CMessageHeader::IsValid() const
	{
		// Check start string
		unsigned char pchMessageStartProtocol[4];
		GetMessageStart(pchMessageStartProtocol);
		if (memcmp(pchMessageStart, pchMessageStartProtocol, sizeof(pchMessageStart)) != 0)
			return false;

		// Check the command string for errors
		for (const char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
		{
			if (*p1 == 0)
			{
				// Must be all zeros after the first zero
				for (; p1 < pchCommand + COMMAND_SIZE; p1++)
					if (*p1 != 0)
						return false;
			}
			else if (*p1 < ' ' || *p1 > 0x7E)
				return false;
		}

		// Message size
		if (nMessageSize > MAX_SIZE)
		{
			printf("CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
			return false;
		}

		return true;
	}



	CAddress::CAddress() : CService()
	{
		Init();
	}

	CAddress::CAddress(CService ipIn, uint64 nServicesIn) : CService(ipIn)
	{
		Init();
		nServices = nServicesIn;
	}

	void CAddress::Init()
	{
		nServices = NODE_NETWORK;
		nTime = 100000000;
		nLastTry = 0;
	}

	CInv::CInv()
	{
		type = 0;
		hash = 0;
	}

	CInv::CInv(int typeIn, const uint1024& hashIn)
	{
		type = typeIn;
		hash = hashIn;
	}

	CInv::CInv(const std::string& strType, const uint1024& hashIn)
	{
		unsigned int i;
		for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
		{
			if (strType == ppszTypeName[i])
			{
				type = i;
				break;
			}
		}
		if (i == ARRAYLEN(ppszTypeName))
			throw std::out_of_range(strprintf("CInv::CInv(string, uint1024) : unknown type '%s'", strType.c_str()));
		hash = hashIn;
	}

	bool operator<(const CInv& a, const CInv& b)
	{
		return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
	}

	bool CInv::IsKnownType() const
	{
		return (type >= 1 && type < (int)ARRAYLEN(ppszTypeName));
	}

	const char* CInv::GetCommand() const
	{
		if (!IsKnownType())
			throw std::out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", type));
		return ppszTypeName[type];
	}

	std::string CInv::ToString() const
	{
		if(GetCommand() == "tx")
		{
			std::string invHash = hash.ToString();
			return strprintf("tx %s", invHash.substr(invHash.length() - 20, invHash.length()).c_str());
		}
			
		return strprintf("%s %s", GetCommand(), hash.ToString().substr(0,20).c_str());
	}

	void CInv::print() const
	{
		printf("CInv(%s)\n", ToString().c_str());
	}

}

