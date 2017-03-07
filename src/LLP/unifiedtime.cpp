/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/unifiedtime.h"
#include "../LLP/client.h"
#include <inttypes.h>


using namespace std;


/** Flag Declarations **/
bool fTimeUnified = false;
bool fIsTimeSeed = false;
bool fIsSeedNode = false;


int UNIFIED_AVERAGE_OFFSET = 0;
int UNIFIED_MOVING_ITERATOR = 0;


/** Unified Time Declarations **/
vector<int> UNIFIED_TIME_DATA;
vector<LLP::CAddress> SEED_NODES;

/** Seed Nodes for Unified Time. **/
vector<string> SEEDS;

/* Maximum number of seconds that a clock can be from one another. */
int MAX_UNIFIED_DRIFT		= 1;

/* Maximum numver of samples in the Unified Time moving Average. */
int MAX_UNIFIED_SAMPLES		= 200;

/* Maximum Frequency in which the Node will be looking up time samples. */
int MAX_UNIFIED_FREQUENCY	= 600;

/** Gets the UNIX Timestamp from your Local Clock **/
int64 GetLocalTimestamp(){ return time(NULL); }

/** Gets the UNIX Timestamp from the Nexus Network **/
int64 GetUnifiedTimestamp(){ return GetLocalTimestamp() + UNIFIED_AVERAGE_OFFSET; }

/** Called from Thread Time Regulator.
    This keeps time keeping separate from regular processing. **/
void InitializeUnifiedTime()
{
	printf("***** Unified Time Initialized to %i\n", UNIFIED_AVERAGE_OFFSET);
	
	//CreateThread(ThreadTimeRegulator,  NULL); TODO: Make this more intelligent with multiple tries to ensure that computer lagging won't give it false readings
	CreateThread(ThreadUnifiedSamples, NULL);
}


/* Calculate the Average Unified Time. Called after Time Data is Added */
int GetUnifiedAverage()
{
	if(UNIFIED_TIME_DATA.empty())
		return UNIFIED_AVERAGE_OFFSET;
		
	int nAverage = 0;
	for(int index = 0; index < std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()); index++)
		nAverage += UNIFIED_TIME_DATA[index];
		
	return round(nAverage / (double) std::min(MAX_UNIFIED_SAMPLES, (int)UNIFIED_TIME_DATA.size()));
}

/* This method will make sure the clock isn't changed at any point. 
	TODO: Assess the ways this can be attacked and make more intelligent to distinguish a real attacker from a real user. */
void ThreadTimeRegulator(void* parg)
{
	while(!fShutdown)
	{
		/* Regulate the Clock while Waiting, and Break if the Clock Changes. */
		int64 nTimestamp = GetLocalTimestamp();
		Sleep(10000);
		
		if(!fTimeUnified)
			continue;
					
		int64 nElapsed = GetLocalTimestamp() - nTimestamp;
		if(nElapsed > (MAX_UNIFIED_DRIFT + 10) || nElapsed < ((MAX_UNIFIED_DRIFT - 10) * -1))
		{
			UNIFIED_TIME_DATA.clear();
			UNIFIED_AVERAGE_OFFSET -= nElapsed;
						
			printf("***** LLP Clock Regulator: Time Changed by %"PRId64" Seconds. New Offset %i\n", nElapsed, UNIFIED_AVERAGE_OFFSET);
		}
	}
}


/** Regulator of the Unified Clock **/
void ThreadUnifiedSamples(void* parg)
{
	/** Compile the Seed Nodes into a set of Vectors. **/
	SEED_NODES    = LLP::DNS_Lookup(fTestNet ? LLP::DNS_SeedNodes_Testnet : LLP::DNS_SeedNodes);
	
	/* If the node happens to be offline, wait and recursively attempt to get the DNS seeds. */
	if(SEED_NODES.empty()) {
		Sleep(10000);
		
		ThreadUnifiedSamples(parg);
	}
	
	/** Iterator to be used to ensure every time seed is giving an equal weight towards the Global Seeds. **/
	int nIterator = -1;
	
	for(int nIndex = 0; nIndex < SEED_NODES.size(); nIndex++)
		SEEDS.push_back(SEED_NODES[nIndex].ToStringIP());
	
	/** The Entry Client Loop for Core LLP. **/
	string ADDRESS = "";
	LLP::CoreOutbound SERVER("", strprintf("%u", (fTestNet ? TESTNET_CORE_LLP_PORT : NEXUS_CORE_LLP_PORT)));
	loop
	{
		try
		{
		
			/* Increment the Time Seed Connection Iterator. */
			nIterator++;
			
			
			/* Reset the ITerator if out of Seeds. */
			if(nIterator == SEEDS.size())
			   nIterator = 0;
				
				
			/* Connect to the Next Seed in the Iterator. */
			SERVER.IP = SEEDS[nIterator];
			SERVER.Connect();
			
			
			/* If the Core LLP isn't connected, Retry in 10 Seconds. */
			if(!SERVER.Connected())
			{
				printf("***** Core LLP: Failed To Connect To %s:%s\n", SERVER.IP.c_str(), SERVER.PORT.c_str());
				
				continue;
			}

			
			/* Use a CMajority to Find the Sample with the Most Weight. */
			CMajority<int> nSamples;
			
			
			/* Get 10 Samples From Server. */
			SERVER.GetOffset((unsigned int)GetLocalTimestamp());
				
				
			/* Read the Samples from the Server. */
			while(SERVER.Connected() && !SERVER.Errors() && !SERVER.Timeout(5))
			{
				Sleep(1);
			
				SERVER.ReadPacket();
				if(SERVER.PacketComplete())
				{
					LLP::Packet PACKET = SERVER.NewPacket();
					
					/** Add a New Sample each Time Packet Arrives. **/
					if(PACKET.HEADER == SERVER.TIME_OFFSET)
					{
						int nOffset = bytes2int(PACKET.DATA);
						nSamples.Add(nOffset);
						
						SERVER.GetOffset((unsigned int)GetLocalTimestamp());
						
						if(GetArg("-verbose", 0) >= 2)
							printf("***** Core LLP: Added Sample %i | Seed %s\n", nOffset, SERVER.IP.c_str());
					}
					
					SERVER.ResetPacket();
				}
				
				/** Close the Connection Gracefully if Received all Packets. **/
				if(nSamples.Samples() == 9)
				{
					SERVER.Close();
					break;
				}
			}
			
			
			/* If there are no Successful Samples, Try another Connection. */
			if(nSamples.Samples() == 0)
			{
				printf("***** Core LLP: Failed To Get Time Samples.\n");
				SERVER.Close();
				
				continue;
			}
			
			/* These checks are for after the first time seed has been established. 
				TODO: Look at the possible attack vector of the first time seed being manipulated.
						This could be easily done by allowing the time seed to be created by X nodes and then check the drift. */
			if(fTimeUnified)
			{
			
				/* Check that the samples don't violate time changes too drastically. */
				if(nSamples.Majority() > GetUnifiedAverage() + MAX_UNIFIED_DRIFT ||
					nSamples.Majority() < GetUnifiedAverage() - MAX_UNIFIED_DRIFT ) {
				
					printf("***** Core LLP: Unified Samples Out of Drift Scope Current (%u) Samples (%u)\n", GetUnifiedAverage(), nSamples.Majority());
					SERVER.Close();
				
					continue;
				}
			}
			
			/* If the Moving Average is filled with samples, continue iterating to keep it moving. */
			if(UNIFIED_TIME_DATA.size() >= MAX_UNIFIED_SAMPLES)
			{
				if(UNIFIED_MOVING_ITERATOR >= MAX_UNIFIED_SAMPLES)
					UNIFIED_MOVING_ITERATOR = 0;
									
				UNIFIED_TIME_DATA[UNIFIED_MOVING_ITERATOR] = nSamples.Majority();
				UNIFIED_MOVING_ITERATOR ++;
			}
				
				
			/* If The Moving Average is filling, move the iterator along with the Time Data Size. */
			else
			{
				UNIFIED_MOVING_ITERATOR = UNIFIED_TIME_DATA.size();
				UNIFIED_TIME_DATA.push_back(nSamples.Majority());
			}
			

			/* Update Iterators and Flags. */
			if((UNIFIED_TIME_DATA.size() > 0))
			{
				fTimeUnified = true;
				UNIFIED_AVERAGE_OFFSET = GetUnifiedAverage();
				
				if(GetArg("-verbose", 0) >= 1)
					printf("***** %i Iterator | %i Offset | %i Current | %"PRId64"\n", UNIFIED_MOVING_ITERATOR, nSamples.Majority(), UNIFIED_AVERAGE_OFFSET, GetUnifiedTimestamp());
			}
			
			/* Sleep for the Unified Frequency Between Sample. */
			Sleep(MAX_UNIFIED_FREQUENCY * 1000);
		}
		catch(std::exception& e){ printf("UTM ERROR: %s\n", e.what()); }
	}
}



