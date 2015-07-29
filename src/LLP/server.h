#ifndef Nexus_LLP_SERVER_H
#define Nexus_LLP_SERVER_H

#include "types.h"

namespace LLP
{

	/** Base Template Thread Class for Server base. Used for Core LLP Packet Functionality. 
		Not to be inherited, only for use by the LLP Server Base Class. **/
	template <class ProtocolType> class DataThread
	{
	public:
		/** Service that is used to handle Connections on this Thread. **/
		Service_t IO_SERVICE;
		
		/** Variables to track Connection / Request Count. **/
		bool fDDOS; unsigned int nConnections, DDOS_rSCORE, ID, TIMEOUT, REQUESTS;
		
		/** Vector to store Connections. **/
		std::vector< ProtocolType* > CONNECTIONS;
		
		/** Data Thread. **/
		Thread_t DATA_THREAD;
		
		/** Returns the index of a component of the CONNECTIONS vector that has been flagged Disconnected **/
		int FindSlot()
		{
			int nSize = CONNECTIONS.size();
			for(int index = 0; index < nSize; index++)
				if(!CONNECTIONS[index])
					return index;
						
			return nSize;
		}

		/** Adds a new connection to current Data Thread **/
		void AddConnection(Socket_t SOCKET, DDOS_Filter* DDOS)
		{
			int nSlot = FindSlot();
			if(nSlot == CONNECTIONS.size())
				CONNECTIONS.push_back(NULL);
					
			CONNECTIONS[nSlot] = new ProtocolType(SOCKET, DDOS);
			++nConnections;
			
			//printf("New LLP Connection Added to Slot %i on Thread %i...\n", nSlot, ID);
		}
		
		/** Removes given connection from current Data Thread. 
			Happens with a timeout / error, graceful close, or disconnect command. **/
		void RemoveConnection(int index)
		{
			CONNECTIONS[index]->Disconnect();
			delete CONNECTIONS[index];
					
			CONNECTIONS[index] = NULL;
			-- nConnections;
			
			//printf("LLP Connection Removed from Slot %i on Thread %i...\n", index, ID);
		}
		
		/** Thread that handles all the Reading / Writing of Data from Sockets. 
			Creates a Packet QUEUE on this connection to be processed by an LLP Messaging Thread. **/
		void Thread()
		{
			loop
			{
				/** Keep data threads at 1000 FPS Maximum. **/
				Sleep(1);
				
				/** Check all connections for data and packets. **/
				int nSize = CONNECTIONS.size();
				for(int nIndex = 0; nIndex < nSize; nIndex++)
				{
					try
					{
						/** Skip over Inactive Connections. **/
						if(!CONNECTIONS[nIndex])
							continue;
							
							
						/** Remove Connection if it has Timed out or had any Errors. **/
						if(CONNECTIONS[nIndex]->Timeout(TIMEOUT) || CONNECTIONS[nIndex]->Errors())
						{
							RemoveConnection(nIndex);
							
							continue;
						}
						
						
						/** Handle any DDOS Filters. **/
						if(fDDOS)
						{
							
							/** Ban a node if it fails DDOS Packet Check or has too many Requests per Second. **/
							if(CONNECTIONS[nIndex]->DDOS->rSCORE.Score() > DDOS_rSCORE)
							   CONNECTIONS[nIndex]->DDOS->Ban();
							
							/** Remove a connection if it was banned by DDOS Protection. **/
							if(CONNECTIONS[nIndex]->DDOS->Banned())
							{
								RemoveConnection(nIndex);
								
								continue;
							}
						}
						
						/** Work on Reading a Packet. **/
						CONNECTIONS[nIndex]->ReadPacket();
						
						/** If a Packet was received successfully, increment request count [and DDOS count if enabled]. **/
						if(CONNECTIONS[nIndex]->PacketComplete())
						{
							
							/** Packet Process return value of False will flag Data Thread to Disconnect. **/
							if(!CONNECTIONS[nIndex] -> ProcessPacket())
							{
								RemoveConnection(nIndex);
								
								continue;
							}
							
							CONNECTIONS[nIndex] -> ResetPacket();
							REQUESTS++;
							
							if(fDDOS)
								CONNECTIONS[nIndex]->DDOS->rSCORE++;
						}
					}
					catch(std::exception& e)
					{
						printf("error: %s\n", e.what());
					}
				}
			}
		}
		
		DataThread<ProtocolType>(unsigned int id, bool isDDOS, unsigned int rScore, unsigned int nTimeout) : 
			ID(id), fDDOS(isDDOS), DDOS_rSCORE(rScore), TIMEOUT(nTimeout), REQUESTS(0), CONNECTIONS(0), nConnections(0), DATA_THREAD(boost::bind(&DataThread::Thread, this)) { }
	};

	
	
	/** Base Class to create a Custom LLP Server. Protocol Type class must inherit Connection,
		and provide a Process method. Optional Broadcasting System added by creating a thread in
		Inherited Server Class, to Broadcast Packet(s) of your choice. **/
	template <class ProtocolType> class Server
	{
	protected:
		int PORT, MAX_THREADS;
		
		/** The data type to keep track of current running threads. **/
		std::vector< DataThread<ProtocolType>* > DATA_THREADS;
		
		/** The DDOS variables. Tracks the Requests and Connections per Second
			from each connected address. **/
		std::map<unsigned int,   DDOS_Filter*> DDOS_MAP;
		bool fDDOS; unsigned int DDOS_cSCORE;
		
	public:
		Server<ProtocolType>(int nPort, int nMaxThreads, bool isDDOS, int cScore, int rScore, int nTimeout) : 
			fDDOS(isDDOS), DDOS_cSCORE(cScore), LISTENER(SERVICE), PORT(nPort), MAX_THREADS(nMaxThreads), LISTEN_THREAD(boost::bind(&Server::ListeningThread, this))
		{
			for(int index = 0; index < MAX_THREADS; index++)
				DATA_THREADS.push_back(new DataThread<ProtocolType>(index, fDDOS, rScore, nTimeout));
		}
		
	private:
	
		/** Basic Socket Handle Variables. **/
		Service_t   SERVICE;
		Listener_t  LISTENER;
		Error_t     ERROR_HANDLE;
		Thread_t    LISTEN_THREAD;
	
		/** Determine the thread with the least amount of active connections. 
			This keeps the load balanced across all server threads. **/
		int FindThread()
		{
			int nIndex = 0, nConnections = DATA_THREADS[0]->nConnections;
			for(int index = 1; index < MAX_THREADS; index++)
			{
				if(DATA_THREADS[index]->nConnections < nConnections)
				{
					nIndex = index;
					nConnections = DATA_THREADS[index]->nConnections;
				}
			}
			
			return nIndex;
		}
		
		/** Main Listening Thread of LLP Server. Handles new Connections and DDOS associated with Connection if enabled. **/
		void ListeningThread()
		{
			/** Don't listen until all data threads are created. **/
			while(DATA_THREADS.size() < MAX_THREADS)
				Sleep(1000);
				
			/** Basic Socket Options for Boost ASIO. Allow aborted connections, don't allow lingering. **/
			boost::asio::socket_base::enable_connection_aborted    CONNECTION_ABORT(true);
			boost::asio::socket_base::linger                       CONNECTION_LINGER(false, 0);
			boost::asio::ip::tcp::acceptor::reuse_address          CONNECTION_REUSE(true);
			boost::asio::ip::tcp::endpoint 						   ENDPOINT(boost::asio::ip::tcp::v4(), PORT);
			
			/** Open the listener with maximum of 1000 queued Connections. **/
			LISTENER.open(ENDPOINT.protocol());
			LISTENER.set_option(CONNECTION_ABORT);
			LISTENER.set_option(CONNECTION_REUSE);
			LISTENER.set_option(CONNECTION_LINGER);
			LISTENER.bind(ENDPOINT);
			LISTENER.listen(1000, ERROR_HANDLE);
			
			printf("LLP Server Listening on Port %u\n", PORT);
			loop
			{
				/** Limit listener to allow maximum of 200 new connections per second. **/
				Sleep(5);
				
				try
				{
					/** Accept a new connection, then process DDOS. **/
					int nThread = FindThread();
					Socket_t SOCKET(new boost::asio::ip::tcp::socket(DATA_THREADS[nThread]->IO_SERVICE));
					LISTENER.accept(*SOCKET);
					
					
					/** Initialize DDOS Protection for Incoming IP Address. **/
					std::vector<unsigned char> BYTES = parse_ip(SOCKET->remote_endpoint().address().to_string());
					unsigned int ADDRESS = bytes2uint(BYTES);
					
					
					if(!DDOS_MAP.count(ADDRESS))
						DDOS_MAP[ADDRESS] = new DDOS_Filter(60);
						
					/** DDOS Operations: Only executed when DDOS is enabled. **/
					if(fDDOS)
					{
						/** Disallow new connections if they are currently banned **/
						if(DDOS_MAP[ADDRESS]->Banned())
						{
							SOCKET -> shutdown(boost::asio::ip::tcp::socket::shutdown_both, ERROR_HANDLE);
							SOCKET -> close();
							
							continue;
						}
							
						/** Ban a node with more than 2 connection attempts per second **/
						if(DDOS_MAP[ADDRESS] -> cSCORE.Score() > DDOS_cSCORE)
						{
							/** DDOS ban time gets larger the more you attempt to DDOS. **/
							DDOS_MAP[ADDRESS]->Ban();
							
							SOCKET -> shutdown(boost::asio::ip::tcp::socket::shutdown_both, ERROR_HANDLE);
							SOCKET -> close();
							
							continue;
						}
						
						/** Continue to increase DDOS score. **/
						DDOS_MAP[ADDRESS] -> cSCORE++;
					
					}
					
					/** Add new connection if passed all DDOS checks. **/
					DATA_THREADS[nThread]->AddConnection(SOCKET, DDOS_MAP[ADDRESS]);
				}
				catch(std::exception& e)
				{
					printf("error: %s\n", e.what());
				}
			}
		}
	};

}

#endif