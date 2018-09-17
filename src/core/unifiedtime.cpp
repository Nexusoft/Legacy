/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "unifiedtime.h"
#include "../LLP/client.h"
#include <inttypes.h>
#include <string>
#include <vector>

using namespace std;


/** Flag Declarations **/
bool fTimeUnified = false;
bool fIsTimeSeed = false;
bool fIsSeedNode = false;


int UNIFIED_AVERAGE_OFFSET = 0;
int UNIFIED_MOVING_ITERATOR = 0;

const int UNIFEED_TIME_ADJUSTMENT = 8240;
const int UNIFIED_TIME_ADJUSTMENT_TESTNET = 144;
int TIME_ADJUSTED = 0;


/** Unified Time Declarations **/
std::vector<int> UNIFIED_TIME_DATA;
std::vector<Net::CAddress> SEED_NODES;

std::map<std::string, int> MAP_TIME_DATA;

/** Declarations for the DNS Seed Nodes. **/
static std::vector<std::string> DNS_SeedNodes =
{
    "node1.nexusearth.com",
    "node1.nexusoft.io",
    "node1.mercuryminer.com",
    "node1.nexusminingpool.com",
    "node1.nexus2.space",
    "node1.barbequemedia.com",
    "node1.nxsorbitalscan.com",
    "node1.nxs.efficienthash.com",
    "node1.henryskinner.net",
    "node1.nexplorer.io",
    "node1.positivism.trade",
    "node2.nexusearth.com",
    "node2.nexusoft.io",
    "node2.mercuryminer.com",
    "node2.nexusminingpool.com",
    "node2.nexus2.space",
    "node2.barbequemedia.com",
    "node2.nxsorbitalscan.com",
    "node2.nxs.efficienthash.com",
    "node2.henryskinner.net",
    "node3.nexusearth.com",
    "node3.nexusoft.io",
    "node3.mercuryminer.com",
    "node3.nexusminingpool.com",
    "node3.nexus2.space",
    "node3.barbequemedia.com",
    "node3.nxsorbitalscan.com",
    "node3.nxs.efficienthash.com",
    "node3.henryskinner.net",
    "node4.nexusearth.com",
    "node4.nexusoft.io",
    "node4.mercuryminer.com",
    "node4.nexus2.space",
    "node4.barbequemedia.com",
    "node4.nxsorbitalscan.com",
    "node4.nxs.efficienthash.com",
    "node4.henryskinner.net",
    "node5.nexusearth.com",
    "node5.nexusoft.io",
    "node5.mercuryminer.com",
    "node5.barbequemedia.com",
    "node5.nxs.efficienthash.com",
    "node5.henryskinner.net",
    "node6.nexusearth.com",
    "node6.mercuryminer.com",
    "node6.barbequemedia.com",
    "node6.nxs.efficienthash.com",
    "node6.henryskinner.net",
    "node7.nexusearth.com",
    "node7.mercuryminer.com",
    "node7.barbequemedia.com",
    "node7.nxs.efficienthash.com",
    "node7.henryskinner.net",
    "node8.nexusearth.com",
    "node8.mercuryminer.com",
    "node8.barbequemedia.com",
    "node8.nxs.efficienthash.com",
    "node8.henryskinner.net",
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
    "node14.mercuryminer.com",
    "node15.mercuryminer.com",
    "node16.mercuryminer.com",
    "node17.mercuryminer.com",
    "node18.mercuryminer.com",
    "node19.mercuryminer.com",
    "node20.mercuryminer.com",
    "node21.mercuryminer.com",
    "wallet5.ddns.net", //vido
    "wallet7.ddns.net",
    "wallet8.ddns.net"
};

/** Declarations for the DNS Seed Nodes. **/
static std::vector<std::string> DNS_SeedNodes_Testnet =
{
    "node1.nexusoft.io",
    "node2.nexusoft.io",
    "node3.nexusoft.io",
    "node4.nexusoft.io"
};


/** Declarations for the DNS Seed Nodes. **/
static std::vector<std::string> DNS_SeedNodes_LISPnet =
{
    "node1.nexus.lispers.net",
    "node2.nexus.lispers.net",
    "node3.nexus.lispers.net",
    "node4.nexus.lispers.net",
    "node5.nexus.lispers.net",
    "node6.nexus.lispers.net",
    "node7.nexus.lispers.net",
    "node8.nexus.lispers.net",
    "node9.nexus.lispers.net",
    "node10.nexus.lispers.net",
    "node11.nexus.lispers.net",
    "node12.nexus.lispers.net",
    "node13.nexus.lispers.net"
};

/** Seed Nodes for Unified Time. **/
std::vector<string> SEEDS;

/** Baseline Maximum Values for Unified Time. **/
int MAX_UNIFIED_DRIFT   = 10;
int MAX_UNIFIED_SAMPLES = 1000;


/** Gets the UNIX Timestamp from your Local Clock **/
int64 GetLocalTimestamp(){ return time(NULL); }


/** Gets the UNIX Timestamp from the Nexus Network **/
int64 GetUnifiedTimestamp(){ return GetLocalTimestamp() + UNIFIED_AVERAGE_OFFSET; }


/** Gets the Local Unversal time converted. **/
int64 UniversalTime(int64 nTimestamp) { return nTimestamp - UNIFIED_AVERAGE_OFFSET; }


/** Called from Thread Time Regulator.
    This keeps time keeping separate from regular processing. **/
void InitializeUnifiedTime()
{
    printf("***** Unified Time Initialized to %i\n", UNIFIED_AVERAGE_OFFSET);

    CreateThread(ThreadUnifiedSamples, NULL);
}


/** Regulator of the Unified Clock **/
void ThreadUnifiedSamples(void* parg)
{
    /* set the proper Thread priorities. */
    SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);

    /* Add Seed Nodes from command line parameter */
    if (mapArgs.count("-addseednode"))
    {
        BOOST_FOREACH(string& strAddSeed, mapMultiArgs["-addseednode"])
            (fLispNet ? DNS_SeedNodes_LISPnet : (fTestNet ? DNS_SeedNodes_Testnet : DNS_SeedNodes)).push_back(strAddSeed);
    }

    /* Compile the Seed Nodes into a set of Vectors. */
    SEED_NODES    = DNS_Lookup(fLispNet ? DNS_SeedNodes_LISPnet : (fTestNet ? DNS_SeedNodes_Testnet : DNS_SeedNodes));

    /* Iterator to be used to ensure every time seed is giving an equal weight towards the Global Seeds. */
    int nIterator = -1;
    for(int nIndex = 0; nIndex < SEED_NODES.size(); nIndex++)
        SEEDS.push_back(SEED_NODES[nIndex].ToStringIP());

    /* Move random shuffle here. */
    std::random_shuffle(SEED_NODES.begin(), SEED_NODES.end(), GetRandInt);
    for(int nIndex = 0; nIndex < SEED_NODES.size(); nIndex++)
        Net::addrman.Add(SEED_NODES[nIndex], (Net::CNetAddr)SEED_NODES[nIndex], true);

    /* The Entry Client Loop for Core LLP. */
    string ADDRESS = "";
    LLP::CoreOutbound SERVER("", strprintf("%u", fLispNet ? LISPNET_CORE_LLP_PORT : (fTestNet ? TESTNET_CORE_LLP_PORT : NEXUS_CORE_LLP_PORT)));

    /* Latency Timer. */
    loop()
    {
        try
        {
            Sleep(1000);

            /* Check for unified time adjustment. This code will be removed in 2.5.1 release - only needed for one time unified time sync rollback */
            if(GetUnifiedTimestamp() > (fTestNet ? Core::TESTNET_VERSION_TIMELOCK[3] : Core::NETWORK_VERSION_TIMELOCK[3]))
            {
                /* Get the time elapsed since the activation time-lock. */
                unsigned int nTimestamp = (GetUnifiedTimestamp() - (fTestNet ? Core::TESTNET_VERSION_TIMELOCK[3] : Core::NETWORK_VERSION_TIMELOCK[3]));

                /* Break this into ten minute increments for adjustment period. */
                unsigned int nTenMinutes = (unsigned int)(nTimestamp / 600.0); //the total time passed unified offset

                /* Add some time checking debug output. */ //TODO: for main release remove this debug output.
                if(nTenMinutes < (fTestNet ? UNIFIED_TIME_ADJUSTMENT_TESTNET : UNIFEED_TIME_ADJUSTMENT) + 1)
                    printf("***** Unified Time Check: time passed %u, ten mins %u, time adjusted %u, total to adjust %u\n", nTimestamp, nTenMinutes, TIME_ADJUSTED, (fTestNet ? UNIFIED_TIME_ADJUSTMENT_TESTNET : UNIFEED_TIME_ADJUSTMENT));

                //adjust the clock if within the span of minutes past the time-lock
                if(nTenMinutes < (fTestNet ? UNIFIED_TIME_ADJUSTMENT_TESTNET : UNIFEED_TIME_ADJUSTMENT) + 1 && nTenMinutes > TIME_ADJUSTED)
                {
                    /* Set the time adjusted to how many intervals of 10 minutes there have been. */
                    TIME_ADJUSTED = nTenMinutes;

                    /* Clear time samples to get the new offset. All time seeds should be rolled back 1 second at this point. */
                    MAP_TIME_DATA.clear();

                    /* Reduce the unified average offset by 1 second. */
                    UNIFIED_AVERAGE_OFFSET -= 1;

                    /* Debug output to show the clock adjustments. */
                    printf("***** Unified Time: New timespan at %u minutes, adjusting clock back one second (%i new offset). Remaining %i seconds\n", (nTimestamp / 60), UNIFIED_AVERAGE_OFFSET, ((fTestNet ? UNIFIED_TIME_ADJUSTMENT_TESTNET : UNIFEED_TIME_ADJUSTMENT) - TIME_ADJUSTED));

                    /* Sleep for 2 seconds then get seeds from other nodes again. */
                    Sleep(2000);
                }
            }


            /* Randomize the Time Seed Connection Iterator. */
            nIterator ++;
            if(nIterator == SEEDS.size())
                nIterator = 0;

            /* Connect to the Next Seed in the Iterator. */
            SERVER.IP = SEEDS[nIterator];
            SERVER.Connect();


            /* If the Core LLP isn't connected, Retry in 10 Seconds. */
            if(!SERVER.Connected())
            {   
                /* Debug output. */
                printf("***** Core LLP: Failed To Connect To %s:%s\n", SERVER.IP.c_str(), SERVER.PORT.c_str());
                 
                continue;
             }
            
            /* Use a CMajority to Find the Sample with the Most Weight. */
            CMajority<int> nSamples;


            /* Create Latency Timer Object. */
            LLP::Timer LatencyTimer;
            LatencyTimer.Reset();


            /* Request an initial offset from Unified Time servers. */
            SERVER.GetOffset((unsigned int)GetLocalTimestamp());


            /** Read the Samples from the Server. **/
            while(SERVER.Connected() && !SERVER.Errors() && !SERVER.Timeout(5))
            {
                Sleep(1);

                SERVER.ReadPacket();
                if(SERVER.PacketComplete())
                {
                    LLP::Packet PACKET = SERVER.NewPacket();

                    /* Add a New Sample each Time Packet Arrives. */
                    if(PACKET.HEADER == SERVER.TIME_OFFSET)
                    {

                        /* Calculate the Latency Round Trip Time. */
                        unsigned int nLatency = LatencyTimer.ElapsedMilliseconds();

                        /* Calculate this particular sample. */
                        int nOffset = bytes2int(PACKET.DATA) + (nLatency / 2000);
                        nSamples.Add(nOffset);

                        /* Reset the Timer and request another sample. */
                        LatencyTimer.Reset();
                        SERVER.GetOffset((unsigned int)GetLocalTimestamp());

                        if(GetArg("-verbose", 0) >= 2)
                            printf("***** Core LLP: Added Sample %i | Seed %s | Latency %u ms\n", nOffset, SERVER.IP.c_str(), nLatency);
                    }

                    SERVER.ResetPacket();
                }

                /* Close the Connection Gracefully if Received all Packets. */
                if(nSamples.Samples() >= 11)
                {
                    MAP_TIME_DATA[SERVER.IP] = nSamples.Majority();

                    SERVER.Close();
                    break;
                }
            }


            /* If there are no Successful Samples, Try another Connection. */
            if(nSamples.Samples() == 0)
            {
                printf("***** Core LLP: Failed To Get Time Samples.\n");
                //SEEDS.erase(SEEDS.begin() + nIterator);

                //SERVER.Close();

                continue;
            }

            /* Update Iterators and Flags. */
            if((MAP_TIME_DATA.size() > 0))
            {
                fTimeUnified = true;

                /* Majority Object to check for consensus on time samples. */
                CMajority<int> UNIFIED_MAJORITY;

                /* Info tracker to see the total samples. */
                std::map<int, unsigned int> TOTAL_SAMPLES;

                /* Iterate the Time Data map to find the majority time seed. */
                for(std::map<std::string, int>::iterator it=MAP_TIME_DATA.begin(); it != MAP_TIME_DATA.end(); ++it)
                {

                    /* Update the Unified Majority. */
                    UNIFIED_MAJORITY.Add(it->second);

                    /* Increase the count per samples (for debugging only). */
                    if(!TOTAL_SAMPLES.count(it->second))
                        TOTAL_SAMPLES[it->second] = 1;
                    else
                        TOTAL_SAMPLES[it->second] ++;
                }

                /* Set the Unified Average to the Majority Seed. */
                UNIFIED_AVERAGE_OFFSET = UNIFIED_MAJORITY.Majority();

                if(GetArg("-verbose", 0) >= 1)
                    printf("***** %i Total Samples | %i Offset (%u) | %i Majority (%u) | %" PRId64 "\n", MAP_TIME_DATA.size(), nSamples.Majority(), TOTAL_SAMPLES[nSamples.Majority()], UNIFIED_AVERAGE_OFFSET, TOTAL_SAMPLES[UNIFIED_AVERAGE_OFFSET], GetUnifiedTimestamp());
            }

            Sleep(20000);

            continue;

        }
        catch(std::exception& e){ printf("UTM ERROR: %s\n", e.what()); }
    }
}

/* DNS Query of Domain Names Associated with Seed Nodes */
std::vector<Net::CAddress> DNS_Lookup(std::vector<std::string>& DNS_Seed)
{
    std::vector<Net::CAddress> vNodes;
    int scount = 0;
    for (std::size_t seed = 0; seed < DNS_Seed.size(); seed++)
    {
        printf("%u Host: %s\n", seed, DNS_Seed[seed].c_str());
        scount++;
        std::vector<Net::CNetAddr> vaddr;
        if (Net::LookupHost(DNS_Seed[seed].c_str(), vaddr))
        {
            BOOST_FOREACH(Net::CNetAddr& ip, vaddr)
            {
                Net::CAddress addr = Net::CAddress(Net::CService(ip, Net::GetDefaultPort()));

                //Randomize the Seed Node Penalty time (3 - 7 days).
                addr.nTime = GetUnifiedTimestamp() - (3 * 86400) - GetRand(7 * 86400);

                vNodes.push_back(addr);

                printf("DNS Seed: %s\n", addr.ToStringIP().c_str());
            }
        }

        printf("DNS Seed Count: %d\n",scount);
    }

    return vNodes;
}
