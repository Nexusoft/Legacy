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

#endif
