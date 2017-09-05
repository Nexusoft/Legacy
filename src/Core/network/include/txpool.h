/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_CORE_NETWORK_INCLUDE_TXPOOL_H
#define NEXUS_CORE_NETWORK_INCLUDE_TXPOOL_H

#include "../../../LLC/types/uint1024.h"
#include "../../../LLP/templates/pool.h"

#include "../../types/include/transaction.h"

namespace LLD { class CIndexDB; }

namespace Core
{		
	class CTxPool : public LLP::CHoldingPool<uint512, CTransaction>
	{
		/* Map to Contain list of Inputs spending by UNVERIFIED transactions. */
		std::map<uint512, uint512> mapInputLocks;
		
	public:
		/** State level messages to hold information about holding data. */
		enum
		{
			//Location States
			ACCEPTED       = 10,
			INDEXED        = 11,
			CONNECTED      = 12,
			DISK           = 13,
			
			//invalid states
			ORPHANED       = 50
		};
		
		
		/** Default Constructor. */
		CTxPool() : LLP::CHoldingPool<uint512, CTransaction>(60 * 60 * 24) {}
		
		
		/** Accept Transaction into Tx Pool
		 * 
		 * @param[in] indexdb The Lower Level Database Instance
		 * @param[in] tx The transaction to accept into the pool 
		 * @param[in] fCheckInputs Verify the transaction inputs are not already spent
		 * @param[out] pfMissingInputs Return if inputs are missing, used to determine orphans
		 * 
		 * @return Returns True if the transaction passed basic requirements, False if invalid
		 * 
		 */
		bool Accept(LLD::CIndexDB& indexdb, CTransaction &tx, bool fCheckInputs, bool* pfMissingInputs = NULL);
		
		
		/** Check Inputs for Conflicts
		 * 
		 *  Input Conflicts come if one transaction is trying to spend the same inputs as a transaciton that already exists.
		 * 
		 * @param[in] tx The transaction object to check
		 * 
		 * @return Returns True if there are no input conflicts, flase if there are
		 * 
		 */
		bool CheckInputs(CTransaction& tx);
		
		
		/** Confirm Transaction
		 * 
		 * Confirm the Transaction as added to the blockchain but let it remain in pool until expiration time for memory relays
		 * 
		 * @param[in] tx The input Transaction
		 * 
		 */
		void Confirm(Core::CTransaction& tx);

	};
	
}

#endif
