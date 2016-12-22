/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <string>
#include "core/version.h"

/** Used for Visual Reference Only **/
const std::string CLIENT_NAME("Nexus");
const std::string CLIENT_BUILD("0.3.0.0 - Beta");
const std::string CLIENT_DATE("December 21st, 2016");

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
