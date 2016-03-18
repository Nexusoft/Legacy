/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../main.h" // for pwalletMain
#include "../net/rpcserver.h"
#include "../util/ui_interface.h"

#include <boost/lexical_cast.hpp>

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#define printf OutputDebugStringF

// using namespace boost::asio;
using namespace json_spirit;
using namespace std;

namespace Net
{
	extern Object JSONRPCError(int code, const string& message);

	class CTxDump
	{
	public:
		Core::CBlockIndex *pindex;
		int64 nValue;
		bool fSpent;
		Wallet::CWalletTx* ptx;
		int nOut;
		CTxDump(Wallet::CWalletTx* ptx = NULL, int nOut = -1)
		{
			pindex = NULL;
			nValue = 0;
			fSpent = false;
			this->ptx = ptx;
			this->nOut = nOut;
		}
	};

	Value importprivkey(const Array& params, bool fHelp)
	{
		if (fHelp || params.size() < 1 || params.size() > 2)
			throw runtime_error(
				"importprivkey <Nexusprivkey> [label]\n"
				"Adds a private key (as returned by dumpprivkey) to your wallet.");

		string strSecret = params[0].get_str();
		string strLabel = "";
		if (params.size() > 1)
			strLabel = params[1].get_str();
		Wallet::NexusSecret vchSecret;
		bool fGood = vchSecret.SetString(strSecret);

		if (!fGood) throw JSONRPCError(-5,"Invalid private key");
		if (pwalletMain->IsLocked())
			throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
		if (Wallet::fWalletUnlockMintOnly) // Nexus: no importprivkey in mint-only mode
			throw JSONRPCError(-102, "Wallet is unlocked for minting only.");

		Wallet::CKey key;
		bool fCompressed;
		Wallet::CSecret secret = vchSecret.GetSecret(fCompressed);
		key.SetSecret(secret, fCompressed);
		Wallet::NexusAddress vchAddress = Wallet::NexusAddress(key.GetPubKey());

		{
			LOCK2(Core::cs_main, pwalletMain->cs_wallet);

			pwalletMain->MarkDirty();
			pwalletMain->SetAddressBookName(vchAddress, strLabel);

			if (!pwalletMain->AddKey(key))
				throw JSONRPCError(-4,"Error adding key to wallet");

			pwalletMain->ScanForWalletTransactions(Core::pindexGenesisBlock, true);
			pwalletMain->ReacceptWalletTransactions();
		}

		MainFrameRepaint();

		return Value::null;
	}

	Value dumpprivkey(const Array& params, bool fHelp)
	{
		if (fHelp || params.size() != 1)
			throw runtime_error(
				"dumpprivkey <Nexusaddress>\n"
				"Reveals the private key corresponding to <Nexusaddress>.");

		string strAddress = params[0].get_str();
		Wallet::NexusAddress address;
		if (!address.SetString(strAddress))
			throw JSONRPCError(-5, "Invalid Nexus address");
		if (pwalletMain->IsLocked())
			throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
		if (Wallet::fWalletUnlockMintOnly) // Nexus: no dumpprivkey in mint-only mode
			throw JSONRPCError(-102, "Wallet is unlocked for minting only.");
		Wallet::CSecret vchSecret;
		bool fCompressed;
		if (!pwalletMain->GetSecret(address, vchSecret, fCompressed))
			throw JSONRPCError(-4,"Private key for address " + strAddress + " is not known");
		return Wallet::NexusSecret(vchSecret, fCompressed).ToString();
	}

}
