/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_LLP_INCLUDE_MESSAGE_H
#define NEXUS_LLP_INCLUDE_MESSAGE_H

#include"../../Util/templates/serialize.h"

#include "../templates/types.h"
#include "network.h"

namespace LLP
{

	/* Class to handle sending and receiving of More Complese Message LLP Packets. */
	class TritiumPacket
	{
	public:
	
		/* Components of a Tritium LLP Packet.
		 * 
		 * BYTE 00 - 01  : Message
		 * BYTE 02 - 05  : Length
		 * BYTE 06 - 09  : Checksum
		 * BYTE 10 - XX  : Data
		 * 
		 * NOTE: Total messages available is 65,536 messages with enumeration
		 * 
		 */
		unsigned short	MESSAGE;
		unsigned int	LENGTH;
		unsigned int	CHECKSUM;
		
		std::vector<unsigned char> DATA;
		
		TritiumPacket()
		{
			SetNull();
			SetHeader();
		}
		
		TritiumPacket(unsigned short MESSAGE_IN)
		{
			SetNull();
			SetMessage(MESSAGE_IN);
		}
		
		IMPLEMENT_SERIALIZE
		(
			READWRITE(MESSAGE);
			READWRITE(LENGTH);
			READWRITE(CHECKSUM);
		)
		
		
		/* Set the Packet Null Flags. */
		void SetNull()
		{
			MESSAGE   = 0;
			LENGTH    = 0;
			CHECKSUM  = 0;
			
			DATA.clear();
		}
		
		
		/* Packet Null Flag. Length and Checksum both 0. */
		bool IsNull() { return MESSAGE == 0 && LENGTH == 0 && CHECKSUM == 0 }
		
		
		/* Determine if a packet is fully read. */
		bool Complete() { return (Header() && DATA.size() == LENGTH); }
		
		
		/* Determine if header is fully read */
		bool Header()   { return IsNull() ? false : (CHECKSUM > 0 && LENGTH > 0 && MESSAGE > 0); }
		
		
		/* Set the message in the packet header. */
		void SetMessage(unsigned short MESSAGE_IN) { MESSAGE = MESSAGE_IN; }
		
		
		/* Set the Packet Checksum Data. */
		void SetChecksum() { CHECKSUM = LLC::HASH::SK32(DATA.begin(), DATA.end()); }
		
		
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
			
			/* Make sure Packet length is within bounds. (Max 512 MB Packet Size) */
			if (LENGTH > (1024 * 1024 * 512))
				return error("Message Packet (%s, %u bytes) : Message too Large", MESSAGE, LENGTH);

			/* Double check the Message Checksum. */
			unsigned int nChecksum = LLC::HASH::SK32(DATA.begin(), DATA.end());
			if (nChecksum != CHECKSUM)
				return error("Message Packet (%u bytes) : CHECKSUM MISMATCH nChecksum=%u hdr.nChecksum=%u",
				   LENGTH, nChecksum, CHECKSUM);
				
			return true;
		}
		
		
		/* Serializes class into a Byte Vector. Used to write Packet to Sockets. */
		std::vector<unsigned char> GetBytes()
		{
			CDataStream ssHeader(SER_NETWORK, MIN_PROTO_VERSION);
			ssHeader << *this;
			
			std::vector<unsigned char> BYTES(ssHeader.begin(), ssHeader.end());
			BYTES.insert(BYTES.end(), DATA.begin(), DATA.end());
			return BYTES;
		}
	};
	
	
	/* Base Template class to handle outgoing / incoming LLP data for both Client and Server. */
	class TritiumConnection : public BaseConnection<TritiumPacket>
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
		virtual bool ProcessPacket(){ return false; }
	public:
		
		/* Constructors for Message LLP Class. */
		TritiumConnection() : BaseConnection<TritiumPacket>(){ }
		TritiumConnection( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : BaseConnection<TritiumPacket>( SOCKET_IN, DDOS_IN, isDDOS ) { }

		
		/* Non-Blocking Packet reader to build a packet from TCP Connection.
		 * This keeps thread from spending too much time for each Connection. 
		 */
		virtual void ReadPacket()
		{
			if(!INCOMING.Complete())
			{
				/** Handle Reading Packet Length Header. **/
				if(SOCKET->available() >= 10 && INCOMING.IsNull())
				{
					std::vector<unsigned char> BYTES(10, 0);
					if(Read(BYTES, 10) == 10)
					{
						CDataStream ssHeader(BYTES, SER_NETWORK, MIN_PROTO_VERSION);
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
		
		
		/** Construct a new Tritium Packet.
		 * 
		 * @param[in] nCommand The command to push in packet header
		 * @param[in] ssData The serialized data to be pushed in packet.
		 * 
		 * @return The new packet responded with.
		 */
		TritiumPacket NewMessage(const unsigned short nCommand, CDataStream ssData)
		{
			TritiumPacket RESPONSE(chCommand);
			RESPONSE.SetData(ssData);
			
			return RESPONSE;
		}
		
		
		void PushMessage(const unsigned short nCommand)
		{
			try
			{
				TritiumPacket RESPONSE(nCommand);
				RESPONSE.SetChecksum();
			
				this->WritePacket(RESPONSE);
			}
			catch(...)
			{
				throw;
			}
		}
		
		template<typename T1>
		void PushMessage(const unsigned short nCommand, const T1& t1)
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
				ssData << t1 << t2 << t3 << t4 << t5 << t6 << t7;
				
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
				ssData << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
				
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
				ssData << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
				
				this->WritePacket(NewMessage(chMessage, ssData));
			}
			catch(...)
			{
				throw;
			}
		}
		
	};
	
	
	class CTritiumNode : public TritiumConnection
	{
		CAddress addrThisNode;
		
	public:
		
		/* Constructors for Message LLP Class. */
		CLegacyNode() : MessageConnection(){ }
		CLegacyNode( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : MessageConnection( SOCKET_IN, DDOS_IN ) { }
		
		
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
		
		
		
		/** Get the current IP address of this node. **/
		inline CAddress GetAddress() { return addrThisNode; }
	};
	
	
	/* DoS Wrapper for Block Level Functions. */
	inline bool DoS(CLegacyNode* pfrom, int nDoS, bool fReturn)
	{
		if(pfrom)
			pfrom->DDOS->rSCORE += nDoS;
			
		return fReturn;
	}
}

#endif
