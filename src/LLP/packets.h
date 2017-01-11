#ifndef NEXUS_LLP_NEXUS_PACKETS_H
#define NEXUS_LLP_NEXUS_PACKETS_H

#include "types.h"
	
namespace LLP
{
	
	/* Class to handle sending and receiving of More Complese Message LLP Packets. */
	class NexusPacket
	{
	public:
		NexusPacket() 
		{ 
			SetNull(); 
			
			Net::GetMesageStart(HEADER); 
		}
	
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
		bool Header(){ return IsNull() ? false : (LENGTH > 0 && CHECKSUM > 0 && COMMAND.length() > 0); }
		
		
		/* Sets the size of the packet from Byte Vector. */
		void SetLength(std::vector<unsigned char> BYTES) 
		{ 
			CDataStream ssLength(BYTES, SER_NETWORK, MIN_PROTO_VERSION),
			ssLength >> LENGTH;
		}
		
		
		/*Set the Packet Checksum Data. */
		void SetChecksum()
		{
			uint512 hash = SK512(DATA.begin(), DATA.end());
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
			
			/* Make sure Packet length is within bounds. */
			if (LENGTH > MAX_SIZE)
				return error("Nexus Packet (%s, %u bytes) : Message too Large\n", COMMAND.c_str(), LENGTH);

			/* Double check the Message Checksum. */
			uint512 hash = SK512(DATA.begin(), DATA.end());
			unsigned int nChecksum = 0;
			memcpy(&nChecksum, &hash, sizeof(nChecksum));
			if (nChecksum != CHECKSUM)
				return error("Nexus Packet (%s, %u bytes) : CHECKSUM MISMATCH nChecksum=%08x hdr.nChecksum=%08x\n",
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
	class NexusConnection : public Connection<NexusPacket>
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
		
		void PushPacket(std::string strMessage)
		{
			NexusPacket RESPONSE;
			RESPONSE.MESSAGE = strMessage;
			
			this->WritePacket(RESPONSE);
		}
		
		void PushPacket(std::string strMessage, CDataStream ssData)
		{
			NexusPacket RESPONSE;
			RESPONSE.MESSAGE = strMessage;

			RESPONSE.SetData(ssData);
			RESPONSE.SetChecksum();
			
			this->WritePacket(RESPONSE);
		}

		/** Non-Blocking Packet reader to build a packet from TCP Connection.
			This keeps thread from spending too much time for each Connection. **/
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
					std::vector<unsigned char> DATA( std::min(nAvailable, (unsigned int)(INCOMING.LENGTH - INCOMING.DATA.size())), 0);
					unsigned int nRead = Read(DATA, DATA.size());
					
					if(nRead == DATA.size())
					{
						INCOMING.DATA.insert(INCOMING.DATA.end(), DATA.begin(), DATA.end());
						Event(EVENT_PACKET, nRead);
					}
				}
			}
		}
	}
}

#endif
	
	
