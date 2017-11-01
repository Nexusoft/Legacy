/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/


#include "../Core/include/manager.h"

#include "include/checkpoints.h"
#include "include/message.h"
#include "include/hosts.h"
#include "include/legacy.h"
#include "include/inv.h"

#include "../LLC/include/random.h"
#include "../LLD/include/index.h"

#include "../Util/include/args.h"
#include "../Util/include/hex.h"

#include "../main.h"


namespace LLP
{	
	
	/* Push a Message With Information about This Current Node. */
	void CLegacyNode::PushVersion()
	{
		/* Random Session ID */
		RAND_bytes((unsigned char*)&nSessionID, sizeof(nSessionID));
		
		/* Current Unified Timestamp. */
		int64 nTime = Core::UnifiedTimestamp();
		
		/* Dummy Variable NOTE: Remove in Tritium ++ */
		uint64 nLocalServices = 0;
		
		/* Relay Your Address. */
		CAddress addrMe  = CAddress(CService("0.0.0.0",0));
		CAddress addrYou = CAddress(CService("0.0.0.0",0));
		
		/* Push the Message to receiving node. */
		PushMessage("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe,
					nSessionID, FormatFullVersion(), Core::nBestHeight);
	}
	
	
	/** Handle Event Inheritance. **/
	void CLegacyNode::Event(unsigned char EVENT, unsigned int LENGTH)
	{
		/** Handle any DDOS Packet Filters. **/
		if(EVENT == EVENT_HEADER)
		{
			if(GetArg("-verbose", 0) >= 4)
				printf("***** Node recieved Message (%s, %u)\n", INCOMING.GetMessage().c_str(), INCOMING.LENGTH);
					
			if(fDDOS)
			{
				
				/* Give higher DDOS score if the Node happens to try to send multiple version messages. */
				if (INCOMING.GetMessage() == "version" && nCurrentVersion != 0)
					DDOS->rSCORE += 25;
				
				
				/* Check the Packet Sizes to Unified Time Commands. */
				if((INCOMING.GetMessage() == "getoffset" || INCOMING.GetMessage() == "offset") && INCOMING.LENGTH != 16)
					DDOS->Ban(strprintf("INVALID PACKET SIZE | OFFSET/GETOFFSET | LENGTH %u", INCOMING.LENGTH));
			}
			
			return;
		}
			
		/** Handle for a Packet Data Read. **/
		if(EVENT == EVENT_PACKET)
		{
			if(GetArg("-verbose", 0) >= 5)
				printf("***** Node Read Data for Message (%s, %u, %s)\n", INCOMING.GetMessage().c_str(), LENGTH, INCOMING.Complete() ? "TRUE" : "FALSE");
					
			/* Check a packet's validity once it is finished being read. */
			if(fDDOS) {

				/* Give higher score for Bad Packets. */
				if(INCOMING.Complete() && !INCOMING.IsValid()){
					
					if(GetArg("-verbose", 0) >= 3)
						printf("***** Dropped Packet (Complete: %s - Valid: %s)\n", INCOMING.Complete() ? "Y" : "N" , INCOMING.IsValid() ? "Y" : "N" );
					
					DDOS->rSCORE += 15;
				}

			}
				
			return;
		}
		
		
		/* Handle Node Pings on Generic Events */
		if(EVENT == EVENT_GENERIC)
		{
			
			if(nLastPing + 5 < Core::UnifiedTimestamp()) {
				RAND_bytes((unsigned char*)&nSessionID, sizeof(nSessionID));
				
				nLastPing = Core::UnifiedTimestamp();
				cLatencyTimer.Reset();
				
				PushMessage("ping", nSessionID);
			}
		}
			
			
		/** On Connect Event, Assign the Proper Handle. **/
		if(EVENT == EVENT_CONNECT)
		{
			addrThisNode = CAddress(CService(GetIPAddress(), GetDefaultPort()));
			nLastPing    = Core::UnifiedTimestamp();
			
			if(GetArg("-verbose", 0) >= 1)
				printf("***** %s Node %s Connected at Timestamp %" PRIu64 "\n", fOUTGOING ? "Ougoing" : "Incoming", addrThisNode.ToString().c_str(), Core::UnifiedTimestamp());
			
			if(fOUTGOING)
				PushVersion();
			
			return;
		}
		
		
		/* Handle the Socket Disconnect */
		if(EVENT == EVENT_DISCONNECT)
		{
			Core::pManager->vDropped.push_back(addrThisNode);
			
			if(GetArg("-verbose", 0) >= 1)
				printf("xxxxx %s Node %s Disconnected (%s) at Timestamp %" PRIu64 "\n", fOUTGOING ? "Ougoing" : "Incoming", addrThisNode.ToString().c_str(), ErrorMessage().c_str(), Core::UnifiedTimestamp());
			
			return;
		}
		
	}
		
		
	/** This function is necessary for a template LLP server. It handles your 
		custom messaging system, and how to interpret it from raw packets. **/
	bool CLegacyNode::ProcessPacket()
	{
		CDataStream ssMessage(INCOMING.DATA, SER_NETWORK, MIN_PROTO_VERSION);
		
		
		


		return true;
	}
	
}
