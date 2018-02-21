/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "../util/ui_interface.h"
#include "../util/util.h"
#include "qtipcserver.h"

using namespace boost::interprocess;
using namespace boost::posix_time;
using namespace boost;
using namespace std;

void ipcShutdown()
{
    message_queue::remove(NEXUS_URI_QUEUE_NAME);
}

void ipcThread(void* parg)
{

}

void ipcInit()
{
#ifdef MAC_OSX
    // TODO: implement Nexus: URI handling the Mac Way
    return;
#endif
#ifdef WIN32
    // TODO: THOROUGHLY test boost::interprocess fix,
    // and make sure there are no Windows argument-handling exploitable
    // problems.
    return;
#endif

}
