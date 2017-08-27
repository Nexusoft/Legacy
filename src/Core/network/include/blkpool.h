/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_CORE_NETWORK_INCLUDE_BLKPOOL_H
#define NEXUS_CORE_NETWORK_INCLUDE_BLKPOOL_H

#include "../../../LLC/types/uint1024.h"
#include "../../../LLP/templates/pool.h"

#include "../../../Core/types/include/block.h"

namespace LLD { class CIndexDB; }
namespace LLP { class CNode;    }

namespace Core
{		
	
	
	class CBlkPool : public LLP::CHoldingPool<uint1024, CBlock>
	{
	public:
		Mutex_t INDEXING;
		
		
		/** State level messages to hold information about holding data. */
		enum
		{
			//Location States
			MAINCHAIN      = 10,
			FORKCHAIN      = 11,
			RELAY          = 12,
			DISK           = 13,
			ERROR          = 14,
			
			//Validation States
			GENESIS = 130
		};
		
		
		/** Default Constructor. */
		CBlkPool() : LLP::CHoldingPool<uint1024, CBlock>(60 * 60 * 24) {}
		
		
		/** Run Basic Processing Checks for Block
		 * 
		 * Makes sure the block passes basic checks off chain (Check)
		 * and on-chain (Accept) which work together to verify block data
		 * 
		 * Each stage of validation the block recieves gets a different state in the pool
		 * 
		 * @param[in] blk The block object to process
		 * @param[out] pfrom The node the block was recieved from
		 * 
		 * @return Returns trus if it passes validation checks, false if failed.
		 */
		bool Process(CBlock blk, LLP::CNode* pfrom);
		
		
		/** Check Block before adding
		 * 
		 * Checks a block that is not a part of the blockchain. Verifies basic rules that don't rely on calculations
		 * of previous blocks in the blockchain.
		 * 
		 * @param[in] blk The Block object to check
		 * @param[out] pfrom The Node that block was recievewd from for DoS and Trust filters
		 * 
		 * @return Returns true if the block passes basic tests.
		 * 
		 */
		bool Check(CBlock blk, LLP::CNode* pfrom);
		
		
		/** Accdept the Block into Disk
		 * 
		 * Checks a block to the blockchain data to ensure that it fits the rules that are based on calculations of previous blocks.
		 * This should be done after the block is checked.
		 * 
		 * @param[in] blk The block object to accept
		 * @param[out] pfrom The Node that block was recieved from for DoS and Trust filters
		 * 
		 * @return Returns true if the block is valid to be added to the blockchain.
		 * 
		 */
		bool Accept(CBlock blk, LLP::CNode* pfrom);
		
		
		/** Add to Chain 
		 * 
		 * Adds the block to the main indexing for the chain and calculates state requirements for blocks to be added on top of it
		 * TODO: Make state calculations per block with child class, wrap that on disk and correlate that to checkpoint states
		 * 
		 * @param[in] blk The block object to added
		 * @param[out] pindexNew The new index object to be returned
		 * @param[out] pfrom The Node that block was recieved from for DoS and Trust filters
		 * 
		 * @return Returns true if the block was valid in the adding, False if it failed.
		 * 
		 */
		bool Index(CBlock blk, CBlockIndex* pindexNew, LLP::CNode* pfrom);
		
		
		/** Set Best Block
		 * 
		 * Sets the pointers in CBlockIndex for the best block or the highest block in the blockchain.
		 * Gives credit to miner and connects the inputs marking transactions as spent.
		 * 
		 * @param[in] indexdb The LLD database instance for the transaction (ACID)
		 * @param[in] pindexNew The index object that is being appended as best block
		 * @param[out] pfrom The Node that block was recieved from for Dos and Trust filters
		 * 
		 * @return Returns true if the block was successfully appended as the best block.
		 * 
		 */
		bool Connect(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew, LLP::CNode* pfrom);
		
		
	};
	
}

#endif
