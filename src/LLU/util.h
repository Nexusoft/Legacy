/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_UTIL_H
#define NEXUS_UTIL_H

#include "../LLH/templates.h"

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

#include "../net/netbase.h"


/** Linux Specific Work Around (For Now). **/
#if defined(MAC_OSX) || defined(WIN32)
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif


static const int64 COIN = 1000000;
static const int64 CENT = 10000;



#define loop                for (;;)
#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))
#define printf              OutputDebugStringF
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

/* Helps to parse out the individual bits from a byte to use multiple flags in a 0xff range. */
bool GetBit(unsigned char byte, int position) // position in range 0-7
{
	return (byte >> position) & 0x1;
}

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
	sscanf(ip.c_str(), "%hu.%hu.%hu.%hu", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
	
	return bytes;
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

/** Create a sorted Multimap for rich lists. **/
template <typename A, typename B> std::multimap<B, A> flip_map(std::map<A,B> & src) 
{
	std::multimap<B,A> dst;
	for(typename std::map<A, B>::const_iterator it = src.begin(); it != src.end(); ++it)
		dst.insert(std::pair<B, A>(it -> second, it -> first));

    return dst;
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
    /*Boost has a year 2038 problem— if the request sleep time is past epoch+2^31 seconds the sleep returns instantly.
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
extern int64 GetUnifiedTimestamp();


extern std::map<std::string, std::string> mapArgs;
extern std::map<std::string, std::vector<std::string> > mapMultiArgs;
extern bool fDebug;
extern bool fPrintToConsole;
extern bool fPrintToDebugger;
extern bool fRequestShutdown;
extern bool fShutdown;
extern bool fDaemon;
extern bool fServer;
extern bool fCommandLine;
extern std::string strMiscWarning;
extern bool fTestNet;
extern bool fNoListen;
extern bool fLogTimestamps;

void RandAddSeed();
void RandAddSeedPerfmon();
int OutputDebugStringF(const char* pszFormat, ...);
int my_snprintf(char* buffer, size_t limit, const char* format, ...);

/* It is not allowed to use va_start with a pass-by-reference argument.
   (C++ standard, 18.7, paragraph 3). Use a dummy argument to work around this, and use a
   macro to keep similar semantics.
*/

/** Global List for Debugging Output. **/
extern boost::mutex DEBUGGING_MUTEX;
extern std::vector<std::pair<bool, std::string> > DEBUGGING_OUTPUT;

std::string real_strprintf(const std::string &format, int dummy, ...);
void debug_server(std::string strOutput, bool fGeneric);
void DebugThread(void* parg);

#define strprintf(format, ...) real_strprintf(format, 0, __VA_ARGS__)
#define printd(format, ...) debug_server(real_strprintf(format, 0, __VA_ARGS__), 0)
#define printg(format, ...) debug_server(real_strprintf(format, 0, __VA_ARGS__), 1)

inline std::string ip_string(std::vector<unsigned char> ip) { return strprintf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]); }

bool error(const char *format, ...);
void LogException(std::exception* pex, const char* pszThread);
void PrintException(std::exception* pex, const char* pszThread);
void PrintExceptionContinue(std::exception* pex, const char* pszThread);
void ParseString(const std::string& str, char c, std::vector<std::string>& v);
std::string FormatMoney(int64 n, bool fPlus=false);
bool ParseMoney(const std::string& str, int64& nRet);
bool ParseMoney(const char* pszIn, int64& nRet);
std::vector<unsigned char> ParseHex(const char* psz);
std::vector<unsigned char> ParseHex(const std::string& str);
bool IsHex(const std::string& str);
std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid = NULL);
std::string DecodeBase64(const std::string& str);
std::string EncodeBase64(const unsigned char* pch, size_t len);
std::string EncodeBase64(const std::string& str);
void ParseParameters(int argc, const char*const argv[]);
bool WildcardMatch(const char* psz, const char* mask);
bool WildcardMatch(const std::string& str, const std::string& mask);

std::vector<std::string> Split(const std::string& strInput, char strDelimiter);
bool CheckPermissions(std::string strAddress, unsigned int nPort);

int GetFilesize(FILE* file);
boost::filesystem::path GetDefaultDataDir(std::string strName = "Nexus");
const boost::filesystem::path &GetDataDir(bool fNetSpecific = true);
boost::filesystem::path GetConfigFile();
boost::filesystem::path GetPidFile();
void CreatePidFile(const boost::filesystem::path &path, pid_t pid);
void ReadConfigFile(std::map<std::string, std::string>& mapSettingsRet, std::map<std::string, std::vector<std::string> >& mapMultiSettingsRet);
bool GetStartOnSystemStartup();
bool SetStartOnSystemStartup(bool fAutoStart);
void ShrinkDebugFile();
int GetRandInt(int nMax);
uint64 GetRand(uint64 nMax);
uint256 GetRand256();
uint512 GetRand512();
std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);

bool copyDir(boost::filesystem::path const & source, boost::filesystem::path const & destination);





/** Wrapped boost mutex: supports recursive locking, but no waiting  */
typedef boost::interprocess::interprocess_recursive_mutex CCriticalSection;

/** Wrapped boost mutex: supports waiting but not recursive locking */
typedef boost::interprocess::interprocess_mutex CWaitableCriticalSection;

#ifdef DEBUG_LOCKORDER
void EnterCritical(const char* pszName, const char* pszFile, int nLine, void* cs, bool fTry = false);
void LeaveCritical();
#else
void static inline EnterCritical(const char* pszName, const char* pszFile, int nLine, void* cs, bool fTry = false) {}
void static inline LeaveCritical() {}
#endif

/** Wrapper around boost::interprocess::scoped_lock */
template<typename Mutex>
class CMutexLock
{
private:
    boost::interprocess::scoped_lock<Mutex> lock;
public:

    void Enter(const char* pszName, const char* pszFile, int nLine)
    {
        if (!lock.owns())
        {
            EnterCritical(pszName, pszFile, nLine, (void*)(lock.mutex()));
#ifdef DEBUG_LOCKCONTENTION
            if (!lock.try_lock())
            {
                printf("LOCKCONTENTION: %s\n", pszName);
                printf("Locker: %s:%d\n", pszFile, nLine);
#endif
            lock.lock();
#ifdef DEBUG_LOCKCONTENTION
            }
#endif
        }
    }

    void Leave()
    {
        if (lock.owns())
        {
            lock.unlock();
            LeaveCritical();
        }
    }

    bool TryEnter(const char* pszName, const char* pszFile, int nLine)
    {
        if (!lock.owns())
        {
            EnterCritical(pszName, pszFile, nLine, (void*)(lock.mutex()), true);
            lock.try_lock();
            if (!lock.owns())
                LeaveCritical();
        }
        return lock.owns();
    }

    CMutexLock(Mutex& mutexIn, const char* pszName, const char* pszFile, int nLine, bool fTry = false) : lock(mutexIn, boost::interprocess::defer_lock)
    {
        if (fTry)
            TryEnter(pszName, pszFile, nLine);
        else
            Enter(pszName, pszFile, nLine);
    }

    ~CMutexLock()
    {
        if (lock.owns())
            LeaveCritical();
    }

    operator bool()
    {
        return lock.owns();
    }

    boost::interprocess::scoped_lock<Mutex> &GetLock()
    {
        return lock;
    }
};

typedef CMutexLock<CCriticalSection> CCriticalBlock;

#define LOCK(cs) CCriticalBlock criticalblock(cs, #cs, __FILE__, __LINE__)
#define LOCK2(cs1,cs2) CCriticalBlock criticalblock1(cs1, #cs1, __FILE__, __LINE__),criticalblock2(cs2, #cs2, __FILE__, __LINE__)
#define TRY_LOCK(cs,name) CCriticalBlock name(cs, #cs, __FILE__, __LINE__, true)

#define ENTER_CRITICAL_SECTION(cs) \
    { \
        EnterCritical(#cs, __FILE__, __LINE__, (void*)(&cs)); \
        (cs).lock(); \
    }

#define LEAVE_CRITICAL_SECTION(cs) \
    { \
        (cs).unlock(); \
        LeaveCritical(); \
    }

#ifdef MAC_OSX
// boost::interprocess::interprocess_semaphore seems to spinlock on OSX; prefer polling instead
class CSemaphore
{
private:
    CCriticalSection cs;
    int val;

public:
    CSemaphore(int init) : val(init) {}

    void wait() {
        do {
            {
                LOCK(cs);
                if (val>0) {
                    val--;
                    return;
                }
            }
            Sleep(100);
        } while(1);
    }

    bool try_wait() {
        LOCK(cs);
        if (val>0) {
            val--;
            return true;
        }
        return false;
    }

    void post() {
        LOCK(cs);
        val++;
    }
};
#else
typedef boost::interprocess::interprocess_semaphore CSemaphore;
#endif

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

template<typename T>
std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
    std::vector<char> rv;
    static char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    rv.reserve((itend-itbegin)*3);
    for(T it = itbegin; it < itend; ++it)
    {
        unsigned char val = (unsigned char)(*it);
        if(fSpaces && it != itbegin)
            rv.push_back(' ');
        rv.push_back(hexmap[val>>4]);
        rv.push_back(hexmap[val&15]);
    }

    return std::string(rv.begin(), rv.end());
}

inline std::string HexStr(const std::vector<unsigned char>& vch, bool fSpaces=false)
{
    return HexStr(vch.begin(), vch.end(), fSpaces);
}

template<typename T>
void PrintHex(const T pbegin, const T pend, const char* pszFormat="%s", bool fSpaces=true)
{
    printf(pszFormat, HexStr(pbegin, pend, fSpaces).c_str());
}

inline void PrintHex(const std::vector<unsigned char>& vch, const char* pszFormat="%s", bool fSpaces=true)
{
    printf(pszFormat, HexStr(vch, fSpaces).c_str());
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

/**
 * Return string argument or default value
 *
 * @param strArg Argument to get (e.g. "-foo")
 * @param default (e.g. "1")
 * @return command-line argument or default value
 */
std::string GetArg(const std::string& strArg, const std::string& strDefault);

/**
 * Return integer argument or default value
 *
 * @param strArg Argument to get (e.g. "-foo")
 * @param default (e.g. 1)
 * @return command-line argument (0 if invalid number) or default value
 */
int64 GetArg(const std::string& strArg, int64 nDefault);

/**
 * Return boolean argument or default value
 *
 * @param strArg Argument to get (e.g. "-foo")
 * @param default (true or false)
 * @return command-line argument or default value
 */
bool GetBoolArg(const std::string& strArg, bool fDefault=false);

/**
 * Set an argument if it doesn't already have a value
 *
 * @param strArg Argument to set (e.g. "-foo")
 * @param strValue Value (e.g. "1")
 * @return true if argument gets set, false if it already had a value
 */
bool SoftSetArg(const std::string& strArg, const std::string& strValue);

/**
 * Set a boolean argument if it doesn't already have a value
 *
 * @param strArg Argument to set (e.g. "-foo")
 * @param fValue Value (e.g. false)
 * @return true if argument gets set, false if it already had a value
 */
bool SoftSetBoolArg(const std::string& strArg, bool fValue);




#define IMPLEMENT_RANDOMIZE_STACK(ThreadFn)     \
    {                                           \
        static char nLoops;                     \
        if (nLoops <= 0)                        \
            nLoops = GetRand(20) + 1;           \
        if (nLoops-- > 1)                       \
        {                                       \
            ThreadFn;                           \
            return;                             \
        }                                       \
    }


	
/** Serialize Hash: Used to Serialize a CTransaction class in order to obtain the Tx Hash. Utilizes CDataStream to serialize the class. **/
template<typename T>
uint512 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    // Most of the time is spent allocating and deallocating CDataStream's
    // buffer.  If this ever needs to be optimized further, make a CStaticStream
    // class with its buffer on the stack.
    CDataStream ss(nType, nVersion);
    ss.reserve(10000);
    ss << obj;
    return SK512(ss.begin(), ss.end());
}


/** Average Filter used to give the Numerical Average over given set of values. **/
template <typename type> class CAverage
{
private:
	std::vector<type> vList;
	
public:
	CAverage(){}
	
	void Add(type value)
	{
		vList.push_back(value);
	}
	
	type Average()
	{
		type average = 0;
		for(int nIndex = 0; nIndex < vList.size(); nIndex++)
			average += vList[nIndex];
			
		return round(average / (double)vList.size());
	}
};


/** Filter designed to give the majority of set of values.
	Keeps count of every addition of template paramter type, 
	in order to give a reasonable majority of votes. **/
template <typename CType> class CMajority
{
private:
	std::map<CType, int> mapList;
	unsigned int nSamples;
	
public:
	CMajority() : nSamples(0) {}
	
	
	/** Add another Element to the Majority Count. **/
	void Add(CType value)
	{
		if(!mapList.count(value))
			mapList[value] = 1;
		else
			mapList[value]++;
			
		nSamples++;
	}
	
	
	/** Return the total number of samples this container holds. **/
	unsigned int Samples(){ return nSamples; }
	
	
	/** Return the Element of Type that has the highest Majority. **/
	CType Majority()
	{
		if(nSamples == 0)
			return 0;
			
		/** Temporary Reference Variable to store the largest majority to then compare every element of the map to it. **/
		std::pair<CType, int> nMajority;
		
		
		for(typename std::map<CType, int>::iterator nIterator = mapList.begin(); nIterator != mapList.end(); ++nIterator)
		{
			/** Set the return to be the first element, to then compare the rest of the map to it. **/
			if(nIterator == mapList.begin())
			{
				nMajority = std::make_pair(nIterator->first, nIterator->second);
				continue;
			}
			
			/** If a record has higher count, use that one. **/
			if(nIterator->second > nMajority.second)
			{
				nMajority.first = nIterator->first;
				nMajority.second = nIterator->second;
			}
		}
		
		return nMajority.first;
	}
};





// Note: It turns out we might have been able to use boost::thread
// by using TerminateThread(boost::thread.native_handle(), 0);
#ifdef WIN32
typedef HANDLE bitcoin_pthread_t;

inline bitcoin_pthread_t CreateThread(void(*pfn)(void*), void* parg, bool fWantHandle=false)
{
    DWORD nUnused = 0;
    HANDLE hthread =
        CreateThread(
            NULL,                        // default security
            0,                           // inherit stack size from parent
            (LPTHREAD_START_ROUTINE)pfn, // function pointer
            parg,                        // argument
            0,                           // creation option, start immediately
            &nUnused);                   // thread identifier
    if (hthread == NULL)
    {
        printf("Error: CreateThread() returned %d\n", GetLastError());
        return (bitcoin_pthread_t)0;
    }
    if (!fWantHandle)
    {
        CloseHandle(hthread);
        return (bitcoin_pthread_t)-1;
    }
    return hthread;
}

inline void SetThreadPriority(int nPriority)
{
    SetThreadPriority(GetCurrentThread(), nPriority);
}
#else
inline pthread_t CreateThread(void(*pfn)(void*), void* parg, bool fWantHandle=false)
{
    pthread_t hthread = 0;
    int ret = pthread_create(&hthread, NULL, (void*(*)(void*))pfn, parg);
    if (ret != 0)
    {
        printf("Error: pthread_create() returned %d\n", ret);
        return (pthread_t)0;
    }
    if (!fWantHandle)
    {
        pthread_detach(hthread);
        return (pthread_t)-1;
    }
    return hthread;
}

#define THREAD_PRIORITY_LOWEST          PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL    2
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_ABOVE_NORMAL    0

inline void SetThreadPriority(int nPriority)
{
    // It's unclear if it's even possible to change thread priorities on Linux,
    // but we really and truly need it for the generation threads.
#ifdef PRIO_THREAD
    setpriority(PRIO_THREAD, 0, nPriority);
#else
    setpriority(PRIO_PROCESS, 0, nPriority);
#endif
}

inline void ExitThread(size_t nExitCode)
{
    pthread_exit((void*)nExitCode);
}
#endif





inline uint32_t ByteReverse(uint32_t value)
{
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    return (value<<16) | (value>>16);
}

#endif

