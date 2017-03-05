/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_UTIL_H
#define NEXUS_UTIL_H

#include "../LLC/hash/SK.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
typedef int pid_t; /* define for windows compatiblity */
#endif
#include <map>
#include <vector>
#include <string>
#include <stdint.h>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/interprocess/sync/interprocess_recursive_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/lock_options.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "../LLP/include/network.h"


/** Linux Specific Work Around (For Now). **/
#if defined(MAC_OSX) || defined(WIN32)
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif





#define loop                for (;;)
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))

#define NULL                0

#ifdef snprintf
#undef snprintf
#endif
#define snprintf my_snprintf

#ifndef PRI64d
#if defined(_MSC_VER) || defined(__MSVCRT__)
#define PRI64d  "I64d"
#define PRI64u  "I64u"
#define PRI64x  "I64x"
#else
#define PRI64d  "lld"
#define PRI64u  "llu"
#define PRI64x  "llx"
#endif
#endif

// This is needed because the foreach macro can't get over the comma in pair<t1, t2>
#define PAIRTYPE(t1, t2)    std::pair<t1, t2>

// Align by increasing pointer, must have extra space at end of buffer
template <size_t nBytes, typename T>
T* alignup(T* p)
{
    union
    {
        T* ptr;
        size_t n;
    } u;
    u.ptr = p;
    u.n = (u.n + (nBytes-1)) & ~(nBytes-1);
    return u.ptr;
}

/** Parse an IP Address into a Byte Vector from Std::String. **/
inline std::vector<unsigned char> parse_ip(std::string ip)
{
	std::vector<unsigned char> bytes(4, 0);
	sscanf(ip.c_str(), "%hhu.%hhu.%hhu.%hhu", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
	
	return bytes;
}

/** Create a sorted Multimap for rich lists. **/
template <typename A, typename B> std::multimap<B, A> flip_map(std::map<A,B> & src) 
{
	std::multimap<B,A> dst;
	for(typename std::map<A, B>::const_iterator it = src.begin(); it != src.end(); ++it)
		dst.insert(std::pair<B, A>(it -> second, it -> first));

    return dst;
}

/** Convert a 32 bit Unsigned Integer to Byte Vector using Bitwise Shifts. **/
inline std::vector<unsigned char> uint2bytes(unsigned int UINT)
{
	std::vector<unsigned char> BYTES(4, 0);
	BYTES[0] = UINT >> 24;
	BYTES[1] = UINT >> 16;
	BYTES[2] = UINT >> 8;
	BYTES[3] = UINT;
				
	return BYTES;
}


/** Convert a byte stream into a signed integer 32 bit. **/	
inline int bytes2int(std::vector<unsigned char> BYTES, int nOffset = 0) { return (BYTES[0 + nOffset] << 24) + (BYTES[1 + nOffset] << 16) + (BYTES[2 + nOffset] << 8) + BYTES[3 + nOffset]; }
		

/** Convert a 32 bit signed Integer to Byte Vector using Bitwise Shifts. **/
inline std::vector<unsigned char> int2bytes(int INT)
{
	std::vector<unsigned char> BYTES(4, 0);
	BYTES[0] = INT >> 24;
	BYTES[1] = INT >> 16;
	BYTES[2] = INT >> 8;
	BYTES[3] = INT;
				
	return BYTES;
}
			
			
/** Convert a byte stream into unsigned integer 32 bit. **/	
inline unsigned int bytes2uint(std::vector<unsigned char> BYTES, int nOffset = 0) { return (BYTES[0 + nOffset] << 24) + (BYTES[1 + nOffset] << 16) + (BYTES[2 + nOffset] << 8) + BYTES[3 + nOffset]; }		
			
			
/** Convert a 64 bit Unsigned Integer to Byte Vector using Bitwise Shifts. **/
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

			
/** Convert a byte Vector into unsigned integer 64 bit. **/
inline uint64 bytes2uint64(std::vector<unsigned char> BYTES, int nOffset = 0) { return (bytes2uint(BYTES, nOffset) | ((uint64)bytes2uint(BYTES, nOffset + 4) << 32)); }


/** Convert Standard String into Byte Vector. **/
inline std::vector<unsigned char> string2bytes(std::string STRING)
{
	std::vector<unsigned char> BYTES(STRING.begin(), STRING.end());
	return BYTES;
}


/** Convert Byte Vector into Standard String. **/
inline std::string bytes2string(std::vector<unsigned char> BYTES)
{
	std::string STRING(BYTES.begin(), BYTES.end());
	return STRING;
}

#ifdef WIN32
#define MSG_NOSIGNAL        0
#define MSG_DONTWAIT        0

#ifndef S_IRUSR
#define S_IRUSR             0400
#define S_IWUSR             0200
#endif
#define unlink              _unlink
#else
#define _vsnprintf(a,b,c,d) vsnprintf(a,b,c,d)
#define strlwr(psz)         to_lower(psz)
#define _strlwr(psz)        to_lower(psz)
#define MAX_PATH            1024
inline void Sleep(int64 n)
{
    /* Boost has a year 2038 problem— if the request sleep time is past epoch+2^31 seconds the sleep returns instantly.
       So we clamp our sleeps here to 10 years and hope that boost is fixed by 2028.*/
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(n>315576000000LL?315576000000LL:n));
}
#endif

#ifndef THROW_WITH_STACKTRACE
#define THROW_WITH_STACKTRACE(exception)  \
{                                         \
    LogStackTrace();                      \
    throw (exception);                    \
}
void LogStackTrace();
#endif


/** Gets the UNIX Timestamp from the Nexus Network **/
extern int64 GetUnifiedTimestamp(bool fAdjusted = false);

void RandAddSeed();
void RandAddSeedPerfmon();


inline std::string ip_string(std::vector<unsigned char> ip) { return strprintf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]); }



void ParseString(const std::string& str, char c, std::vector<std::string>& v);
std::string FormatMoney(int64 n, bool fPlus=false);
bool ParseMoney(const std::string& str, int64& nRet);
bool ParseMoney(const char* pszIn, int64& nRet);

std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid = NULL);
std::string DecodeBase64(const std::string& str);
std::string EncodeBase64(const unsigned char* pch, size_t len);
std::string EncodeBase64(const std::string& str);
void ParseParameters(int argc, const char*const argv[]);
bool WildcardMatch(const char* psz, const char* mask);
bool WildcardMatch(const std::string& str, const std::string& mask);

std::vector<std::string> Split(const std::string& strInput, char strDelimiter);
bool CheckPermissions(std::string strAddress, unsigned int nPort);


int GetRandInt(int nMax);
uint64 GetRand(uint64 nMax);
uint256 GetRand256();
uint512 GetRand512();


bool copyDir(boost::filesystem::path const & source, boost::filesystem::path const & destination);



inline std::string i64tostr(int64 n)
{
    return strprintf("%"PRI64d, n);
}

inline std::string itostr(int n)
{
    return strprintf("%d", n);
}

inline int64 atoi64(const char* psz)
{
#ifdef _MSC_VER
    return _atoi64(psz);
#else
    return strtoll(psz, NULL, 10);
#endif
}

inline int64 atoi64(const std::string& str)
{
#ifdef _MSC_VER
    return _atoi64(str.c_str());
#else
    return strtoll(str.c_str(), NULL, 10);
#endif
}

inline int atoi(const std::string& str)
{
    return atoi(str.c_str());
}

inline int roundint(double d)
{
    return (int)(d > 0 ? d + 0.5 : d - 0.5);
}

inline int64 roundint64(double d)
{
    return (int64)(d > 0 ? d + 0.5 : d - 0.5);
}

inline int64 abs64(int64 n)
{
    return (n >= 0 ? n : -n);
}



inline int64 GetPerformanceCounter()
{
    int64 nCounter = 0;
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
    timeval t;
    gettimeofday(&t, NULL);
    nCounter = t.tv_sec * 1000000 + t.tv_usec;
#endif
    return nCounter;
}

inline int64 GetTimeMillis()
{
    return (boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time()) -
            boost::posix_time::ptime(boost::gregorian::date(1970,1,1))).total_milliseconds();
}

inline std::string DateTimeStrFormat(const char* pszFormat, int64 nTime)
{
    time_t n = nTime;
    struct tm* ptmTime = gmtime(&n);
    char pszTime[200];
    strftime(pszTime, sizeof(pszTime), pszFormat, ptmTime);
    return pszTime;
}

static const std::string strTimestampFormat = "%Y-%m-%d %H:%M:%S UTC";
inline std::string DateTimeStrFormat(int64 nTime)
{
    return DateTimeStrFormat(strTimestampFormat.c_str(), nTime);
}

template<typename T>
void skipspaces(T& it)
{
    while (isspace(*it))
        ++it;
}

inline bool IsSwitchChar(char c)
{
#ifdef WIN32
    return c == '-' || c == '/';
#else
    return c == '-';
#endif
}

#endif



