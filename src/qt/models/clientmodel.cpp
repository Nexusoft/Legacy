/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include "clientmodel.h"
#include "../core/guiconstants.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "transactiontablemodel.h"

#include "../../core/core.h"

#include <QDateTime>
#include <algorithm>

ClientModel::ClientModel(OptionsModel *optionsModel, QObject *parent) :
    QObject(parent), optionsModel(optionsModel),
    cachedNumConnections(0), cachedNumBlocks(0)
{
    numBlocksAtStartup = -1;
}

int ClientModel::getNumConnections() const
{
    return Net::vNodes.size();
}

int ClientModel::getNumBlocks() const
{
    return Core::nBestHeight;
}

bool ClientModel::getIsWaitPeriod() const
{
    return Core::fIsWaitPeriod;
}

double ClientModel::getTrustWeight() const
{
    return Core::dTrustWeight;
}

double ClientModel::getBlockWeight() const
{
    return Core::dBlockWeight;
}

double ClientModel::getInterestRate() const
{
    return Core::dInterestRate;
}

unsigned int ClientModel::GetCoinSupply() const
{
    return Core::GetMoneySupply(Core::pindexBest) / 1000000;
}


unsigned int ClientModel::GetIdealSupply() const
{
    return Core::CompoundSubsidy(Core::GetChainAge(Core::pindexBest->GetBlockTime())) / 1000000;
}


double ClientModel::GetPrimeDifficulty() const
{
    return Core::GetDifficulty(Core::GetNextTargetRequired(Core::pindexBest, 1, false), 1);
}


double ClientModel::GetHashDifficulty() const
{
    return Core::GetDifficulty(Core::GetNextTargetRequired(Core::pindexBest, 2, false), 2);
}


double ClientModel::GetPrimeReward() const
{
    return Core::GetCoinbaseReward(Core::pindexBest, 1, 0) / 1000000.0;
}

double ClientModel::GetHashReward() const
{
    return Core::GetCoinbaseReward(Core::pindexBest, 2, 0) / 1000000.0;
}

unsigned int ClientModel::GetPrimeReserves() const
{
    const Core::CBlockIndex* pindexPrime = Core::GetLastChannelIndex(Core::pindexBest, 1);
    int64 nPrimeReserve = 0;
    for(int nIndex = 0; nIndex < 3; nIndex++)
        nPrimeReserve += pindexPrime->nReleasedReserve[nIndex];

    return std::max((unsigned int)(nPrimeReserve / 1000000), 0u);
}


unsigned int ClientModel::GetHashReserves() const
{
    const Core::CBlockIndex* pindexHash = Core::GetLastChannelIndex(Core::pindexBest, 2);
    int64 nHashReserve = 0;
    for(int nIndex = 0; nIndex < 3; nIndex++)
        nHashReserve += pindexHash->nReleasedReserve[nIndex];

    return std::max((unsigned int)(nHashReserve / 1000000), 0u);
}


unsigned int ClientModel::GetPrimeHeight() const
{
    return Core::GetLastChannelIndex(Core::pindexBest, 1)->nChannelHeight;
}


unsigned int ClientModel::GetHashHeight() const
{
    return Core::GetLastChannelIndex(Core::pindexBest, 2)->nChannelHeight;
}


int ClientModel::getNumBlocksAtStartup()
{
    if (numBlocksAtStartup == -1) numBlocksAtStartup = getNumBlocks();
    return numBlocksAtStartup;
}

QDateTime ClientModel::getLastBlockDate() const
{
    return QDateTime::fromTime_t(Core::pindexBest->GetBlockTime());
}

void ClientModel::update()
{
    int newNumConnections = getNumConnections();
    int newNumBlocks = getNumBlocks();
    QString newStatusBar = getStatusBarWarnings();

    if(cachedNumConnections != newNumConnections)
        emit numConnectionsChanged(newNumConnections);

    if(cachedNumBlocks != newNumBlocks || cachedStatusBar != newStatusBar)
        emit numBlocksChanged(newNumBlocks);

    cachedNumConnections = newNumConnections;
    cachedNumBlocks = newNumBlocks;
    cachedStatusBar = newStatusBar;
}

bool ClientModel::isTestNet() const
{
    return fTestNet;
}

bool ClientModel::inInitialBlockDownload() const
{
    return Core::IsInitialBlockDownload();
}

int ClientModel::getNumBlocksOfPeers() const
{
    return Core::GetNumBlocksOfPeers();
}

QString ClientModel::getStatusBarWarnings() const
{
    return QString::fromStdString(Core::GetWarnings("statusbar"));
}

OptionsModel *ClientModel::getOptionsModel()
{
    return optionsModel;
}

QString ClientModel::formatFullVersion() const
{
    return QString::fromStdString(FormatFullVersion());
}

QString ClientModel::formatBuildDate() const
{
    return QString::fromStdString(CLIENT_DATE);
}

QString ClientModel::clientName() const
{
    return QString::fromStdString(CLIENT_NAME);
}
