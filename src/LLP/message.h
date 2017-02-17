/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2013])) == Videlicet[2014] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLP_MESSAGE_SERVER_H
#define NEXUS_LLP_MESSAGE_SERVER_H

#include "Templates/base.h"
#include "Include/protocol.h"

namespace LLP
{
	
	/* Message Packet Leading Bytes. */
	static unsigned char MESSAGE_START_TESTNET[4] = { 0xe9, 0x59, 0x0d, 0x05 };
	static unsigned char MESSAGE_START_MAINNET[4] = { 0x05, 0x0d, 0x59, 0xe9 };
	
	
	enum
	{
		MSG_TX = 1,
		MSG_BLOCK,
	};
	
	
		/** inv message data */
	class CInv
	{
		public:
			CInv();
			CInv(int typeIn, const uint1024& hashIn);
			CInv(const std::string& strType, const uint1024& hashIn);

			IMPLEMENT_SERIALIZE
			(
				READWRITE(type);
				READWRITE(hash);
			)

			friend bool operator<(const CInv& a, const CInv& b);

			bool IsKnownType() const;
			const char* GetCommand() const;
			std::string ToString() const;
			void print() const;

		// TODO: make private (improves encapsulation)
		public:
			int type;
			uint1024 hash;
	};
	
	
	/* Class to handle sending and receiving of More Complese Message LLP Packets. */
	class MessagePacket
	{
	public:
	
		/* 
		 * Components of an Nexus LLP Packet.
		 * BYTE 0 - 4    : Start
		 * BYTE 5 - 17   : Command
		 * BYTE 18 - 22  : Size      
		 * BYTE 23 - 26  : Checksum
		 * BYTE 26 - X   : Data
		 * 
		 */
		unsigned char HEADER[4];
		char 			  COMMAND[12];
		unsigned int  LENGTH;
		unsigned int  CHECKSUM;
		
		std::vector<unsigned char> DATA;
		
		MessagePacket()
		{ 
			SetNull();
			
			memcpy(HEADER, (fTestNet ? MESSAGE_START_TESTNET : MESSAGE_START_MAINNET), sizeof(HEADER));
		}
		
		IMPLEMENT_SERIALIZE
		(
			READWRITE(FLATDATA(HEADER));
			READWRITE(FLATDATA(COMMAND));
			READWRITE(LENGTH);
			READWRITE(CHECKSUM);
		)
		
		/* Set the Packet Null Flags. */
		inline void SetNull()
		{
			LENGTH    = 0;
			CHECKSUM  = 0;
			COMMAND   = "";
			
			DATA.clear();
		}
		
		
		/* Packet Null Flag. Length and Checksum both 0. */
		bool IsNull() { return (LENGTH == 0 && CHECKSUM == 0 && COMMAND == "" && DATA.empty()); }
		
		
		/* Determine if a packet is fully read. */
		bool Complete() { return (Header() && DATA.size() == LENGTH); }
		
		
		/* Determine if header is fully read */
		bool Header()   { return IsNull() ? false : (LENGTH > 0 && CHECKSUM > 0 && COMMAND.length() > 0); }
		
		
		/* Sets the size of the packet from Byte Vector. */
		void SetLength(std::vector<unsigned char> BYTES) 
		{ 
			CDataStream ssLength(BYTES, SER_NETWORK, MIN_PROTO_VERSION),
			ssLength >> LENGTH;
		}
		
		
		/*Set the Packet Checksum Data. */
		void SetChecksum()
		{
			uint512 hash = LLH::SK512(DATA.begin(), DATA.end());
			memcpy(&CHECKSUM, &hash, sizeof(CHECKSUM));
		}
		
		/* Set the Packet Data. */
		void SetData(CDataStream ssData)
		{
			std::vector<unsigned char> vData(ssData.begin(), ssData.end());
			
			LENGTH = vData.size();
			DATA   = vData;
			
			SetChecksum();
		}
		
		
		/* Check the Validity of the Packet. */
		bool IsValid()
		{
			/* Check that the packet isn't NULL. */
			if(IsNull())
				return false;
			
			/* Check the Header Bytes. */
			if(memcmp(HEADER, (fTestNet ? MESSAGE_START_TESTNET : MESSAGE_START_TESTNET), sizeof(HEADER)) != 0)
				return error("Message Packet (Invalid Packet Header");
			
			/* Make sure Packet length is within bounds. */
			if (LENGTH > MAX_SIZE)
				return error("Message Packet (%s, %u bytes) : Message too Large", COMMAND.c_str(), LENGTH);

			/* Double check the Message Checksum. */
			uint512 hash = LLH::SK512(DATA.begin(), DATA.end());
			unsigned int nChecksum = 0;
			memcpy(&nChecksum, &hash, sizeof(nChecksum));
			if (nChecksum != CHECKSUM)
				return error("Nexus Packet (%s, %u bytes) : CHECKSUM MISMATCH nChecksum=%08x hdr.nChecksum=%08x",
				   strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
				
			return true;
		}
		
		
		/** Serializes class into a Byte Vector. Used to write Packet to Sockets. **/
		std::vector<unsigned char> GetBytes()
		{
			CDataStream ssHeader(SER_NETWORK, MIN_PROTO_VERSION),
			ssHeader << *this;
			
			std::vector<unsigned char> BYTES(ssHeader.begin(), ssHeader.end());
			BYTES.insert(BYTES.end(), DATA.begin(), DATA.end());
			return BYTES;
		}
	};
	
	
	/* Base Template class to handle outgoing / incoming LLP data for both Client and Server. */
	class MessageConnection : public Connection<MessagePacket>
	{
	protected:
		/* 
			Virtual Event Function to be Overridden allowing Custom Read Events. 
			Each event fired on Header Complete, and each time data is read to fill packet.
			Useful to check Header length to maximum size of packet type for DDOS protection, 
			sending a keep-alive ping while downloading large files, etc.
			
			LENGTH == 0: General Events
			LENGTH  > 0 && PACKET: Read nSize Bytes into Data Packet
		*/
		virtual void Event(unsigned char EVENT, unsigned int LENGTH = 0){ }
		
		/* Virtual Process Function. To be overridden with your own custom packet processing. */
		virtual bool ProcessPacket(){ }
	public:

		/* Non-Blocking Packet reader to build a packet from TCP Connection.
		 * This keeps thread from spending too much time for each Connection. 
		 */
		void ReadPacket()
		{
			if(!INCOMING.Complete())
			{
				/** Handle Reading Packet Length Header. **/
				if(SOCKET->available() >= 24 && INCOMING.IsNull())
				{
					std::vector<unsigned char> BYTES(24, 0);
					if(Read(BYTES, 24) == 24)
					{
						CDataStream ssHeader(BYTES, SER_NETWORK, MIN_PROTO_VERSION),
						ssHeader >> INCOMING;
						
						Event(EVENT_HEADER);
					}
				}
					
				/** Handle Reading Packet Data. **/
				unsigned int nAvailable = SOCKET->available();
				if(nAvailable > 0 && !INCOMING.IsNull() && INCOMING.DATA.size() < INCOMING.LENGTH)
				{
					std::vector<unsigned char> DATA( std::min(std::min(nAvailable, 512u), (unsigned int)(INCOMING.LENGTH - INCOMING.DATA.size())), 0);
					unsigned int nRead = Read(DATA, DATA.size());
					
					if(nRead == DATA.size())
					{
						INCOMING.DATA.insert(INCOMING.DATA.end(), DATA.begin(), DATA.end());
						Event(EVENT_PACKET, nRead);
					}
				}
			}
		}
		
		MessagePacket NewMessage(const char* chMessage, CDataStream ssData)
		{
			NexusPacket RESPONSE;
			RESPONSE.MESSAGE = chMessage;
			
			RESPONSE.SetData(ssData);
			RESPONSE.SetChecksum();
		}
		
		void PushMessage(const char* chMessage)
		{
			try
			{
				NexusPacket RESPONSE;
				RESPONSE.MESSAGE = chMessage;
			
				this->WritePacket(RESPONSE);
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1>
		void PushMessage(const char* chMessage, const T1& t1)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}

		template<typename T1, typename T2, typename T3, typename T4>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3, typename T4, typename T5>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4 << t5;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4 << t5 << t6;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4 << t5 << t6 <<t7;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4 << t5 << t6 <<t7 << t8;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
		void PushMessage(const char* chMessage, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
		{
			try
			{
				CDataStream ssData(SER_NETWORK, MIN_PROTO_VERSION);
				ssData << t1 << t2 << t3 << t4 << t5 << t6 <<t7 << t8 << t9;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
	};
	
	
	class MessageLLP : public MessageConnection
	{	
	public:
		
		/* Address of the current Message LLP Connection. */
		CAddress addr;
		
		
		/* Node Information about this Message LLP Connection. */
		std::string addrName;
		std::string strSubVer;
		
		
		/* Basic node Stats for this Current Message LLP Connection. */
		int nVersion;
		int nHeight;
		int nDuraction;
		int nTimestamp;
		
		
		/* Basic flags to set the behavior of the Message LLP Connection. */
		bool fClient;
		bool fInbound;
		bool fNetworkNode;
		
		
		/* Mutex for Inventory Operations. */
		Mutex_t INVENTORY_MUTEX;
		
		
		/* Known Inventory to make sure duplicate requests are not called out. */
		mruset<CInv> setInventoryKnown;
		
		
		/* Constructors for Message LLP Class. */
		MessageLLP() : MessageConnection(){ }
		MessageLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : NexusConnection( SOCKET_IN, DDOS_IN ) 
		{ ADDRESS = parse_ip(SOCKET_IN->remote_endpoint().address().to_string()); }
		
		
		/* Virtual Functions to Determine Behavior of Message LLP. */
		void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		bool ProcessPacket();
		
		
		/* Send an Address to Node. */
		void PushAddress(const CAddress& addr)
		{
			if (addr.IsValid() && !setAddrKnown.count(addr))
				this->PushMessage("addr", addr);
		}

		
		/* Keep Track of the Inventory we Already have. */
		void AddInventoryKnown(const CInv& inv)
		{
			LOCK_GUARD(INVENTORY_MUTEX);
			
			setInventoryKnown.insert(inv);
		}

		
		/* Send Inventory We have. */
		void PushInventory(const CInv& inv)
		{
			if (!setInventoryKnown.count(inv))
				this->PushMessage("inv", inv);
		}
	};
}

#endif
