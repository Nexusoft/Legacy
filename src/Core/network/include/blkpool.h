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
		
		
		/** Default Constructor. */
		CBlkPool() : LLP::CHoldingPool<uint1024, CBlock>(60 * 60 * 24) {}
		
		
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
		 * @param[in] nFile The File number block will be appended to (NOTE: To be deprecated into LLD)
		 * @param[in] nFilePos The Binary position of the RAW block data in the blk.....dat files (NOTE: To be deprecated into LLD)
		 * @param[out] pfrom The Node that block was recieved from for DoS and Trust filters
		 * 
		 * @return Returns true if the block was valid in the adding, False if it failed.
		 * 
		 */
		bool AddToChain(CBlock blk, unsigned int nFile, unsigned int nFilePos, LLP::CNode* pfrom);
		
		
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
		bool SetBestBlock(LLD::CIndexDB& indexdb, CBlockIndex* pindexNew, LLP::CNode* pfrom);
		
		
	};
	
}

#endif
