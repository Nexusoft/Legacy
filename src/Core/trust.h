/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_TRUST_H
#define NEXUS_CORE_TRUST_H

namespace Core
{
	
	/* The holding location in memory of all the trust keys on the network and their corresponding statistics. */
	extern CTrustPool cTrustPool;
	
	
	/* __________________________________________________ (Trust Key Methods) __________________________________________________  */
	
	
	
	/* Method to Fire up the Staking Thread. */
	void StartStaking(Wallet::CWallet *pwallet);
	
	
	/* Declration of the Function that will act for the Staking Thread. */
	void StakeMinter(void* parg);
	
}


#endif
