/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_UTIL_INCLUDE_BYTES_H
#define NEXUS_UTIL_INCLUDE_BYTES_H

#include "debug.h"

/* Parse an IP Address into a Byte Vector from Std::String. */
inline std::vector<unsigned char> parse_ip(std::string ip)
{
	std::vector<unsigned char> bytes(4, 0);
	sscanf(ip.c_str(), "%hhu.%hhu.%hhu.%hhu", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
	
	return bytes;
}


inline std::string ip_string(std::vector<unsigned char> ip) { return strprintf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]); }


/* Convert a 32 bit Unsigned Integer to Byte Vector using Bitwise Shifts. */
inline std::vector<unsigned char> uint2bytes(unsigned int UINT)
{
	std::vector<unsigned char> BYTES(4, 0);
	BYTES[0] = UINT >> 24;
	BYTES[1] = UINT >> 16;
	BYTES[2] = UINT >> 8;
	BYTES[3] = UINT;
				
	return BYTES;
}


/* Convert a byte stream into a signed integer 32 bit. */	
inline int bytes2int(std::vector<unsigned char> BYTES, int nOffset = 0) { return (BYTES[0 + nOffset] << 24) + (BYTES[1 + nOffset] << 16) + (BYTES[2 + nOffset] << 8) + BYTES[3 + nOffset]; }
		

/* Convert a 32 bit signed Integer to Byte Vector using Bitwise Shifts. */
inline std::vector<unsigned char> int2bytes(int INT)
{
	std::vector<unsigned char> BYTES(4, 0);
	BYTES[0] = INT >> 24;
	BYTES[1] = INT >> 16;
	BYTES[2] = INT >> 8;
	BYTES[3] = INT;
				
	return BYTES;
}
			
			
/* Convert a byte stream into unsigned integer 32 bit. */	
inline unsigned int bytes2uint(std::vector<unsigned char> BYTES, int nOffset = 0) { return (BYTES[0 + nOffset] << 24) + (BYTES[1 + nOffset] << 16) + (BYTES[2 + nOffset] << 8) + BYTES[3 + nOffset]; }		
			
			
/* Convert a 64 bit Unsigned Integer to Byte Vector using Bitwise Shifts. */
inline std::vector<unsigned char> uint2bytes64(uint64 UINT)
{
	std::vector<unsigned char> INTS[2];
	INTS[0] = uint2bytes((unsigned int) UINT);
	INTS[1] = uint2bytes((unsigned int) (UINT >> 32));
				
	std::vector<unsigned char> BYTES;
	BYTES.insert(BYTES.end(), INTS[0].begin(), INTS[0].end());
	BYTES.insert(BYTES.end(), INTS[1].begin(), INTS[1].end());
				
	return BYTES;
}

			
/* Convert a byte Vector into unsigned integer 64 bit. */
inline uint64 bytes2uint64(std::vector<unsigned char> BYTES, int nOffset = 0) { return (bytes2uint(BYTES, nOffset) | ((uint64)bytes2uint(BYTES, nOffset + 4) << 32)); }


/* Convert Standard String into Byte Vector. */
inline std::vector<unsigned char> string2bytes(std::string STRING)
{
	std::vector<unsigned char> BYTES(STRING.begin(), STRING.end());
	return BYTES;
}


/* Convert Byte Vector into Standard String. */
inline std::string bytes2string(std::vector<unsigned char> BYTES)
{
	std::string STRING(BYTES.begin(), BYTES.end());
	return STRING;
}

#endif
