/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef CLIENTMODEL_H
#define CLIENTMODEL_H

#include <QObject>

class OptionsModel;
class AddressTableModel;
class TransactionTableModel;
class CWallet;

QT_BEGIN_NAMESPACE
class QDateTime;
QT_END_NAMESPACE

/** Model for Nexus network client. */
class ClientModel : public QObject
{
    Q_OBJECT
public:
    explicit ClientModel(OptionsModel *optionsModel, QObject *parent = 0);

    OptionsModel *getOptionsModel();

    int getNumConnections() const;
    int getNumBlocks() const;
    int getNumBlocksAtStartup();

    QDateTime getLastBlockDate() const;

    //! Return true if client connected to testnet
    bool isTestNet() const;


    //! Return true if core is doing initial block download
    bool inInitialBlockDownload() const;

    double getTrustWeight() const;
    double getBlockWeight() const;
    double getInterestRate() const;
    
    //! Return true if in 72hr wait period for staking
    bool getIsWaitPeriod() const;
    
    /** Return the Total Coin supply from the Block Chain. **/
    unsigned int GetCoinSupply() const;
    unsigned int GetIdealSupply() const;

    double GetPrimeDifficulty() const;
    double GetHashDifficulty() const;

    unsigned int GetPrimeReserves() const;
    unsigned int GetHashReserves() const;

    double GetPrimeReward() const;
    double GetHashReward() const;

    unsigned int GetPrimeHeight() const;
    unsigned int GetHashHeight() const;

    //! Return conservative estimate of total number of blocks, or 0 if unknown
    int getNumBlocksOfPeers() const;


    //! Return warnings to be displayed in status bar
    QString getStatusBarWarnings() const;

    QString formatFullVersion() const;
    QString formatBuildDate() const;
    QString clientName() const;

private:
    OptionsModel *optionsModel;

    int cachedNumConnections;
    int cachedNumBlocks;
    QString cachedStatusBar;

    int numBlocksAtStartup;

signals:
    void numConnectionsChanged(int count);
    void numBlocksChanged(int count);

    //! Asynchronous error notification
    void error(const QString &title, const QString &message, bool modal);

public slots:

private slots:
    void update();
};

#endif // CLIENTMODEL_H
