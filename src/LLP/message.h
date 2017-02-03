#ifndef NEXUS_MESSAGE_SERVER_H
#define NEXUS_MESSAGE_SERVER_H

#include "server.h"
#include "types.h"

namespace LLP
{
	
	class MessageLLP : public NexusConnection
	{	
	public:
		Net::CAddress addr;
		std::string addrName;
		int nVersion;
		int nHeight;
		
		std::string strSubVer;
		bool fClient;
		bool fInbound;
		bool fNetworkNode;
		
		std::set<CAddress> setAddrKnown;
		mruset<CInv> setInventoryKnown;
		
		MessageLLP() : NexusConnection(){ }
		MessageLLP( Socket_t SOCKET_IN, DDOS_Filter* DDOS_IN, bool isDDOS = false ) : NexusConnection( SOCKET_IN, DDOS_IN ) 
		{ ADDRESS = parse_ip(SOCKET_IN->remote_endpoint().address().to_string()); }
		
		void Event(unsigned char EVENT, unsigned int LENGTH = 0);
		bool ProcessPacket();
}

#endif
