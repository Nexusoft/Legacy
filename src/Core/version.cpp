/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <string>
#include "include/version.h"

#include "../LLU/include/debug.h"

/* Used for Visual Reference Only */
#ifdef USE_LLD
const std::string DATABASE_NAME("LLD");
#else
const std::string DATABASE_NAME("BDB");
#endif

#define DATABASE_MAJOR       0
#define DATABASE_MINOR       1
#define DATABASE_REVISION    1
#define DATABASE_BUILD       0

const int DATABASE_VERSION =
                    1000000 * DATABASE_MAJOR
                  +   10000 * DATABASE_MINOR 
                  +     100 * DATABASE_REVISION
                  +       1 * DATABASE_BUILD;


/* Used for Visual Reference Only */
const std::string CLIENT_NAME("Tritium");

#define CLIENT_MAJOR       0
#define CLIENT_MINOR       3
#define CLIENT_REVISION    0
#define CLIENT_BUILD       0

const int CLIENT_VERSION =
                    1000000 * CLIENT_MAJOR
                  +   10000 * CLIENT_MINOR 
                  +     100 * CLIENT_REVISION
                  +       1 * CLIENT_BUILD;


const std::string CLIENT_BUILD("Alpha");


/* Update Build Time to Be at Compile Time. */
const std::string CLIENT_DATE(__DATE__ " " __TIME__);

						
std::string FormatVersion(int nVersion)
{
    if (nVersion % 100 == 0)
        return strprintf("%d.%d.%d", nVersion/1000000, (nVersion/10000)%100, (nVersion/100)%100);
    else
        return strprintf("%d.%d.%d.%d", nVersion/1000000, (nVersion/10000)%100, (nVersion/100)%100, nVersion%100);
}


std::string FormatSubVersion()
{
	 std::ostringstream ss;
	 ss << "Nexus " << FormatVersion(CLIENT_VERSION) << " - " << CLIENT_NAME << " " << CLIENT_BUILD << " [" << DATABASE_NAME << "]";
	 
    return ss.str();
}


std::string FormatFullVersion()
{
    std::ostringstream ss;
    ss << FormatSubVersion();
	 
    ss << " | DB: "    << FormatVersion(DATABASE_VERSION);
    ss << " | PROTO: " << FormatVersion(LLP::PROTOCOL_VERSION);
    
    return ss.str();
}
