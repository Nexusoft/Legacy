/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef Nexus_INIT_H
#define Nexus_INIT_H

#include "wallet/wallet.h"

extern Wallet::CWallet* pwalletMain;

void StartShutdown();
void Shutdown(void* parg);
bool AppInit(int argc, char* argv[]);
bool AppInit2(int argc, char* argv[]);

#endif
