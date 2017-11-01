/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_CORE_INCLUDE_OPERATIONS_H
#define NEXUS_CORE_INCLUDE_OPERATIONS_H


namespace Core
{
	
	/* The data needs to be held in a lexer id map
	 * 
	 * This can be stored in non-volatile memory
	 * If this value is changes as more calls to functions are executed (authorized)
	 * then this value is changed by the history of each call based on each Contracting
	 */

	/* Contracting Operations */
	enum
	{
		/* State Storage */
		WRITE_REGISTER   //WRITE_STATE <lexer-id>
		READ_REGISTER    //READ_STATE  <lexer-id>
		
		/* Logic Flow */
		IF     // if(boolean) {
		ELSE   // } else {
		ENDIF  // }
		
		
		FOR    // for(var ; boolean ; operator) {
		ENDFOR // }
		
		
		WHILE     // while(boolean) {
		ENDWHILE  // }
		
		
		/* Data Typs. */
		UINT8     //unsigned char
		UINT16    //unsigned short
		UINT32    //unsigned int
		UINT64    //unsigned long long
		UINT128   //uint128
		UINT256   //uint256
		UINT512   //uint512
		UINT576   //uint576
		UINT1024  //uint1024
		
		BIGNUM    //CBigNum
		
		STRING    //std::string or char*
		BOOL      //bool
		
		
		SINT8
		SINT16
		SINT32
		SINT64
		
		
		/* Hashing */
		SK32
		SK64
		SK256
		SK512
		SK576
		SK1024
		
		
		/* Elliptic Curve */
		ECDSA_VERIFY
		ECDSA_SIGN
		
		
		/* Network */
		TIMESTAMP //Core::UnifiedTimestamp()
		
		
		/* Nexus Account Specific */
		ADDRESS  //get the account by ID
		BALANCE  //
		CALLER 
		CALLDATA
		
		
		/* Bitwise */
		OR
		XOR 
		AND 
		NOT 
		BOTH      //yes / no 
		NONE      //not yes / not no
		EQUALS 
		RSHIFT
		LSHIFT
		
		
		/* Mathematics */
		ADD 
		DIV
		MUL 
		SUB 
		EXP       //EXP base exp (base ^ exp)
		MOD       //MOD base div (base % div)
		LOG       //LOG base num
		
		
		/* Functions */
		FUNCTION  //paramters 
	}

}
