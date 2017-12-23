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
		WRITE_REGISTER = 0,  //WRITE_STATE <lexer-id>
		READ_REGISTER  = 1,   //READ_STATE  <lexer-id>
		
		/* Control Flow */
		IF     = 2, // if(boolean) {
		ELSE   = 3, // } else {
		ENDIF  = 4, // }
		
		
		FOR    = 5, // for(var ; boolean ; operator) {
		ENDFOR = 6, // }
		
		
		WHILE     = 7, // while(boolean) {
		ENDWHILE  = 8, // }
		
		
		/* Data Typs. */
		UINT8     = 9, //unsigned char
		UINT16    = 10, //unsigned short
		UINT32    = 11, //unsigned int
		UINT64    = 12, //unsigned long long
		UINT128   = 13, //uint128
		UINT256   = 14, //uint256
		UINT512   = 15, //uint512
		UINT576   = 16, //uint576
		UINT1024  = 17, //uint1024
		
		BIGNUM    = 18, //CBigNum (openssl)
		
		STRING    = 19, //std::string or char*
		BOOL      = 20, //bool
		
		
		SINT8     = 21, //signed char
		SINT16    = 22, //signed short
		SINT32    = 23, //signed int
		SINT64    = 24, //signed int64
        
        //RANGE 25 - 31 for extensibility
		
		
		/* Hashing (Skien and Keccak) */
		SK32      = 32,
		SK64      = 33,
		SK256     = 34,
		SK512     = 35,
		SK576     = 36,
		SK1024    = 37,
		
		
		/* Elliptic Curve */
		ECDSA_VERIFY = 38,
		ECDSA_SIGN   = 39,
		
		
		/* Network */
		TIMESTAMP    = 40, //Core::UnifiedTimestamp()
		
		
		/* Nexus Account Specific */
		ADDRESS      = 41, //get the account by ID
		BALANCE      = 42, //account balance by id <ID = UINT512>
		CALLER       = 43, //get caller account by ID (the signchain contract that called this one
		CALLDATA     = 44, //ability to send data into a contract level function (TODO: think on this more)
		
        //RANGE 45 - 63 for caller and account extensibility
		
		/* Bitwise */
		BIT_OR           = 64, // c++ symbol |
		BIT_XOR          = 65, // c++ symbol ^
		BIT_AND          = 66, // c++ symbol &
		BIT_RSHIFT       = 69, // c++ symbol >>
		BIT_LSHIFT       = 70, // c++ symbol <<
        BIT_FLIP         = 71, // c++ symbol ~
		
		
		/* Operators */
		ADD       = 72, //ADD reg_a reg_b (+)
		DIV       = 73, //DIV reg_a reg_b (/)
		MUL       = 74, //MUL reg_a reg_b (*)
		SUB       = 75, //SUB reg_a reg_B (-)
		EXP       = 76, //EXP base exp (base ^ exp)
		MOD       = 77, //MOD base div (base % div)
		LOG       = 78 //LOG base num

        //79 - 255 TO BE DETERMINED
	}

}
