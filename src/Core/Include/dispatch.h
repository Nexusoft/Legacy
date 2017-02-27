/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_DISPATCH_H
#define NEXUS_CORE_DISPATCH_H

namespace Core
{
	
	/* Register a new wallet class to the dispatch registry (Can handle multiple wallets). */
	void RegisterWallet(Wallet::CWallet* pwalletIn);
	
	
	/* Remove a given wallet class from the dispatch registry. */
	void UnregisterWallet(Wallet::CWallet* pwalletIn);
	
	
	/* Syncronize a transaction with the wallet so the wallet class knows what is happening in the core. */
	void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false, bool fConnect = true);
	
	
	/* Check if a transaction is sent from a key we own in our wallet class. */
	bool IsFromMe(CTransaction& tx);
	
	
	/* Gert a transaction from the wallet class to determine and compare data. */
	bool GetTransaction(const uint512& hashTx, Wallet::CWalletTx& wtx);
	
	
	/* Remove a given transaction from the wallet class. */
	void EraseFromWallets(uint512 hash);
	
	
	/* Set the wallet's best chain pointer and data record. */
	void SetBestChain(const CBlockLocator& loc);
	
	
	/* Flag that a transaction has been updated. Signals the walelt to do the same. */
	void UpdatedTransaction(const uint512& hashTx);
	
	
	/* Update the inventory into the wallet class to give wallet an idea of what has been processed / processing. */
	void Inventory(const uint1024& hash);
	
	
	/* Re-Broadcast transactions in the wallet if they haven't been broadcasted to the network yet. */
	void ResendWalletTransactions();
	
	
	/* Output the wallets to the console or STD_OUT. */
	void PrintWallets(const CBlock& block);
	
}

