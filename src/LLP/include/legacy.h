/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

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
    
    
	
	class CLegacyNode : public MessageConnection
	{
		CAddress addrThisNode;
		
	public:
		
		/* Constructors for Message LLP Class. */
		CLegacyNode() : MessageConnection(){ }
		CLegacyNode( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : MessageConnection( SOCKET_IN, DDOS_IN ) { }
		
		
		/** Randomly genearted session ID. **/
		uint64 nSessionID;
		
		
		/** String version of this Node's Version. **/
		std::string strNodeVersion;
		
		
		/** The current Protocol Version of this Node. **/
		int nCurrentVersion;
		
		
		/** The last checkpoint that was sent from this node. **/
		uint1024 hashLastCheckpoint;
		
		
		/** The best block that was recieved by this current node. **/
		uint1024 hashBestBlock;
		
		
		/** The last block that was recieved by this current node. **/
		uint1024 hashLastBlock;
		
		
		/** LEGACY: The height of this ndoe given at the version message. **/
		int nStartingHeight;
		
		
		/** Flag to determine if a connection is Inbound. **/
		bool fInbound;
		
		
		/** Flag to determine if a node is setup for push-syncing. **/
		bool fSync;
		
		
		/** Latency in Milliseconds to determine a node's reliability. **/
		unsigned int nNodeLatency; //milli-seconds
		
		
		/** Counter to keep track of the last time a ping was made. **/
		unsigned int nLastPing;
		
		
		/** Last time a block was requested from this specific node. **/
		unsigned int nLastBlockRequest;
		
		
		/** Timer object to keep track of ping latency. **/
		Timer cLatencyTimer;
		
		
		/** Time samples from this specific node. **/
		mruset<int> setTimeSamples;
		
		
		/** Last time a unified sample was asked for from this peer. **/
		unsigned int nLastUnifiedCheck;
		
		
		/** Mao to keep track of sent request ID's while witing for them to return. **/
		std::map<unsigned int, uint64> mapSentRequests;
		
		
		/** Log of this node's bad response messages. **/
		std::map<std::string, unsigned int> mapBadResponse;
		
		
		/** Virtual Functions to Determine Behavior of Message LLP.
		 * 
		 * @param[in] EVENT The byte header of the event type
		 * @param[in[ LENGTH The size of bytes read on packet read events
		 *
		 */
		 void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		
		
		/** Main message handler once a packet is recieved. **/
		bool ProcessPacket();
		
		
		/** Handle for version message **/
		void PushVersion();
		
		
		/** Send an Address to Node. 
		 * 
		 * @param[in] addr The address to send to nodes
		 * 
		 */
		void PushAddress(const CAddress& addr);
		
		
		/** Send the DoS Score to DDOS Filte
		 * 
		 * @param[in] nDoS The score to add for DoS banning
		 * @param[in] fReturn The value to return (False disconnects this node)
		 * 
		 */
		inline bool DoS(int nDoS, bool fReturn)
		{
			if(fDDOS)
				DDOS->rSCORE += nDoS;
			
			return fReturn;
		}
		
		
		/** Get the current IP address of this node. **/
		inline CAddress GetAddress() { return addrThisNode; }
		
	};
	
}

#endif
