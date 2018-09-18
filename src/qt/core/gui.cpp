/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/


#include "gui.h"
#include "../../main.h"
#include "../models/transactiontablemodel.h"
#include "../pages/addressbookpage.h"
#include "../dialogs/sendcoinsdialog.h"
#include "../pages/messagepage.h"
#include "../dialogs/optionsdialog.h"
#include "../dialogs/aboutdialog.h"
#include "../models/clientmodel.h"
#include "../models/walletmodel.h"
#include "../dialogs/editaddressdialog.h"
#include "../models/optionsmodel.h"
#include "../dialogs/transactiondescdialog.h"
#include "../models/addresstablemodel.h"
#include "../wallet/transactionview.h"
#include "../pages/overviewpage.h"
#include "units.h"
#include "guiconstants.h"
#include "../dialogs/askpassphrasedialog.h"
#include "../util/notificator.h"
#include "../util/guiutil.h"
#include "rpcconsole.h"
#include "../../wallet/wallet.h"

#ifdef Q_WS_MAC
#include "../util/macdockiconhandler.h"
#endif

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QApplication>
 #include <QMainWindow>
 #include <QMenuBar>
 #include <QMenu>
 #include <QTabWidget>
 #include <QVBoxLayout>
 #include <QToolBar>
 #include <QStatusBar>
 #include <QLabel>
 #include <QLineEdit>
 #include <QPushButton>
 #include <QMessageBox>
 #include <QProgressBar>
 #include <QStackedWidget>
 #include <QFileDialog>
#else
 #include <QtWidgets/QApplication>
 #include <QtWidgets/QMainWindow>
 #include <QtWidgets/QMenuBar>
 #include <QtWidgets/QMenu>
 #include <QtWidgets/QTabWidget>
 #include <QtWidgets/QVBoxLayout>
 #include <QtWidgets/QToolBar>
 #include <QtWidgets/QStatusBar>
 #include <QtWidgets/QLabel>
 #include <QtWidgets/QLineEdit>
 #include <QtWidgets/QPushButton>
 #include <QtWidgets/QMessageBox>
 #include <QtWidgets/QProgressBar>
 #include <QtWidgets/QStackedWidget>
 #include <QtWidgets/QFileDialog>
#endif

#include <QDesktopServices>
#include <QDateTime>
#include <QMovie>
#include <QLocale>
#include <QTimer>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QUrl>
#include <QIcon>
#include <iostream>

NexusGUI::NexusGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    changePassphraseAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0)
{
    resize(850, 550);
    setWindowTitle(tr("Nexus Wallet"));
#ifndef Q_WS_MAC
    setWindowIcon(QIcon(":icons/Nexus"));
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    /** Set the Styles of the Qt **/
    setStyleSheet("selection-background-color: #084B8A; background-color: #F7F7F7");
    setAutoFillBackground(true);

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create tabs
    overviewPage = new OverviewPage();

    transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();

    transactionView = new TransactionView(this);
    vbox->addWidget(transactionView);
    transactionsPage->setLayout(vbox);

    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);

    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);

    sendCoinsPage = new SendCoinsDialog(this);

    messagePage = new MessagePage(this);

    centralWidget = new QStackedWidget(this);
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
#ifdef FIRST_CLASS_MESSAGING
    centralWidget->addWidget(messagePage);
#endif
    setCentralWidget(centralWidget);

    // Create status bar
    statusBar();

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setMinimumWidth(68);
    frameBlocks->setMaximumWidth(88);


    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(5,5,5,5);
    frameBlocksLayout->setSpacing(3);
    labelEncryptionIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelWeightIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelEncryptionIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelWeightIcon);
    frameBlocksLayout->addStretch();

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new QProgressBar();
    progressBar->setStyleSheet("QProgressBar { border: 2px solid grey; border-radius: 5px; } QProgressBar::chunk { background-color: #0288D9; width: 20px; }");
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);

    syncIconMovie = new QMovie(":/movies/update_spinner", "gif", this);

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));

    // Doubleclicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    gotoOverviewPage();
}

NexusGUI::~NexusGUI()
{
#ifdef Q_WS_MAC
    delete appMenuBar;
#endif
}

void NexusGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);
    overviewAction->setToolTip(tr("Show general overview of wallet"));
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    historyAction = new QAction(QIcon(":/icons/history"), tr("&Transactions"), this);
    historyAction->setToolTip(tr("Browse transaction history"));
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(QIcon(":/icons/address-book"), tr("&Address Book"), this);
    addressBookAction->setToolTip(tr("Edit the list of stored addresses and labels"));
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Receive"), this);
    receiveCoinsAction->setToolTip(tr("Show the list of addresses for receiving payments"));
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("&Send"), this);
    sendCoinsAction->setToolTip(tr("Send coins to a Nexus address"));
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    messageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message"), this);
    messageAction->setToolTip(tr("Prove you control an address"));
#ifdef FIRST_CLASS_MESSAGING
    messageAction->setCheckable(true);
#endif
    tabGroup->addAction(messageAction);

    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(messageAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(messageAction, SIGNAL(triggered()), this, SLOT(gotoMessagePage()));

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/Nexus"), tr("&About %1").arg(qApp->applicationName()), this);
    aboutAction->setToolTip(tr("Show information about Nexus"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for Nexus"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/Nexus"), tr("Show/Hide &Nexus"), this);
    toggleHideAction->setToolTip(tr("Show or hide the Nexus window"));
    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet"), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet"), this);
    unlockWalletAction->setToolTip(tr("Unlock Wallet for Minting or Transactions"));
    unlockWalletAction->setCheckable(true);
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock Wallet Manually"));
    lockWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet"), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase"), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    openRPCConsoleAction = new QAction(tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(lockWalletAction, SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
}

void NexusGUI::createMenuBar()
{
#ifdef Q_WS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();

#endif

    //set the color of menu bar
    appMenuBar->setStyleSheet("background-color: #F7F7F7; selection-background-color: #084B8A;");

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    file->addAction(backupWalletAction);
    file->addAction(exportAction);
#ifndef FIRST_CLASS_MESSAGING
    file->addAction(messageAction);
#endif
    file->addSeparator();
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(encryptWalletAction);
    settings->addAction(lockWalletAction);
    settings->addAction(unlockWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addSeparator();
    settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

void NexusGUI::createToolBars()
{
    QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(overviewAction);
    toolbar->addAction(sendCoinsAction);
    toolbar->addAction(receiveCoinsAction);
    toolbar->addAction(historyAction);
    toolbar->addAction(addressBookAction);
#ifdef FIRST_CLASS_MESSAGING
    toolbar->addAction(messageAction);
#endif
    toolbar->addAction(exportAction);
}

void NexusGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        if(clientModel->isTestNet())
        {
            QString title_testnet = windowTitle() + QString(" ") + tr("[testnet]");
            setWindowTitle(title_testnet);
#ifndef Q_WS_MAC
            setWindowIcon(QIcon(":icons/Nexus_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/Nexus_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(title_testnet);
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
            }
        }
        else
        {

#ifndef Q_WS_MAC
            setWindowIcon(QIcon(":icons/Nexus"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/Nexus"));
#endif
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks());
        connect(clientModel, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString, bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
    }
}

void NexusGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        messagePage->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon popup for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(tempUnlockWallet()));
    }
}

void NexusGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_WS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("Nexus client"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(messageAction);
#ifndef FIRST_CLASS_MESSAGING
    trayIconMenu->addSeparator();
#endif
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
#ifndef Q_WS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(tr("Nexus-qt"), trayIcon);
}

#ifndef Q_WS_MAC
void NexusGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers "show/hide Nexus"
        toggleHideAction->trigger();
    }
}
#endif

void NexusGUI::toggleHidden()
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else
        hide();
}

void NexusGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void NexusGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void NexusGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n Connection(s) to Nexus", "", count));
}

void NexusGUI::setWeight(double trustWeight, double blockWeight, double interestRate, bool isWaitPeriod)
{
    double dPercent = (trustWeight + blockWeight) / 100.0;

    QString icon;
    if(dPercent < 0.2)
        icon = ":/icons/transaction_1";
    else if(dPercent < 0.4)
        icon = ":/icons/transaction_2";
    else if(dPercent < 0.6)
        icon = ":/icons/transaction_3";
    else if(dPercent < 0.8)
        icon = ":/icons/transaction_4";
    else
        icon = ":/icons/transaction_5";

    if (isWaitPeriod == true)
    {
        icon = ":/icons/notsynced";
        labelWeightIcon->setToolTip(tr("Waiting the required 72 hours\n since Trust Key creation.\nStaking will begin after..."));
    }
    else if(dPercent == 0)
    {
        icon = ":/icons/transaction_0";
        labelWeightIcon->setToolTip(tr("Staking Inactive...\nNo Coins to Stake"));
    }
    else
    {
        labelWeightIcon->setToolTip(tr("%1 % Interest Rate").arg(interestRate * 100.0, 0, 'f', 4) + QString("\n") + tr("%1 % Stake Weight").arg(dPercent * 100.0, 0, 'f', 2) + QString("\n") + tr("%1 % Trust Weight").arg((100.0 * trustWeight) / 90.0, 0, 'f', 2)+ QString("\n") + tr("%1 % Block Weight").arg((100.0 * blockWeight) / 10.0, 0, 'f', 2));
    }

    labelWeightIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
}

void NexusGUI::setNumBlocks(int count)
{
    // don't show / hide progressBar and it's label if we have no connection(s) to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    // set trust and block weight on this signal for now...
    setWeight(clientModel->getTrustWeight(), clientModel->getBlockWeight(), clientModel->getInterestRate(), clientModel->getIsWaitPeriod());

    int nTotalBlocks = clientModel->getNumBlocksOfPeers();
    QString tooltip;

    if(count < nTotalBlocks)
    {
        int nRemainingBlocks = nTotalBlocks - count;
        float nPercentageDone = count / (nTotalBlocks * 0.01f);

        if (clientModel->getStatusBarWarnings() == "")
        {
            progressBarLabel->setText(tr("Synchronizing with network..."));
            progressBarLabel->setVisible(true);
            progressBar->setFormat(tr("~%n block(s) remaining", "", nRemainingBlocks));
            progressBar->setMaximum(nTotalBlocks);
            progressBar->setValue(count);
            progressBar->setVisible(true);
        }
        else
        {
            progressBarLabel->setText(clientModel->getStatusBarWarnings());
            progressBarLabel->setVisible(true);
            progressBar->setVisible(false);
        }
        tooltip = tr("Downloaded %1 of %2 blocks of transaction history (%3% done).").arg(count).arg(nTotalBlocks).arg(nPercentageDone, 0, 'f', 2);
    }
    else
    {
        if (clientModel->getStatusBarWarnings() == "")
            progressBarLabel->setVisible(false);
        else
        {
            progressBarLabel->setText(clientModel->getStatusBarWarnings());
            progressBarLabel->setVisible(true);
        }
        progressBar->setVisible(false);
        tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
    }

    QDateTime now = QDateTime::fromTime_t(GetUnifiedTimestamp());//QDateTime::currentDateTime();
    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(now);
    QString text;

    // Represent time from last generated block in human readable text
    if(secs <= 0)
    {
        // Fully up to date. Leave text empty.
    }
    else if(secs < 60)
    {
        text = tr("%n second(s) ago","",secs);
    }
    else if(secs < 60*60)
    {
        text = tr("%n minute(s) ago","",secs/60);
    }
    else if(secs < 24*60*60)
    {
        text = tr("%n hour(s) ago","",secs/(60*60));
    }
    else
    {
        text = tr("%n day(s) ago","",secs/(60*60*24));
    }

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60 && count >= nTotalBlocks)
    {
        tooltip = tr("Up to date") + QString(".\n") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
    }
    else
    {
        tooltip = tr("Catching up...") + QString("\n") + tooltip;
        labelBlocksIcon->setMovie(syncIconMovie);
        syncIconMovie->start();
    }

    if(!text.isEmpty())
    {
        tooltip += QString("\n");
        tooltip += tr("Last received block was generated %1.").arg(text);
    }

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void NexusGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void NexusGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_WS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void NexusGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_WS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void NexusGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                NexusUnits::formatWithUnit(NexusUnits::Nexus, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Sending..."), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void NexusGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? tr("Sent transaction") :
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(NexusUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
    }
}

void NexusGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void NexusGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
}

void NexusGUI::gotoAddressBookPage()
{
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
}

void NexusGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
}

void NexusGUI::gotoSendCoinsPage()
{
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void NexusGUI::gotoMessagePage()
{
#ifdef FIRST_CLASS_MESSAGING
    messageAction->setChecked(true);
    centralWidget->setCurrentWidget(messagePage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
#else
    messagePage->show();
    messagePage->setFocus();
#endif
}

void NexusGUI::gotoMessagePage(QString addr)
{
    gotoMessagePage();
    messagePage->setAddress(addr);
}

void NexusGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void NexusGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        gotoSendCoinsPage();
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            sendCoinsPage->handleURI(uri.toString());
        }
    }

    event->acceptProposedAction();
}

void NexusGUI::handleURI(QString strURI)
{
    gotoSendCoinsPage();
    sendCoinsPage->handleURI(strURI);

    if(!isActiveWindow())
        activateWindow();

    showNormalIfMinimized();
}

void NexusGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setVisible(false);
        lockWalletAction->setVisible(false);
        unlockWalletAction->setVisible(false);
        encryptWalletAction->setVisible(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(Wallet::fWalletUnlockMintOnly? tr("Wallet is <b>encrypted</b> and currently <b>unlocked for block minting only</b>") : tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        //unlockWalletAction->setChecked(true);
        unlockWalletAction->setVisible(false);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        //lockWalletAction->setChecked(true);
        unlockWalletAction->setVisible(true);
        unlockWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void NexusGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void NexusGUI::backupWallet()
{
    #if QT_VERSION < QT_VERSION_CHECK(5,0,0)
     QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    #else
     QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    #endif
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void NexusGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void NexusGUI::lockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Unlocked)
    {
        Wallet::fWalletUnlockMintOnly = false;
        walletModel->setWalletLocked(true, "");
    }
}

void NexusGUI::unlockWallet()
{
    if(!walletModel)
        return;

    AskPassphraseDialog dlg(AskPassphraseDialog::UnlockOrMint, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void NexusGUI::tempUnlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void NexusGUI::showNormalIfMinimized()
{
    if(!isVisible()) // Show, if hidden
        show();
    if(isMinimized()) // Unminimize, if minimized
        showNormal();
}
