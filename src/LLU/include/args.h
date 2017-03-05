/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLU_INCLUDE_ARGS_H
#define NEXUS_LLU_INCLUDE_ARGS_H

#include <map>
#include <string>
#include <vector>

extern std::map<std::string, std::string> mapArgs;
extern std::map<std::string, std::vector<std::string> > mapMultiArgs;
extern bool fDebug;
extern bool fPrintToConsole;
extern bool fPrintToDebugger;
extern bool fRequestShutdown;
extern bool fShutdown;
extern bool fDaemon;
extern bool fServer;
extern bool fCommandLine;
extern std::string strMiscWarning;
extern bool fTestNet;
extern bool fNoListen;
extern bool fLogTimestamps;


/**
* Return string argument or default value
*
* @param strArg Argument to get (e.g. "-foo")
* @param default (e.g. "1")
* @return command-line argument or default value
*/
std::string GetArg(const std::string& strArg, const std::string& strDefault);

/**
* Return integer argument or default value
*
* @param strArg Argument to get (e.g. "-foo")
* @param default (e.g. 1)
* @return command-line argument (0 if invalid number) or default value
*/
int64 GetArg(const std::string& strArg, int64 nDefault);

/**
* Return boolean argument or default value
*
* @param strArg Argument to get (e.g. "-foo")
* @param default (true or false)
* @return command-line argument or default value
*/
bool GetBoolArg(const std::string& strArg, bool fDefault=false);

/**
* Set an argument if it doesn't already have a value
*
* @param strArg Argument to set (e.g. "-foo")
* @param strValue Value (e.g. "1")
* @return true if argument gets set, false if it already had a value
*/
bool SoftSetArg(const std::string& strArg, const std::string& strValue);

/**
* Set a boolean argument if it doesn't already have a value
*
* @param strArg Argument to set (e.g. "-foo")
* @param fValue Value (e.g. false)
* @return true if argument gets set, false if it already had a value
*/
bool SoftSetBoolArg(const std::string& strArg, bool fValue);

#endif
