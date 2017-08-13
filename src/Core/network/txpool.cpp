/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "include/txpool.h"

#include "../types/include/transaction.h"

#include "../../LLD/include/index.h"
#include "../../LLP/include/node.h"


namespace Core
{
	
	void CTxPool::Confirm(CTransaction& tx)
	{
		
		/* Iterate the Inputs to remove from Input Locks. */
		for(auto input : tx.vin)
		{
			{ LOCK(MUTEX);
				if(mapInputLocks.count(input.prevout.hash))
					mapInputLocks.erase(input.prevout.hash);
			}
			
			SetState(tx.GetHash(), CHAIN);
		}
	}
	
	
	bool CTxPool::CheckInputs(CTransaction& tx)
	{
		/* Iterate the Inputs to check for duplicate spends. */
		for(auto input : tx.vin)
		{
			LOCK(MUTEX);
			
			if(mapInputLocks.count(input.prevout.hash))
				return false;
		}
		
		return true;
	}
	
	
	bool CTxPool::Accept(LLD::CIndexDB& indexdb, CTransaction &tx, bool fCheckInputs, bool* pfMissingInputs)
	{

		/* Standard Transaction Check. */
		if (!tx.CheckTransaction())
			return error("CTxPool::Accept() : CheckTransaction failed");
		
		
		/* Disallow Coinbase Transaction into TxPool */
		if (tx.IsCoinBase())
			return LLP::DoS(NULL, 100, error("CTxPool::Accept() : coinbase as individual tx"));
			
			
		/* Disallow Coinstake Transaction into TxPool. */
		if (tx.IsCoinStake())
			return LLP::DoS(NULL, 100, error("CTxPool::Accept() : coinstake as individual tx"));
		
		
		/* Disallow non-standard transactions except in TestNet (For Testing new Transaciton Types). */
		if (!fTestNet && !tx.IsStandard())
			return error("CTxPool::Accept() : nonstandard transaction type");

		
		/* Check for Duplicate Transactions in TxPool. */
		uint512 hash = tx.GetHash();
		if(Has(hash))
			return error("CTxPool::accept() : transaction already exists in pool");
		
		
		/* Furthur Input checking if Flagged. */
		if (fCheckInputs)
		{
			
			/* Check whether the Disk contains the Transaction. */
			if (indexdb.ContainsTx(hash))
				return error("CTxPool::accept() : transaction already exists on disk");
			
			
			/* Check for conflicting inputs in the pool. */
			if(!CheckInputs(tx))
				return error("CTxPool::Accept() : transaction input conflict");
			
			
			/* Grab the inputs of previous transactions for further checks. */
			MapPrevTx mapInputs;
			std::map<uint512, CTxIndex> mapUnused;
			bool fInvalid = false;
			if (!tx.FetchInputs(indexdb, mapUnused, false, false, mapInputs, fInvalid))
			{
			
				if (fInvalid)
					return error("CTxPool::Accept() : FetchInputs found invalid tx %s", hash.ToString().substr(0,10).c_str());
				
				
				if (pfMissingInputs)
					*pfMissingInputs = true;
				
				
				return error("CTxPool::Accept() : FetchInputs failed %s", hash.ToString().substr(0,10).c_str());
			}
			
			
			/* Check for Standard Inputs which are Pay-to-Public-Key-Hash. 
			 * NOTE: Make sure to do reasonable ECDSA verifications on non-standard transactions.
			 */
			if (!tx.AreInputsStandard(mapInputs) && !fTestNet)
				return error("CTxPool::Accept() : nonstandard transaction input");
			
			
			/* Check the Transaction Fees
			 * TODO: Remove Fees with new priority algorithm
			 */
			int64 nFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();
			
			
			/* Dismiss Transaction if fees are too low. */
			if (nFees < tx.GetMinFee(1000, false, GMF_RELAY))
				return error("CTxPool::Accept() : not enough fees");
			
			
			/* Check the previous transactions are not spent already.
			 * TODO: Handle better checking in the future.
			 */
			if (!tx.ConnectInputs(indexdb, mapInputs, mapUnused, CDiskTxPos(1,1,1), pindexBest, false, false))
				return error("CTxPool::Accept() : ConnectInputs failed %s", hash.ToString().substr(0,10).c_str());
			
		}
		
		
		/* Add the transaction to Pool. */
		Add(hash, tx);
		
		
		/* Set the Proper States. */
		SetState(hash, ACCEPTED);

		
		/* Verbose Debug Logging. */
		if(GetArg("-verbose", 0) >= 2)
			printf("CTxPool::Accept() : accepted %s\n", hash.ToString().substr(0,10).c_str());
		
		
		return true;
	}
	
}
