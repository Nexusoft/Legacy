#ifndef NEXUS_LLP_CLIENT_H
#define NEXUS_LLP_CLIENT_H

#include "types.h"

namespace LLP
{
	

		
	/* Base Class to create a Custom LLP Server. Protocol Type class must inherit Connection,
	 * and provide a ProcessPacket method. Optional Events by providing GenericEvent method. 
	 */
	template <class ProtocolType> class Client
	{
		Service_t IO_SERVICE;
		
	public:
		unsigned int MAX_THREADS;
		
		/** The data type to keep track of current running threads. **/
		std::vector< DataThread<ProtocolType>* > DATA_THREADS;
		
		
		Server<ProtocolType>(int nMaxThreads, bool isDDOS, int cScore, int rScore, int nTimeout, bool fListen = true) : 
			MAX_THREADS(nMaxThreads)
		{
			for(int index = 0; index < MAX_THREADS; index++)
				DATA_THREADS.push_back(new DataThread<ProtocolType>(index, fDDOS, rScore, cScore, nTimeout));
		}
		
		bool AddConnection(std::string strAdress, std::string strPort)
		{
			try
			{
				using boost::asio::ip::tcp;
				
				tcp::resolver 			  RESOLVER(IO_SERVICE);
				tcp::resolver::query      QUERY   (tcp::v4(), strAddress.c_str(), strPort.c_str());
				tcp::resolver::iterator   ADDRESS = RESOLVER.resolve(QUERY);
				
				Socket_t CONNECTION = Socket_t(new tcp::socket(IO_SERVICE));
				CONNECTION -> connect(*ADDRESS, ERROR_HANDLE);
				
				if(ERROR_HANDLE)
					return false;
				
				CONNECTED = true;
				TIMER.Start();
				
				unsigned int nIndex = FindThread();
				DATA_THREADS[nIndex]->AddConnection(CONNECTION);
				
				if(GetArgs("-verbose", 0) >= 3)
					printf("***** Connected to %s:%s::Assigned to thread %u\n", strAddress.c_str(), strPort.c_str(), nIndex);
				
				return true;
			}
			catch(...){ }
			
			return false;
		}
		
	private:
	
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
		
		/** Used for Meter. Adds up the total amount of requests from each Data Thread. **/
		int TotalRequests()
		{
			int nTotalRequests = 0;
			for(int nThread = 0; nThread < MAX_THREADS; nThread++)
				nTotalRequests += DATA_THREADS[nThread]->REQUESTS;
					
			return nTotalRequests;
		}
			
		/** Used for Meter. Resets the REQUESTS variable to 0 in each Data Thread. **/
		void ClearRequests()
		{
			for(int nThread = 0; nThread < MAX_THREADS; nThread++)
				DATA_THREADS[nThread]->REQUESTS = 0;
		}
	};
	
	
	/* Old Single Connection one per thread outbound LLP connection handler.
	 * TODO: Deprecate this specific Class once the new Core LLP is complete.
	 */
	class Outbound : public Connection<>
	{
		Service_t IO_SERVICE;
		
	public:
		std::string IP, PORT;
		
		/** Outgoing Client Connection Constructor **/
		Outbound(std::string ip, std::string port) : IP(ip), PORT(port), Connection() { }
		
		bool Connect()
		{
			try
			{
				using boost::asio::ip::tcp;
				
				tcp::resolver 			  RESOLVER(IO_SERVICE);
				tcp::resolver::query      QUERY   (tcp::v4(), IP.c_str(), PORT.c_str());
				tcp::resolver::iterator   ADDRESS = RESOLVER.resolve(QUERY);
				
				SOCKET = Socket_t(new tcp::socket(IO_SERVICE));
				SOCKET -> connect(*ADDRESS, ERROR_HANDLE);
				
				if(ERROR_HANDLE)
					return false;
				
				CONNECTED = true;
				TIMER.Start();
				
				printf("***** Connected to %s:%s...\n", IP.c_str(), PORT.c_str());
				
				return true;
			}
			catch(...){ Disconnect(); }
			
			return false;
		}
		
	};
	
	class CoreOutbound : public Outbound
	{
	public:
		CoreOutbound(std::string ip, std::string port) : Outbound(ip, port){}
		
		enum
		{
			/** DATA PACKETS **/
			TIME_DATA     = 0,
			ADDRESS_DATA  = 1,
			TIME_OFFSET   = 2,
			
			/** DATA REQUESTS **/
			GET_OFFSET    = 64,
			
			
			/** REQUEST PACKETS **/
			GET_TIME      = 129,
			GET_ADDRESS   = 130,
					

			/** GENERIC **/
			PING          = 253,
			CLOSE         = 254
		};
		
		inline Packet NewPacket() { return this->INCOMING; }
		
		inline Packet GetPacket(unsigned char HEADER)
		{
			Packet PACKET;
			PACKET.HEADER = HEADER;
			return PACKET;
		}
		
		inline void GetOffset(unsigned int nTimestamp)
		{
			Packet REQUEST = GetPacket(GET_OFFSET);
			REQUEST.LENGTH = 4;
			REQUEST.DATA   = uint2bytes(nTimestamp);
			
			this->WritePacket(REQUEST);
		}
		
		inline void GetTime()
		{
			Packet REQUEST = GetPacket(GET_TIME);
			this->WritePacket(REQUEST);
		}
		
		void Close()
		{
			Packet RESPONSE = GetPacket(CLOSE);
			this->WritePacket(RESPONSE);
			this->Disconnect();
		}
		
		inline void GetAddress()
		{
			Packet REQUEST = GetPacket(GET_ADDRESS);
			this->WritePacket(REQUEST);
		}
	};
	
}



#endif
