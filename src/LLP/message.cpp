/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "message.h"

namespace LLP
{
	/* Message LLP Core Functions. 
	 * TODO: Integrate the Core LLP with this Message LLP by keeping Core Protocol alive for next mandatory update and removing on version 3.1.
	 * This will allow the time keeping and address seeding to be on one port and protocol.
	 * 
	 * Mining LLP should remain on its own dedicated protocol for efficiency.
	 * 
	 * Basic Protocol Functioning when Complete:
	 * 1. Get Time seeds from Core LLP (Work out how this will be transitioned into the next update.)
	 * 2. Only start once time is unified. This can be done by an initial time seed received from a version message.
	 * 3. Once Version is seen by new connected node, return an ACK to show that the connection is live.
	 * 4. Seed addresses when asked by other nodes.
	 * 5. Send Blocks when asked by other nodes.
	 * 6. Handle new Message "bestblock" to get the hash and height of greatest block.
	 * 7. Relay new inventory when recieved. This will let the network know of new inventory that hasn't been relayed. If it says "inventory already in vector" then don't relay that. Only relay new inventory one time.
	 * 8. Seed blocks and transactions independent of the main thread to accept them.
	 * 9. Add new "tx" message data into the Memory Pool. 
	 * 10. Run all new block data into ProcessBlock Function. Use a mutex to allow only one process block function to run at a time.
	 * 11. Handle Ping / Pong to know the node latency. Record this as a stat in this class declared in "message.h"
	 * 13. Check the new best block once per minute, if it doesn't have it ask for all blocks up to it.
	 * 14. If a getblocks requests more than 1000 blocks send the inventory, but only allow For a certain threshold of blocks to be sent per second.
	 * 15. Add "checkpoints" message to track the checkpoints other nodes have designated.
	 * 16. Add a "trustkey" command to let other nodes know what your trust key is. Require signature to verify ownership. This then allows nodes to see the network reputation of nodes claiming a key.
	 * 17. Keep eye out for another node using the same trust key. First come first serve on trust keys. If there are duplicate trust keys in two nodes drop both of the node's connections.
	 * 18. Add a "doubletrust" command to let the network know there is a node using a trust key on more than one computer. Ban those nodes from the network. 
	 * 19. Add a "txlock" command to agree on the protocol level what transactions are accepted into memory pool. If there is a lock conflict from two of the same inputs but different outputs remove both from mempool. 
	 * 
	 * 
	 * TODO: Remove all the net / netbase code that isn't needed anymore. Clean up the net folder and move rpcserver.h to RPC/server.h, so forth...
	 * TODO: Work on a HTTP Packet to be used in reading and writing the HTTP headers so that the JSON RPC server can use the LLP as well. This will inheret the LLP threading model and server.
	 */
	
	
	/* Check the Inventory to See if a given hash is found. 
		TODO: Rebuild this Inventory System. Much Better Ways to Do it. 
		*/

	CInv::CInv()
	{
		type = 0;
		hash = 0;
	}

	
	CInv::CInv(int typeIn, const uint1024& hashIn)
	{
		type = typeIn;
		hash = hashIn;
	}

	
	CInv::CInv(const std::string& strType, const uint1024& hashIn)
	{
		unsigned int i;
		for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
		{
			if (strType == ppszTypeName[i])
			{
				type = i;
				break;
			}
		}
		if (i == ARRAYLEN(ppszTypeName))
			throw std::out_of_range(strprintf("CInv::CInv(string, uint1024) : unknown type '%s'", strType.c_str()));
		hash = hashIn;
	}

	
	bool operator<(const CInv& a, const CInv& b)
	{
		return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
	}

	
	bool CInv::IsKnownType() const
	{
		return (type >= 1 && type < (int)ARRAYLEN(ppszTypeName));
	}

	
	const char* CInv::GetCommand() const
	{
		if (!IsKnownType())
			throw std::out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", type));
		return ppszTypeName[type];
	}

	
	std::string CInv::ToString() const
	{
		if(GetCommand() == "tx")
		{
			std::string invHash = hash.ToString();
			return strprintf("tx %s", invHash.substr(invHash.length() - 20, invHash.length()).c_str());
		}
			
		return strprintf("%s %s", GetCommand(), hash.ToString().substr(0,20).c_str());
	}

	
	void CInv::print() const
	{
		printf("CInv(%s)\n", ToString().c_str());
	}
	
	
}
