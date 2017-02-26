/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_TRANSACTION_H
#define NEXUS_CORE_TRANSACTION_H

namespace Core
{
	
	/* The global Memory Pool to Hold New Transactions. */
	extern CTxMemPool mempool;
	
	
	
	/* __________________________________________________ (Transaction Methods) __________________________________________________  */
	
	
	
	
	/* Add an oprhan transaction to the Orphans Memory Map. */
	bool AddOrphanTx(const CDataStream& vMsg);
	
	
	/* Remove an Orphaned Transaction from the Memory Mao. */
	void EraseOrphanTx(uint512 hash);
	
	
	/* Set the Limit of the Orphan Transaction Sizes Dynamically. */
	unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans);
	
	
	/* Get a specific transaction from the Database or from a block's binary position. */
	bool GetTransaction(const uint512 &hash, CTransaction &tx, uint1024 &hashBlock);
	
}


#endif
