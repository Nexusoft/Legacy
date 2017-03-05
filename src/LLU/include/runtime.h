/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLU_INCLUDE_RUNTIME_H
#define NEXUS_LLU_INCLUDE_RUNTIME_H

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


/* Thread Type for easy reference. */
typedef boost::thread                                        Thread_t;
		

/* Sleep for a duration in Milliseconds. */
inline void Sleep(unsigned int nTime){ boost::this_thread::sleep(boost::posix_time::milliseconds(nTime)); }

		
/* Class the tracks the duration of time elapsed in seconds or milliseconds.
	Used for socket timers to determine time outs. */
class Timer
{
private:
	boost::posix_time::ptime TIMER_START, TIMER_END;
	bool fStopped = false;
		
public:
	void Start() { TIMER_START = boost::posix_time::microsec_clock::local_time(); fStopped = false; }
	void Reset() { Start(); }
	void Stop()  { TIMER_END = boost::posix_time::microsec_clock::local_time(); fStopped = true; }
			
	/* Return the Total Seconds Elapsed Since Timer Started. */
	unsigned int Elapsed()
	{
		if(fStopped)
			return (TIMER_END - TIMER_START).total_seconds();
					
		return (boost::posix_time::microsec_clock::local_time() - TIMER_START).total_seconds();
	}
			
	/* Return the Total Milliseconds Elapsed Since Timer Started. */
	unsigned int ElapsedMilliseconds()
	{
		if(fStopped)
			return (TIMER_END - TIMER_START).total_milliseconds();
					
		return (boost::posix_time::microsec_clock::local_time() - TIMER_START).total_milliseconds();
	}
};





// NOTE: It turns out we might have been able to use boost::thread
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
