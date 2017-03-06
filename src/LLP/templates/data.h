/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_TEMPLATES_DATA_H
#define NEXUS_LLP_TEMPLATES_DATA_H

#include "types.h"

namespace LLP
{

	/** Base Template Thread Class for Server base. Used for Core LLP Packet Functionality. 
		Not to be inherited, only for use by the LLP Server Base Class. **/
	template <class ProtocolType> class DataThread
	{
		/* Data Thread. */
		Thread_t DATA_THREAD;
		
	public:
	
		/** Service that is used to handle Connections on this Thread. */
		Service_t IO_SERVICE;
		
		/* Variables to track Connection / Request Count. */
		bool fDDOS, fACTIVE, fMETER; unsigned int nConnections, ID, REQUESTS, TIMEOUT, DDOS_rSCORE, DDOS_cSCORE;
		
		/* Vector to store Connections. */
		std::vector< ProtocolType* > CONNECTIONS;
		
		DataThread<ProtocolType>(unsigned int id, bool isDDOS, unsigned int rScore, unsigned int cScore, unsigned int nTimeout, bool fMeter = false) : 
			ID(id), fDDOS(isDDOS), DDOS_rSCORE(rScore), DDOS_cSCORE(cScore), TIMEOUT(nTimeout), fACTIVE(true), fMETER(fMeter), REQUESTS(0), CONNECTIONS(0), nConnections(0), DATA_THREAD(boost::bind(&DataThread::Thread, this)){ }
			
		~DataThread<ProtocolType>()
		{
			fACTIVE = false;
			fMETER  = false;
			
		}
		
		/* Returns the index of a component of the CONNECTIONS vector that has been flagged Disconnected */
		int FindSlot()
		{
			int nSize = CONNECTIONS.size();
			for(int index = 0; index < nSize; index++)
				if(!CONNECTIONS[index])
					return index;
						
			return nSize;
		}

		/* Adds a new connection to current Data Thread */
		bool AddConnection(Socket_t SOCKET, DDOS_Filter* DDOS)
		{
			int nSlot = FindSlot();
			if(nSlot == CONNECTIONS.size())
				CONNECTIONS.push_back(NULL);
				
			if(fDDOS)
				DDOS -> cSCORE += 1;
			
			CONNECTIONS[nSlot] = new ProtocolType(SOCKET, DDOS, fDDOS);
			
			{ LOCK(CONNECTIONS[nSlot]->MUTEX);
				
				CONNECTIONS[nSlot]->Event(EVENT_CONNECT);
				CONNECTIONS[nSlot]->CONNECTED = true;
			}
			
			nConnections ++;
			
			return true;
		}
		
		/* Adds a new connection to current Data Thread */
		bool AddConnection(std::string strAddress, std::string strPort, DDOS_Filter* DDOS)
		{
			int nSlot = FindSlot();
			if(nSlot == CONNECTIONS.size())
				CONNECTIONS.push_back(NULL);
			
			Socket_t SOCKET;
			CONNECTIONS[nSlot] = new ProtocolType(SOCKET, DDOS, fDDOS);
			
			{ LOCK(CONNECTIONS[nSlot]->MUTEX);
				
				if(!CONNECTIONS[nSlot]->Connect(strAddress, strPort, IO_SERVICE))
				{
					delete CONNECTIONS[index];
					CONNECTIONS[index] = NULL;
					
					return false;
				}
				
				if(fDDOS)
					DDOS -> cSCORE += 1;
				
				CONNECTIONS[nSlot]->fOUTGOING = true;
				CONNECTIONS[nSlot]->Event(EVENT_CONNECT);
				CONNECTIONS[nSlot]->fCONNECTED = true;
			}
			
			nConnections ++;
			
			return true;
		}
		
		/* Removes given connection from current Data Thread. 
			Happens with a timeout / error, graceful close, or disconnect command. */
		void RemoveConnection(int index)
		{
			LOCK(CONNECTIONS[index]->MUTEX);
			{
				CONNECTIONS[index]->Event(EVENT_DISCONNECT);
				CONNECTIONS[index]->Disconnect();
			
				delete CONNECTIONS[index];
					
				CONNECTIONS[index] = NULL;
			}
			
			nConnections --;
		}
		
		/* Thread that handles all the Reading / Writing of Data from Sockets. 
			Creates a Packet QUEUE on this connection to be processed by an LLP Messaging Thread. */
		void Thread()
		{
			while(fACTIVE)
			{
				/* Keep data threads at 1000 FPS Maximum. */
				Sleep(1);
				
				/* Check all connections for data and packets. */
				int nSize = CONNECTIONS.size();
				for(int nIndex = 0; nIndex < nSize; nIndex++)
				{
					Sleep(1);
					
					try
					{
						
						/* Skip over Inactive Connections. */
						if(!CONNECTIONS[nIndex])
							continue;
		
						/* Skip over Connection if not Connected. */
						if(!CONNECTIONS[nIndex]->Connected())
							continue;
						
						/* Remove Connection if it has Timed out or had any Errors. */
						if(CONNECTIONS[nIndex]->Timeout(TIMEOUT) || CONNECTIONS[nIndex]->Errors())
						{
							RemoveConnection(nIndex);
							
							continue;
						}
						
						/* Lock the Main Processing of this Thread. */
						{ LOCK(CONNECTIONS[nIndex]->MUTEX);
						
							/* Handle any DDOS Filters. */
							boost::system::error_code ec;
							std::string ADDRESS = CONNECTIONS[nIndex]->GetIPAddress();
							
							if(fDDOS && ADDRESS != "127.0.0.1")
							{
								/* Ban a node if it has too many Requests per Second. **/
								if(CONNECTIONS[nIndex]->DDOS->rSCORE.Score() > DDOS_rSCORE || CONNECTIONS[nIndex]->DDOS->cSCORE.Score() > DDOS_cSCORE)
									CONNECTIONS[nIndex]->DDOS->Ban();
								
								/* Remove a connection if it was banned by DDOS Protection. */
								if(CONNECTIONS[nIndex]->DDOS->Banned())
								{
									RemoveConnection(nIndex);
									
									continue;
								}
							}
							
							
							/* Generic event for Connection. */
							CONNECTIONS[nIndex]->Event(EVENT_GENERIC);
							
							/* Work on Reading a Packet. **/
							CONNECTIONS[nIndex]->ReadPacket();
							
							/* If a Packet was received successfully, increment request count [and DDOS count if enabled]. */
							if(CONNECTIONS[nIndex]->PacketComplete())
							{
								
								/* Packet Process return value of False will flag Data Thread to Disconnect. */
								if(!CONNECTIONS[nIndex] -> ProcessPacket())
								{
									RemoveConnection(nIndex);
									
									continue;
								}
								
								CONNECTIONS[nIndex] -> ResetPacket();
								
								if(fMETER)
									REQUESTS++;
								
								if(fDDOS)
									CONNECTIONS[nIndex]->DDOS->rSCORE += 1;
								
							}
						
						}
					}
					catch(std::exception& e)
					{
						printf("error: %s\n", e.what());
					}
				}
			}
		}
	};
}

#endif
