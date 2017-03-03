/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLU_INCLUDE_MUTEX_H
#define NEXUS_LLU_INCLUDE_MUTEX_H

#include <boost/thread/thread.hpp>

#define LOCK(a) boost::lock_guard<boost::mutex> lock(a)

typedef boost::mutex                                         Mutex_t;

#endif
