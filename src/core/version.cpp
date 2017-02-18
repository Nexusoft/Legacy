/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <string>
#include "version.h"


/** Used for Visual Reference Only **/
const std::string CLIENT_NAME("Nexus");


#ifdef USE_LLD
const std::string CLIENT_BUILD("0.3.0.0 [ Tritium (LLD) ]");
#else
const std::string CLIENT_BUILD("0.3.0.0 [ Tritium (BDB) ]");
#endif


/* Update Build Time to Be at Compile Time. */
const std::string CLIENT_DATE(__DATE__ " " __TIME__);


/** Used to determine the current features available on the local database */
const int DATABASE_VERSION =
                    1000000 * DATABASE_MAJOR
                  +   10000 * DATABASE_MINOR 
                  +     100 * DATABASE_REVISION
                  +       1 * DATABASE_BUILD;
