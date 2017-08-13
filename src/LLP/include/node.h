/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_INCLUDE_NODE_H
#define NEXUS_LLP_INCLUDE_NODE_H

#include <queue>

#include "network.h"
#include "message.h"
#include "inv.h"

#include "../../Core/types/include/transaction.h"
#include "../../Core/types/include/block.h"

#include "../../Util/templates/mruset.h"
#include "../../Util/templates/containers.h"


namespace LLP
{
	static CAddress addrMyNode;
    
    
	
	class CNode : public MessageConnection
	{
		CAddress addrThisNode;
		
	public:
		
		/* Mutex for handling node inventory messages. */
		Mutex_t NODE_MUTEX;
		
		
		/* Node Identifier */
		uint64 nSessionID;
		
		
		/* Basic Stats. */
		std::string strNodeVersion;
		int nCurrentVersion;
		int nStartingHeight;
		
		
		/* Positive: Node Stats. */
		unsigned int nLastMessageTime;
		unsigned int nNodeGenesisTime;
		unsigned int nTotalObjectAcks;
		
		
		/* Negative: Node Stats. */
		unsigned int nTotalBans;
		unsigned int nTotalRejects;
		
		
		/* Consensus Stats. */
		uint1024 hashLastCheckpoint;
		unsigned int nUnifiedOffset;
		
		
		/* Behavior Flags*/
		bool fClient;
		bool fInbound;
		bool fNetworkNode;
        bool fTritium;
		
		
		/* Network Statistics. */
		unsigned int nNodeLatency; //milli-seconds
		unsigned int nLastPing;
		
		
		/* Node Timers (Internal Statistics) */
		Timer cLatencyTimer;
		
		
		/* Unified Time Specific Variablesy. */
		mruset<int> setTimeSamples;
		unsigned int nLastUnifiedCheck;
		
		
		/* Node Process Queue. TODO: Make Process Queue Generate std::pair<key_data, data> */
		std::queue< std::vector<unsigned char> > queueProcessing; //Need this to be able to accept node data pushes into the server (manager) class logic.
		
		
		/* Message ID's of Requests Sent. */
		std::map<unsigned int, uint64> mapSentRequests;
		
		
		/* Keep track of this nodes bad responses. */
		std::map<std::string, unsigned int> mapBadResponse;

		
		
		/* Constructors for Message LLP Class. */
		CNode() : MessageConnection(){ }
		CNode( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : MessageConnection( SOCKET_IN, DDOS_IN ) { }
		
		
		/* Virtual Functions to Determine Behavior of Message LLP. */
		void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		bool ProcessPacket();
		void PushVersion();
		
		
		/* Send an Address to Node. */
		void PushAddress(const CAddress& addr);

		
		/* Keep Track of the Inventory we Already have. */
		void AddInventoryKnown(const CInv& inv);

		
		/* Send Inventory We have. */
		void PushInventory(const CInv& inv);
		
		
		/* Send the DoS Score to DDOS Filter. */
		inline bool DoS(int nDoS, bool fReturn)
		{
			if(fDDOS)
				DDOS->rSCORE += nDoS;
			
			return fReturn;
		}
		
		/* Return the IP address of this node. */
		inline CAddress GetAddress() { return addrThisNode; }
	};
	
	
	/* DoS Wrapper for Block Level Functions. */
	inline bool DoS(CNode* pfrom, int nDoS, bool fReturn)
	{
		if(pfrom)
			pfrom->DDOS->rSCORE += nDoS;
			
		return fReturn;
	}
}

#endif
