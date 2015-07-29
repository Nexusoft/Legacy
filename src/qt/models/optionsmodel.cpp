/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "optionsmodel.h"
#include "../core/units.h"
#include <QSettings>

#include "../../start.h"
#include "../../wallet/walletdb.h"

OptionsModel::OptionsModel(QObject *parent) :
    QAbstractListModel(parent)
{
    Init();
}

void OptionsModel::Init()
{
    QSettings settings;

    // These are QT-only settings:
    nDisplayUnit = settings.value("nDisplayUnit", NexusUnits::Niro).toInt();
    bDisplayAddresses = settings.value("bDisplayAddresses", false).toBool();
    fMinimizeToTray = settings.value("fMinimizeToTray", false).toBool();
    fMinimizeOnClose = settings.value("fMinimizeOnClose", false).toBool();
    Core::nTransactionFee = settings.value("nTransactionFee").toLongLong();

    // These are shared with core Nexus; we want
    // command-line options to override the GUI settings:
    if (settings.contains("fUseUPnP"))
        SoftSetBoolArg("-upnp", settings.value("fUseUPnP").toBool());
    if (settings.contains("Net::addrProxy") && settings.value("fUseProxy").toBool())
        SoftSetArg("-proxy", settings.value("Net::addrProxy").toString().toStdString());
    if (settings.contains("detachDB"))
        SoftSetBoolArg("-detachdb", settings.value("detachDB").toBool());
}

bool OptionsModel::Upgrade()
{
    QSettings settings;

    if (settings.contains("bImportFinished"))
        return false; // Already upgraded

    settings.setValue("bImportFinished", true);

    // Move settings from old wallet.dat (if any):
    Wallet::CWalletDB walletdb("wallet.dat");

    QList<QString> intOptions;
    intOptions << "nDisplayUnit" << "nTransactionFee";
    foreach(QString key, intOptions)
    {
        int value = 0;
        if (walletdb.ReadSetting(key.toStdString(), value))
        {
            settings.setValue(key, value);
            walletdb.EraseSetting(key.toStdString());
        }
    }
    QList<QString> boolOptions;
    boolOptions << "bDisplayAddresses" << "fMinimizeToTray" << "fMinimizeOnClose" << "fUseProxy" << "fUseUPnP";
    foreach(QString key, boolOptions)
    {
        bool value = false;
        if (walletdb.ReadSetting(key.toStdString(), value))
        {
            settings.setValue(key, value);
            walletdb.EraseSetting(key.toStdString());
        }
    }
    try
    {
        Net::CAddress addrProxyAddress;
        if (walletdb.ReadSetting("Net::addrProxy", addrProxyAddress))
        {
            Net::addrProxy = addrProxyAddress;
            settings.setValue("Net::addrProxy", Net::addrProxy.ToStringIPPort().c_str());
            walletdb.EraseSetting("Net::addrProxy");
        }
    }
    catch (std::ios_base::failure &e)
    {
        // 0.6.0rc1 saved this as a CService, which causes failure when parsing as a Net::CAddress
        if (walletdb.ReadSetting("Net::addrProxy", Net::addrProxy))
        {
            settings.setValue("Net::addrProxy", Net::addrProxy.ToStringIPPort().c_str());
            walletdb.EraseSetting("Net::addrProxy");
        }
    }
    Init();

    return true;
}


int OptionsModel::rowCount(const QModelIndex & parent) const
{
    return OptionIDRowCount;
}

QVariant OptionsModel::data(const QModelIndex & index, int role) const
{
    if(role == Qt::EditRole)
    {
        QSettings settings;
        switch(index.row())
        {
        case StartAtStartup:
            return QVariant(GetStartOnSystemStartup());
        case MinimizeToTray:
            return QVariant(fMinimizeToTray);
        case MapPortUPnP:
            return settings.value("fUseUPnP", GetBoolArg("-upnp", true));
        case MinimizeOnClose:
            return QVariant(fMinimizeOnClose);
        case ConnectSOCKS4:
            return settings.value("fUseProxy", false);
        case ProxyIP:
            return QVariant(QString::fromStdString(Net::addrProxy.ToStringIP()));
        case ProxyPort:
            return QVariant(Net::addrProxy.GetPort());
        case Fee:
            return QVariant(Core::nTransactionFee);
        case DisplayUnit:
            return QVariant(nDisplayUnit);
        case DisplayAddresses:
            return QVariant(bDisplayAddresses);
        case DetachDatabases:
            return QVariant(Wallet::fDetachDB);
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool OptionsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool successful = true; /* set to false on parse error */
    if(role == Qt::EditRole)
    {
        QSettings settings;
        switch(index.row())
        {
        case StartAtStartup:
            successful = SetStartOnSystemStartup(value.toBool());
            break;
        case MinimizeToTray:
            fMinimizeToTray = value.toBool();
            settings.setValue("fMinimizeToTray", fMinimizeToTray);
            break;
        case MapPortUPnP:
            {
                bool bUseUPnP = value.toBool();
                settings.setValue("fUseUPnP", bUseUPnP);
                Net::MapPort(bUseUPnP);
            }
            break;
        case MinimizeOnClose:
            fMinimizeOnClose = value.toBool();
            settings.setValue("fMinimizeOnClose", fMinimizeOnClose);
            break;
        case ConnectSOCKS4:
            Net::fUseProxy = value.toBool();
            settings.setValue("fUseProxy", Net::fUseProxy);
            break;
        case ProxyIP:
            {
                // Use Net::CAddress to parse and check IP
                Net::CNetAddr addr(value.toString().toStdString());
                if (addr.IsValid())
                {
                    Net::addrProxy.SetIP(addr);
                    settings.setValue("Net::addrProxy", Net::addrProxy.ToStringIPPort().c_str());
                }
                else
                {
                    successful = false;
                }
            }
            break;
        case ProxyPort:
            {
                int nPort = atoi(value.toString().toAscii().data());
                if (nPort > 0 && nPort < std::numeric_limits<unsigned short>::max())
                {
                    Net::addrProxy.SetPort(nPort);
                    settings.setValue("Net::addrProxy", Net::addrProxy.ToStringIPPort().c_str());
                }
                else
                {
                    successful = false;
                }
            }
            break;
        case Fee: {
            Core::nTransactionFee = value.toLongLong();
            settings.setValue("nTransactionFee", Core::nTransactionFee);
            }
            break;
        case DisplayUnit: {
            int unit = value.toInt();
            nDisplayUnit = unit;
            settings.setValue("nDisplayUnit", nDisplayUnit);
            emit displayUnitChanged(unit);
            }
            break;
        case DisplayAddresses: {
            bDisplayAddresses = value.toBool();
            settings.setValue("bDisplayAddresses", bDisplayAddresses);
            }
            break;
        case DetachDatabases: {
            Wallet::fDetachDB = value.toBool();
            settings.setValue("detachDB", Wallet::fDetachDB);
            }
            break;
        default:
            break;
        }
    }
    emit dataChanged(index, index);

    return successful;
}

qint64 OptionsModel::getTransactionFee()
{
    return Core::nTransactionFee;
}

bool OptionsModel::getMinimizeToTray()
{
    return fMinimizeToTray;
}

bool OptionsModel::getMinimizeOnClose()
{
    return fMinimizeOnClose;
}

int OptionsModel::getDisplayUnit()
{
    return nDisplayUnit;
}

bool OptionsModel::getDisplayAddresses()
{
    return bDisplayAddresses;
}
