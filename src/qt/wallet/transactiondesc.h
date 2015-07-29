/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef TRANSACTIONDESC_H
#define TRANSACTIONDESC_H

#include <QString>
#include <QObject>
#include <string>

namespace Wallet
{
	class CWallet;
	class CWalletTx;
}

/** Provide a human-readable extended HTML description of a transaction.
 */
class TransactionDesc: public QObject
{
    Q_OBJECT
public:
    static QString toHTML(Wallet::CWallet *wallet, Wallet::CWalletTx &wtx);
private:
    TransactionDesc() {}

    static QString FormatTxStatus(const Wallet::CWalletTx& wtx);
};

#endif // TRANSACTIONDESC_H
