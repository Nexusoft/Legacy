/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "wallet/db.h"
#include "wallet/walletdb.h"
#include "net/rpcserver.h"
#include "main.h"
#include "LLP/coreserver.h"
#include "LLP/miningserver.h"
#include "LLD/keychain.h"
#include "core/unifiedtime.h"
#include "util/util.h"
#include "util/ui_interface.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#ifndef WIN32
#include <signal.h>
#else
#include <csignal>
#endif

using namespace std;
using namespace boost;

Wallet::CWallet* pwalletMain;
LLP::Server<LLP::CoreLLP>* LLP_SERVER;

namespace LLP
{
    LLP::Server<LLP::MiningLLP>* MINING_LLP;
}

//////////////////////////////////////////////////////////////////////////////
//
// Shutdown
//

void ExitTimeout(void* parg)
{
    #ifdef WIN32
        Sleep(5000);
        ExitProcess(0);
    #endif
}

void StartShutdown()
{
    #ifdef QT_GUI
        // ensure we leave the Qt main loop for a clean GUI exit (Shutdown() is called afterwards)
        QueueShutdown();
    #else
        // Without UI, Shutdown() can simply be started in a new thread
        CreateThread(Shutdown, NULL);
    #endif
}

void Shutdown(void* parg)
{
    static CCriticalSection cs_Shutdown;
    static bool fTaken;
    bool fFirstThread = false;
    {
        TRY_LOCK(cs_Shutdown, lockShutdown);
        if (lockShutdown)
        {
            fFirstThread = !fTaken;
            fTaken = true;
        }
    }
    static bool fExit;
    if (fFirstThread)
    {
        fShutdown = true;

        Wallet::DBFlush(false);
        Net::StopNode();
        Wallet::DBFlush(true);
        boost::filesystem::remove(GetPidFile());
        Core::UnregisterWallet(pwalletMain);
        delete pwalletMain;
        CreateThread(ExitTimeout, NULL);
        Sleep(50);
        printf("Nexus exiting\n\n");
        fExit = true;
    #ifndef QT_GUI
            // ensure non UI client get's exited here, but let Nexus-Qt reach return 0;
            exit(0);
    #endif
    }
    else
    {
        while (!fExit)
            Sleep(500);
        Sleep(100);
        ExitThread(0);
    }
}

void HandleSIGTERM(int)
{
    StartShutdown();

    fRequestShutdown = true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Start
//
#if !defined(QT_GUI)
int main(int argc, char* argv[])
{
    bool fRet = false;
    fRet = AppInit(argc, argv);

    if (fRet && fDaemon)
        return 0;

    return 1;
}
#endif

bool AppInit(int argc, char* argv[])
{
    bool fRet = false;
    try
    {
        fRet = AppInit2(argc, argv);
    }
    catch (std::exception& e) {
        PrintException(&e, "AppInit()");
    } catch (...) {
        PrintException(NULL, "AppInit()");
    }
    if (!fRet)
        Shutdown(NULL);
    return fRet;
}

bool AppInit2(int argc, char* argv[])
{
    #ifdef _MSC_VER
        // Turn off microsoft heap dump noise
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
    #endif
    #if _MSC_VER >= 1400
        // Disable confusing "helpful" text message on abort, ctrl-c
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    #endif
    #ifndef WIN32
        umask(077);
    #endif
    #ifndef WIN32
        // Clean shutdown on SIGTERM
        struct sigaction sa;
        sa.sa_handler = HandleSIGTERM;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        //catch all signals to flag fShutdown for all threads
        sigaction(SIGABRT, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

    #else
        //catch all signals to flag fShutdown for all threads
        signal(SIGABRT, HandleSIGTERM);
        signal(SIGILL, HandleSIGTERM);
        signal(SIGINT, HandleSIGTERM);
        signal(SIGTERM, HandleSIGTERM);

    #ifdef SIGBREAK
        signal(SIGBREAK, HandleSIGTERM);
    #endif

    #endif

    //
    // Parameters
    //
    #if !defined(QT_GUI)
        ParseParameters(argc, argv);
        if (!boost::filesystem::is_directory(GetDataDir(false)))
        {
            fprintf(stderr, "Error: Specified directory does not exist\n");
            Shutdown(NULL);
        }
        ReadConfigFile(mapArgs, mapMultiArgs);
    #endif

    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        string strUsage = string() +
          _("Nexus version") + " " + FormatFullVersion() + "\n\n" +
          _("Usage:") + "\t\t\t\t\t\t\t\t\t\t\n" +
            "  Nexus [options]                   \t  " + "\n" +
            "  Nexus [options] <command> [params]\t  " + _("Send command to -server or Nexus") + "\n" +
            "  Nexus [options] help              \t\t  " + _("List commands") + "\n" +
            "  Nexus [options] help <command>    \t\t  " + _("Get help for a command") + "\n" +
          _("Options:") + "\n" +
            "  -conf=<file>     \t\t  " + _("Specify configuration file (default: nexus.conf)") + "\n" +
            "  -pid=<file>      \t\t  " + _("Specify pid file (default: Nexus.pid)") + "\n" +
            "  -wallet=<file>   \t\t  " + _("Specify wallet fille (default: wallet.dat)") + "\n" +
            "  -gen             \t\t  " + _("Generate coins") + "\n" +
            "  -gen=0           \t\t  " + _("Don't generate coins") + "\n" +
            "  -min             \t\t  " + _("Start minimized") + "\n" +
            "  -splash          \t\t  " + _("Show splash screen on startup (default: 1)") + "\n" +
            "  -datadir=<dir>   \t\t  " + _("Specify data directory") + "\n" +
            "  -dbcache=<n>     \t\t  " + _("Set database cache size in megabytes (default: 25)") + "\n" +
            "  -dblogsize=<n>   \t\t  " + _("Set database disk log size in megabytes (default: 100)") + "\n" +
            "  -timeout=<n>     \t  "   + _("Specify connection timeout (in milliseconds)") + "\n" +
            "  -proxy=<ip:port> \t  "   + _("Connect through socks4 proxy") + "\n" +
            "  -dns             \t  "   + _("Allow DNS lookups for addnode and connect") + "\n" +
            "  -port=<port>     \t\t  " + _("Listen for connections on <port> (default: 9323 or testnet: 8313)") + "\n" +
            "  -maxconnections=<n>\t  " + _("Maintain at most <n> connections to peers (default: 125)") + "\n" +
            "  -addnode=<ip>    \t  "   + _("Add a node to connect to and attempt to keep the connection open") + "\n" +
            "  -addseednode=<ip>    "   + _("Add a node to list of hardcoded seed nodes") + "\n" +
            "  -connect=<ip>    \t\t  " + _("Connect only to the specified node") + "\n" +
            "  -listen          \t  "   + _("Accept connections from outside (default: 1)") + "\n" +
            "  -unified         \t  "   + _("Enable sending unified time samples. Used for seed nodes") + "\n" +
            "  -unifiedport     \t  "   + _("Listen for unified time samples on <port> (default: 9324 or testnet: 8329). Does not affect outgoing port.") + "\n" +
        #ifdef QT_GUI
                    "  -lang=<lang>     \t\t  " + _("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        #endif
                    "  -dnsseed         \t  "   + _("Find peers using DNS lookup (default: 1)") + "\n" +
                    "  -banscore=<n>    \t  "   + _("Threshold for disconnecting misbehaving peers (default: 100)") + "\n" +
                    "  -bantime=<n>     \t  "   + _("Number of seconds to keep misbehaving peers from reconnecting (default: 86400)") + "\n" +
                    "  -maxreceivebuffer=<n>\t  " + _("Maximum per-connection receive buffer, <n>*1000 bytes (default: 10000)") + "\n" +
                    "  -maxsendbuffer=<n>\t  "   + _("Maximum per-connection send buffer, <n>*1000 bytes (default: 10000)") + "\n" +
        #ifdef USE_UPNP
        #if USE_UPNP
                    "  -upnp            \t  "   + _("Use Universal Plug and Play to map the listening port (default: 1)") + "\n" +
        #else
                    "  -upnp            \t  "   + _("Use Universal Plug and Play to map the listening port (default: 0)") + "\n" +
        #endif
                    "  -detachdb        \t  "   + _("Detach block and address databases. Increases shutdown time (default: 0)") + "\n" +
        #endif
                    "  -paytxfee=<amt>  \t  "   + _("Fee per KB to add to transactions you send") + "\n" +
        #ifdef QT_GUI
                    "  -server          \t\t  " + _("Accept command line and JSON-RPC commands") + "\n" +
        #endif
        #if !defined(WIN32) && !defined(QT_GUI)
                    "  -daemon          \t\t  " + _("Run in the background as a daemon and accept commands") + "\n" +
        #endif
                    "  -testnet         \t\t  " + _("Use the test network") + "\n" +
                    "  -regtest         \t\t  " + _("Run nexus in regression test mode for lower difficulty, etc. For local code testing only.") + "\n" +
                    "  -istimeseed      \t\t  " + _("Advanced option. Use to set this as a dns time seed e.g. for bootstraping new test block chain.") + "\n" +
                    "  -debug           \t\t  " + _("Output extra debugging information") + "\n" +
                    "  -logtimestamps   \t  "   + _("Prepend debug output with timestamp") + "\n" +
                    "  -printtoconsole  \t  "   + _("Send trace/debug info to console instead of debug.log file") + "\n" +
        #ifdef WIN32
                    "  -printtodebugger \t  "   + _("Send trace/debug info to debugger") + "\n" +
        #endif
                    "  -llpallowip=<ip> \t  "   + _("Allow mining from specified IP address or range (192.168.6.* for example") + "\n" +
                    "  -banned=<ip>     \t  "   + _("Manually Ban Addresses from Config File") + "\n" +
                    "  -mining             \t  "   + _("Allow mining (default: 0)") + "\n" +
                    "  -miningport=<port> "     + _("Listen for mining connections on <port> (default: 9325)") + "\n" +
                    "  -rpcuser=<user>  \t  "   + _("Username for JSON-RPC connections") + "\n" +
                    "  -rpcpassword=<pw>\t  "   + _("Password for JSON-RPC connections") + "\n" +
                    "  -rpcport=<port>  \t\t  " + _("Listen for JSON-RPC connections on <port> (default: 9325)") + "\n" +
                    "  -rpcallowip=<ip> \t\t  " + _("Allow JSON-RPC connections from specified IP address") + "\n" +
                    "  -rpcconnect=<ip> \t  "   + _("Send commands to node running on <ip> (default: 127.0.0.1)") + "\n" +
                    "  -blocknotify=<cmd> "     + _("Execute command when the best block changes (%s in cmd is replaced by block hash)") + "\n" +
                    "  -upgradewallet   \t  "   + _("Upgrade wallet to latest format") + "\n" +
                    "  -keypool=<n>     \t  "   + _("Set key pool size to <n> (default: 100)") + "\n" +
                    "  -rescan          \t  "   + _("Rescan the block chain for missing wallet transactions") + "\n" +
                    "  -checkblocks=<n> \t\t  " + _("How many blocks to check at startup (default: 2500, 0 = all)") + "\n" +
                    "  -checklevel=<n>  \t\t  " + _("How thorough the block verification is (0-6, default: 1)") + "\n";

                strUsage += string() +
                    _("\nSSL options: (see the Nexus Wiki for SSL setup instructions)") + "\n" +
                    "  -rpcssl                                \t  " + _("Use OpenSSL (https) for JSON-RPC connections") + "\n" +
                    "  -rpcsslcertificatechainfile=<file.cert>\t  " + _("Server certificate file (default: server.cert)") + "\n" +
                    "  -rpcsslprivatekeyfile=<file.pem>       \t  " + _("Server private key (default: server.pem)") + "\n" +
                    "  -rpcsslciphers=<ciphers>               \t  " + _("Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)") + "\n";

                strUsage += string() +
                    "  -?               \t\t  " + _("This help message") + "\n";

                // Remove tabs
                strUsage.erase(std::remove(strUsage.begin(), strUsage.end(), '\t'), strUsage.end());
        #if defined(QT_GUI) && defined(WIN32)
                // On windows, show a message box, as there is no stderr
                ThreadSafeMessageBox(strUsage, _("Usage"), wxOK | wxMODAL);
        #else
                fprintf(stderr, "%s", strUsage.c_str());
        #endif
        return false;
    }

    fTestNet = GetBoolArg("-testnet", false);
    fLispNet = GetBoolArg("-lispnet", false);
    if(fLispNet)
        fTestNet = true;

    fIsTimeSeed = GetBoolArg("-istimeseed", false);

    fDebug = GetBoolArg("-debug", false);
    Wallet::fDetachDB = GetBoolArg("-detachdb", false);

    #if !defined(WIN32) && !defined(QT_GUI)
        fDaemon = GetBoolArg("-daemon");
    #else
        fDaemon = false;
    #endif

        if (fDaemon)
            fServer = true;
        else
            fServer = GetBoolArg("-server");

        /* force fServer when running without GUI */
    #if !defined(QT_GUI)
        fServer = true;
    #endif
    fPrintToConsole = GetBoolArg("-printtoconsole");
    fPrintToDebugger = GetBoolArg("-printtodebugger");
    fLogTimestamps = GetBoolArg("-logtimestamps");

    #ifndef QT_GUI
        for (int i = 1; i < argc; i++)
            if (!IsSwitchChar(argv[i][0]) && !(strlen(argv[i]) >= 7 && strncasecmp(argv[i], "Nexus:", 7) == 0))
                fCommandLine = true;

        if (fCommandLine)
        {
            int ret = Net::CommandLineRPC(argc, argv);
            exit(ret);
        }
    #endif

    #if !defined(WIN32) && !defined(QT_GUI) && !defined(NO_DAEMON)
        if (fDaemon)
        {
            // Daemonize
            pid_t pid = fork();
            if (pid < 0)
            {
                fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
                return false;
            }
            if (pid > 0)
            {
                CreatePidFile(GetPidFile(), pid);
                return true;
            }

            pid_t sid = setsid();
            if (sid < 0)
                fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
        }
    #endif

    if(fTestNet)
    {
        Wallet::CScript scriptSig;
        scriptSig.SetNexusAddress(Core::TESTNET_DUMMY_ADDRESS);
        Core::TESTNET_DUMMY_SIGNATURE = (std::vector<unsigned char>)scriptSig;


        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::TESTNET_DUMMY_AMBASSADOR_RECYCLED);
        Core::TESTNET_DUMMY_SIGNATURE_AMBASSADOR_RECYCLED = (std::vector<unsigned char>)scriptSig;


        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::TESTNET_DUMMY_DEVELOPER_RECYCLED);
        Core::TESTNET_DUMMY_SIGNATURE_DEVELOPER_RECYCLED = (std::vector<unsigned char>)scriptSig;
    }

    Wallet::CScript scriptSig;
    for(int i = 0; i < 13; i++)
    {
        /* Set the script byte code for ambassador addresses old. */
        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::CHANNEL_ADDRESSES[i]);
        Core::AMBASSADOR_SCRIPT_SIGNATURES[i] = (std::vector<unsigned char>)scriptSig;

        if(GetBoolArg("-dumpsignatures", false))
        {
            printf("Ambassador Script %s\n", Core::CHANNEL_ADDRESSES[i].c_str());
            PrintHex(scriptSig);
            printf("\n\n");
        }

        /* Set the script byte code for ambassador addresses new. */
        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::AMBASSADOR_ADDRESSES_RECYCLED[i]);
        Core::AMBASSADOR_SCRIPT_SIGNATURES_RECYCLED[i] = (std::vector<unsigned char>)scriptSig;

        if(GetBoolArg("-dumpsignatures", false))
        {
            printf("Ambassador New Script %s\n", Core::AMBASSADOR_ADDRESSES_RECYCLED[i].c_str());
            PrintHex(Core::AMBASSADOR_SCRIPT_SIGNATURES_RECYCLED[i]);
            printf("\n\n");
        }

        /* Set the script byte code for developer addresses old. */
        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::DEVELOPER_ADDRESSES[i]);
        Core::DEVELOPER_SCRIPT_SIGNATURES[i] = (std::vector<unsigned char>)scriptSig;

        if(GetBoolArg("-dumpsignatures", false))
        {
            printf("Developer Script %s\n", Core::DEVELOPER_ADDRESSES[i].c_str());
            PrintHex(Core::DEVELOPER_SCRIPT_SIGNATURES[i]);
            printf("\n\n");
        }

        /* Set the script byte code for developer addresses new. */
        scriptSig.clear();
        scriptSig.SetNexusAddress(Core::DEVELOPER_ADDRESSES_RECYCLED[i]);
        Core::DEVELOPER_SCRIPT_SIGNATURES_RECYCLED[i] = (std::vector<unsigned char>)scriptSig;

        if(GetBoolArg("-dumpsignatures", false))
        {
            printf("Developer New Script %s\n", Core::DEVELOPER_ADDRESSES_RECYCLED[i].c_str());
            PrintHex(Core::DEVELOPER_SCRIPT_SIGNATURES_RECYCLED[i]);
            printf("\n\n");
        }
    }

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("Nexus version %s (%s)\n", FormatFullVersion().c_str(), CLIENT_DATE.c_str());
    printf("Default data directory %s\n", GetDefaultDataDir().string().c_str());


    for(auto node : mapMultiArgs["-banned"])
        printf("PERMANENT BAN %s\n", node.c_str());

    #ifdef USE_LLD
        InitMessage(_("Initializing LLD Keychains..."));
        LLD::RegisterKeychain("blkindex", "blkindex");
        LLD::RegisterKeychain("trust", "trust");
    #endif

    InitMessage(_("Initializing Unified Time..."));
    printf("Initializing Unified Time...\n");
    InitializeUnifiedTime();

    if (!fDebug)
        ShrinkDebugFile();

    /** Locks to the Local Database. This will keep another process from using the Nexus Databases. **/
    boost::filesystem::path pathLockFile = GetDataDir() / ".lock";
    FILE* file = fopen(pathLockFile.string().c_str(), "a");
    if (file) fclose(file);

    static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
    if (!lock.try_lock())
    {
        ThreadSafeMessageBox(strprintf(_("Cannot obtain a lock on data directory %s.  Nexus is probably already running."), GetDataDir().string().c_str()), _("Nexus"), wxOK|wxMODAL);
        return false;
    }
    std::ostringstream strErrors;

    /**Wallet filename validity check. **/
    if (!boost::filesystem::native(GetArg("-wallet", "wallet.dat")))
    {
	printf("Invalid wallet file name");
        strErrors << _("Invalid wallet filename") << "\n";
        ThreadSafeMessageBox(strErrors.str(), _("Nexus"), wxOK | wxMODAL);
        return false;
    }


    /** Run the process as Daemon RPC/LLP Server if Flagged. **/
    if (fDaemon)
        fprintf(stdout, "Nexus server starting\n");

    int64 nStart;


    /** Load Peer Addresses from the Address Database. **/
    InitMessage(_("Loading addresses..."));
    printf("Loading addresses...\n");
    nStart = GetTimeMillis();
    if (!Wallet::LoadAddresses())
        strErrors << _("Error loading addr.dat") << "\n";
    printf(" addresses   %15" PRI64d "ms\n", GetTimeMillis() - nStart);



    /** Load the Block Index Database. **/
    InitMessage(_("Loading block index..."));
    printf("Loading block index...\n");
    nStart = GetTimeMillis();
    if (!Core::LoadBlockIndex())
        strErrors << _("Error loading blkindex.dat") << "\n";




    /** Catch a Shutdown if requested thus far. [Initialization can take some time] **/
    if (fRequestShutdown)
    {
        printf("Shutdown requested. Exiting.\n");
        return false;
    }
    printf(" block index %15" PRI64d "ms\n", GetTimeMillis() - nStart);

    /** Load the Wallet Database. **/
    InitMessage(_("Loading wallet..."));
    printf("Loading wallet...\n");
    nStart = GetTimeMillis();
    bool fFirstRun;
    pwalletMain = new Wallet::CWallet(GetArg("-wallet", "wallet.dat"));
    int nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);
    if (nLoadWalletRet != Wallet::DB_LOAD_OK)
    {
        if (nLoadWalletRet == Wallet::DB_CORRUPT)
            strErrors << _("Error loading wallet.dat: Wallet corrupted") << "\n";
        else if (nLoadWalletRet == Wallet::DB_TOO_NEW)
            strErrors << _("Error loading wallet.dat: Wallet requires newer version of Nexus") << "\n";
        else if (nLoadWalletRet == Wallet::DB_NEED_REWRITE)
        {
            strErrors << _("Wallet needed to be rewritten: restart Nexus to complete") << "\n";
            printf("%s", strErrors.str().c_str());
            ThreadSafeMessageBox(strErrors.str(), _("Nexus"), wxOK | wxICON_ERROR | wxMODAL);
            return false;
        }
        else
            strErrors << _("Error loading wallet.dat") << "\n";
    }

    if (GetBoolArg("-upgradewallet", fFirstRun))
    {
        int nMaxVersion = GetArg("-upgradewallet", 0);
        if (nMaxVersion == 0) // the -walletupgrade without argument case
        {
            printf("Performing wallet upgrade to %i\n", Wallet::FEATURE_LATEST);
            nMaxVersion = DATABASE_VERSION;
            pwalletMain->SetMinVersion(Wallet::FEATURE_LATEST); // permanently upgrade the wallet immediately
        }
        else
            printf("Allowing wallet upgrade up to %i\n", nMaxVersion);
        if (nMaxVersion < pwalletMain->GetVersion())
            strErrors << _("Cannot downgrade wallet") << "\n";
        pwalletMain->SetMaxVersion(nMaxVersion);
    }

    if (fFirstRun)
    {
        // Create new keyUser and set as default key
        RandAddSeedPerfmon();

        std::vector<unsigned char> newDefaultKey;
        if (!pwalletMain->GetKeyFromPool(newDefaultKey, false))
            strErrors << _("Cannot initialize keypool") << "\n";
        pwalletMain->SetDefaultKey(newDefaultKey);
        if (!pwalletMain->SetAddressBookName(Wallet::NexusAddress(pwalletMain->vchDefaultKey), ""))
            strErrors << _("Cannot write default address") << "\n";
    }

    printf("%s", strErrors.str().c_str());
    printf(" wallet      %15" PRI64d "ms\n", GetTimeMillis() - nStart);

    Core::RegisterWallet(pwalletMain);
    Core::CBlockIndex *pindexRescan = Core::pindexBest;

    if (GetBoolArg("-rescan"))
        pindexRescan = Core::pindexGenesisBlock;

     /* OMIT for Now
    else
    {
        Wallet::CWalletDB walletdb("wallet.dat");
        Core::CBlockLocator locator;
        if (walletdb.ReadBestBlock(locator))
            pindexRescan = locator.GetBlockIndex();
    }
    */

    if (Core::pindexBest != pindexRescan && Core::pindexBest && pindexRescan && Core::pindexBest->nHeight > pindexRescan->nHeight)
    {
        InitMessage(_("Rescanning..."));
        printf("Rescanning last %i blocks (from block %i)...\n", Core::pindexBest->nHeight - pindexRescan->nHeight, pindexRescan->nHeight);
        nStart = GetTimeMillis();
        pwalletMain->ScanForWalletTransactions(pindexRescan, true);
        printf(" rescan      %15" PRI64d "ms\n", GetTimeMillis() - nStart);
    }


    InitMessage(_("Done loading"));
    printf("Done loading\n");

    //// debug print
    printf("mapBlockIndex.size() = %d\n",   Core::mapBlockIndex.size());
    printf("nBestHeight = %d\n",            Core::nBestHeight);
    printf("setKeyPool.size() = %d\n",      pwalletMain->setKeyPool.size());
    printf("mapWallet.size() = %d\n",       pwalletMain->mapWallet.size());
    printf("mapAddressBook.size() = %d\n",  pwalletMain->mapAddressBook.size());

    if (!strErrors.str().empty())
    {
        ThreadSafeMessageBox(strErrors.str(), _("Nexus"), wxOK | wxICON_ERROR | wxMODAL);
        return false;
    }

    // Add wallet transactions that aren't already in a block to mapTransactions
    //pwalletMain->ReacceptWalletTransactions();

    // Note: Nexus-QT stores several settings in the wallet, so we want
    // to load the wallet BEFORE parsing command-line arguments, so
    // the command-line/nexus.conf settings override GUI setting.
    if (mapArgs.count("-timeout"))
    {
        int nNewTimeout = GetArg("-timeout", 30000);
        if (nNewTimeout > 0 && nNewTimeout < 600000)
            Net::nConnectTimeout = nNewTimeout;
    }

    if (mapArgs.count("-printblock"))
    {
        string strMatch = mapArgs["-printblock"];
        int nFound = 0;
        for (map<uint1024, Core::CBlockIndex*>::iterator mi = Core::mapBlockIndex.begin(); mi != Core::mapBlockIndex.end(); ++mi)
        {
            uint1024 hash = (*mi).first;
            if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
            {
                Core::CBlockIndex* pindex = (*mi).second;
                Core::CBlock block;
                block.ReadFromDisk(pindex);
                block.BuildMerkleTree();
                block.print();
                printf("\n");
                nFound++;
            }
        }
        if (nFound == 0)
            printf("No blocks matching %s were found\n", strMatch.c_str());
        return false;
    }

    if (mapArgs.count("-proxy"))
    {
        Net::fUseProxy = true;
        Net::addrProxy = Net::CService(mapArgs["-proxy"], 9050);
        if (!Net::addrProxy.IsValid())
        {
            ThreadSafeMessageBox(_("Invalid -proxy address"), _("Nexus"), wxOK | wxMODAL);
            return false;
        }
    }

    bool fTor = (Net::fUseProxy && Net::addrProxy.GetPort() == 9050);
    if (fTor)
    {
        // Use SoftSetBoolArg here so user can override any of these if they wish.
        // Note: the GetBoolArg() calls for all of these must happen later.
        SoftSetBoolArg("-listen", false);
        SoftSetBoolArg("-irc", false);
        SoftSetBoolArg("-dnsseed", false);
        SoftSetBoolArg("-upnp", false);
        SoftSetBoolArg("-dns", false);
    }

    Net::fAllowDNS = GetBoolArg("-dns");
    fNoListen = !GetBoolArg("-listen", true);

    if (!fNoListen)
    {
        std::string strError;
        if (!Net::BindListenPort(strError))
        {
            ThreadSafeMessageBox(strError, _("Nexus"), wxOK | wxMODAL);
            return false;
        }
    }

    if (mapArgs.count("-addnode"))
    {
        BOOST_FOREACH(string strAddr, mapMultiArgs["-addnode"])
        {
            Net::CAddress addr(Net::CService(strAddr, Net::GetDefaultPort(), Net::fAllowDNS));
            addr.nTime = 0; // so it won't relay unless successfully connected
            if (addr.IsValid())
                Net::addrman.Add(addr, Net::CNetAddr("127.0.0.1"));
        }
    }
    
    if (mapArgs.count("-paytxfee"))
    {
        if (!ParseMoney(mapArgs["-paytxfee"], Core::nTransactionFee) || Core::nTransactionFee < Core::MIN_TX_FEE)
        {
            ThreadSafeMessageBox(_("Invalid amount for -paytxfee=<amount>"), _("Nexus"), wxOK | wxMODAL);
            return false;
        }
        if (Core::nTransactionFee > 0.25 * COIN)
            ThreadSafeMessageBox(_("Warning: -paytxfee is set very high.  This is the transaction fee you will pay if you send a transaction."), _("Nexus"), wxOK | wxICON_EXCLAMATION | wxMODAL);
    }

    if (mapArgs.count("-reservebalance")) // Nexus: reserve balance amount
    {
        int64 nReserveBalance = 0;
        if (!ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
        {
            ThreadSafeMessageBox(_("Invalid amount for -reservebalance=<amount>"), _("Nexus"), wxOK | wxMODAL);
            return false;
        }
    }


    //
    // Start the node
    //

    /** Wait for Unified Time if First Start. **/
    if (GetBoolArg("-istimeseed",false)) {
        printf("WARNING: -istimeseed Was set, not waiting for unified time.\n");

        fTimeUnified = true;
    }
    else {
        printf("Waiting for unified time...\n");
        while(!fTimeUnified)
            Sleep(1000);
    }

    /* Initialize the Core LLP if it is enabled. */
    if(GetBoolArg("-unified", false)) {
        InitMessage(_("Initializing Core LLP..."));
        printf("%%%%%%%% Initializing Core LLP...\n");
        LLP_SERVER = new LLP::Server<LLP::CoreLLP>(GetArg("-unifiedport", fLispNet ? LISPNET_CORE_LLP_PORT : fTestNet ? TESTNET_CORE_LLP_PORT : NEXUS_CORE_LLP_PORT), 5, true, 2, 5, 5);
    }


    /* Initialize the Mining LLP if it is enabled. */
    if(GetBoolArg("-mining", false)) {
        InitMessage(_("Initializing Mining LLP..."));
        printf("%%%%%%%%%% Initializing Mining LLP...\n");

        LLP::MINING_LLP = new LLP::Server<LLP::MiningLLP>(GetArg("-miningport", fLispNet ? LISPNET_MINING_LLP_PORT : fTestNet ? TESTNET_MINING_LLP_PORT : NEXUS_MINING_LLP_PORT), GetArg("-mining_threads", 10), true, GetArg("-mining_cscore", 5), GetArg("-mining_rscore", 50), GetArg("-mining_timout", 60));
    }

    if (!Core::CheckDiskSpace())
        return false;

    RandAddSeedPerfmon();

    if (!CreateThread(Net::StartNode, NULL))
        ThreadSafeMessageBox(_("Error: CreateThread(StartNode) failed"), _("Nexus"), wxOK | wxMODAL);

    #ifndef QT_GUI
    if(GetBoolArg("-stake", false))
    {
    #else
    if(GetBoolArg("-stake", true))
    {
    #endif
        CreateThread(Core::StakeMinter, NULL);

        printf("%%%%%%%%%%%%%%%% Staking Thread Initialized...\n");
    }

    if (fServer)
        CreateThread(Net::ThreadRPCServer, NULL);

    //CreateThread(DebugThread, NULL);
    #ifdef QT_GUI
        if (GetStartOnSystemStartup())
            SetStartOnSystemStartup(true); // Remove startup links
    #endif

    #if !defined(QT_GUI)
        while (!fShutdown)
            Sleep(5000);
    #endif

    return true;
}
