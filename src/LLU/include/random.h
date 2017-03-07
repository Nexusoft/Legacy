/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLU_INCLUDE_RANDOM_H
#define NEXUS_LLU_INCLUDE_RANDOM_H

#include <openssl/rand.h>


/** Performance counter wrapper for Random Seed Generating. **/
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

/** Add a Seed to Random Functions. */
void RandAddSeed();
void RandAddSeedPerfmon();

/* Generate Random Number. */
int GetRandInt(int nMax);

/* Generate Random Number. */
int GetRandInt(int nMax);

/* Get random 64 bit number. */
uint64 GetRand(uint64 nMax);

/* Get random 256 bit number. */
uint256 GetRand256();

/* Get a random 512 bit number. */
uint512 GetRand512();

#endif
