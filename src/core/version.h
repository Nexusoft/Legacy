/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_CORE_VERSION_H
#define NEXUS_CORE_VERSION_H

#include <string>

#define DATABASE_MAJOR       0
#define DATABASE_MINOR       1
#define DATABASE_REVISION    1
#define DATABASE_BUILD       0


/** Used to determine the current features available on the local database */
extern const int DATABASE_VERSION;


/** These external variables are the display only variables. They are used to track the updates of Nexus independent of Database and Protocol Upgrades. **/
extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;


#endif
