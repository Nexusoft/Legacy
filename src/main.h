/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_MAIN_H
#define NEXUS_MAIN_H

#include "Wallet/wallet.h"
#include "Core/include/manager.h"


/** Manager for all wallet data. */
extern Wallet::CWallet* pwalletMain;

/** Manager for all node inventory. **/
extern Core::NodeManager* pNodeManager;

/** Manager for all protocol inventory. **/
extern Core::InventoryManager* pInvManager;


void StartShutdown();
void Shutdown(void* parg);
bool AppInit(int argc, char* argv[]);
bool AppInit2(int argc, char* argv[]);

#endif
