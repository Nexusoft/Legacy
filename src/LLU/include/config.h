/*******************************************************************************************
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright Nexus Developers 2014 - 2017
			
			http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_LLU_INCLUDE_CONFIG_H
#define NEXUS_LLU_INCLUDE_CONFIG_H


/* Read the Config file from the Disk. */
void ReadConfigFile(std::map<std::string, std::string>& mapSettingsRet, std::map<std::string, std::vector<std::string> >& mapMultiSettingsRet);


/* Setup PID file for Linux users. */
void CreatePidFile(const boost::filesystem::path &path, pid_t pid);


/* Check if set to start when system boots. */
bool GetStartOnSystemStartup();


/* Setup to auto start when system boots. */
bool SetStartOnSystemStartup(bool fAutoStart);


/* Get the default directory Nexus data is stored in. */
boost::filesystem::path GetDefaultDataDir(std::string strName = "Nexus");


/* Get the Location of the Config File. */
boost::filesystem::path GetConfigFile();


/* Get the Location of the PID File. */
boost::filesystem::path GetPidFile();


/* Get the location that Nexus data is being stored in. */
const boost::filesystem::path &GetDataDir(bool fNetSpecific = true);


#endif
