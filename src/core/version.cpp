/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <string>
#include "core/version.h"

/** Used for Visual Reference Only **/
const std::string CLIENT_NAME("Nexus");

/* The database type used (Berklee DB or Lower Level Database) */
#ifdef x64
	#if defined USE_LLD
		const std::string CLIENT_BUILD("0.2.3.6 [LLD] 64Bit");
	#elif defined USE_LEVELDB
		const std::string CLIENT_BUILD("0.2.3.6 [LVD] 64Bit");
	#else
		const std::string CLIENT_BUILD("0.2.3.6 [BDB] 64Bit");
	#endif
#else
	#if defined USE_LLD
		const std::string CLIENT_BUILD("0.2.3.6 [LLD] 32Bit");
	#elif defined USE_LEVELDB
		const std::string CLIENT_BUILD("0.2.3.6 [LVD] 32Bit");
	#else
		const std::string CLIENT_BUILD("0.2.3.6 [BDB] 32Bit");
	#endif
#endif

const std::string CLIENT_DATE(__DATE__ " " __TIME__);

/** Used to determine the current features available on the local database */
 const int DATABASE_VERSION =
                    1000000 * DATABASE_MAJOR
                  +   10000 * DATABASE_MINOR 
                  +     100 * DATABASE_REVISION
                  +       1 * DATABASE_BUILD;

/** Used to determine the features available in the Nexus Network **/
const int PROTOCOL_VERSION =
                   1000000 * PROTOCOL_MAJOR
                 +   10000 * PROTOCOL_MINOR
                 +     100 * PROTOCOL_REVISION
                 +       1 * PROTOCOL_BUILD;

/** Used to Lock-Out Nodes that are running a protocol version that is too old, 
    Or to allow certain new protocol changes without confusing Old Nodes. **/
const int MIN_PROTO_VERSION = 10000;
