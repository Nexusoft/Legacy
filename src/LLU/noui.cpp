/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../LLU/ui_interface.h"

#include <string>
#include "../core/core.h"
#include "../main.h"

int ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style)
{
    printf("%s: %s\n", caption.c_str(), message.c_str());
    fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    return 4;
}

bool ThreadSafeAskFee(int64 nFeeRequired, const std::string& strCaption)
{
    return true;
}

void MainFrameRepaint()
{
}

void AddressBookRepaint()
{
}

void InitMessage(const std::string &message)
{
}

std::string _(const char* psz)
{
    return psz;
}

void QueueShutdown()
{
    // Without UI, Shutdown can simply be started in a new thread
    CreateThread(Shutdown, NULL);
}

