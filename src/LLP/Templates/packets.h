#ifndef NEXUS_LLP_NEXUS_PACKETS_H
#define NEXUS_LLP_NEXUS_PACKETS_H

#include "types.h"
	
namespace LLP
{
	/* Class to handle sending and receiving of LLP Packets. */
	class Packet
	{
	public:
		Packet() { SetNull(); }
	
		/* Components of an LLP Packet.
			BYTE 0       : Header
			BYTE 1 - 5   : Length
			BYTE 6 - End : Data      */
		unsigned char    HEADER;
		unsigned int     LENGTH;
		std::vector<unsigned char> DATA;
		
		
		/* Set the Packet Null Flags. */
		inline void SetNull()
		{
			HEADER   = 255;
			LENGTH   = 0;
			
			DATA.clear();
		}
		
		
		/* Packet Null Flag. Header = 255. */
		bool IsNull() { return (HEADER == 255); }
		
		
		/* Determine if a packet is fully read. */
		bool Complete() { return (Header() && DATA.size() == LENGTH); }
		
		
		/* Determine if header is fully read */
		bool Header() { return IsNull() ? false : (HEADER < 128 && LENGTH > 0) || (HEADER >= 128 && HEADER < 255 && LENGTH == 0); }
		
		
		/* Sets the size of the packet from Byte Vector. */
		void SetLength(std::vector<unsigned char> BYTES) { LENGTH = (BYTES[0] << 24) + (BYTES[1] << 16) + (BYTES[2] << 8) + (BYTES[3] ); }
		
		
		/* Serializes class into a Byte Vector. Used to write Packet to Sockets. */
		std::vector<unsigned char> GetBytes()
		{
			std::vector<unsigned char> BYTES(1, HEADER);
			
			/* Handle for Data Packets. */
			if(HEADER < 128)
			{
				BYTES.push_back((LENGTH >> 24)); BYTES.push_back((LENGTH >> 16));
				BYTES.push_back((LENGTH >> 8));  BYTES.push_back(LENGTH);
				
				BYTES.insert(BYTES.end(),  DATA.begin(), DATA.end());
			}
			
			return BYTES;
		}
	};
	
	
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
			
			/* Make sure Packet length is within bounds. */
			if (LENGTH > MAX_SIZE)
				return error("Nexus Packet (%s, %u bytes) : Message too Large\n", COMMAND.c_str(), LENGTH);

			/* Double check the Message Checksum. */
			uint512 hash = LLH::SK512(DATA.begin(), DATA.end());
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
	

}

#endif
	
	
