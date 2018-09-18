/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef NEXUS_GUI_H
#define NEXUS_GUI_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QMainWindow>
 #include <QSystemTrayIcon>
#else
 #include <QtWidgets/QMainWindow>
 #include <QtWidgets/QSystemTrayIcon>
#endif

class TransactionTableModel;
class ClientModel;
class WalletModel;
class TransactionView;
class OverviewPage;
class AddressBookPage;
class SendCoinsDialog;
class MessagePage;
class Notificator;
class RPCConsole;

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QTableView;
class QAbstractItemModel;
class QModelIndex;
class QProgressBar;
class QStackedWidget;
class QUrl;
QT_END_NAMESPACE

/**
  Nexus GUI main class. This class represents the main window of the Nexus UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class NexusGUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit NexusGUI(QWidget *parent = 0);
    ~NexusGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    /** Set the wallet model.
        The wallet model represents a Nexus wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel *walletModel);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    ClientModel *clientModel;
    WalletModel *walletModel;

    QStackedWidget *centralWidget;

    OverviewPage *overviewPage;
    QWidget *transactionsPage;
    AddressBookPage *addressBookPage;
    AddressBookPage *receiveCoinsPage;
    SendCoinsDialog *sendCoinsPage;
    MessagePage *messagePage;

    QLabel *labelEncryptionIcon;
    QLabel *labelConnectionsIcon;
    QLabel *labelWeightIcon;
    QLabel *labelBlocksIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;

    QMenuBar *appMenuBar;
    QAction *overviewAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *addressBookAction;
    QAction *messageAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *exportAction;
    QAction *encryptWalletAction;
    QAction *unlockWalletAction;
    QAction *lockWalletAction;
    QAction *backupWalletAction;
    QAction *changePassphraseAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;

    QSystemTrayIcon *trayIcon;
    Notificator *notificator;
    TransactionView *transactionView;
    RPCConsole *rpcConsole;

    QMovie *syncIconMovie;

    /** Create the main UI actions. */
    void createActions();
    /** Create the menu bar and submenus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray (notification) icon */
    void createTrayIcon();

public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);
    void setWeight(double trustWeight, double blockWeight, double interestRate, bool isWaitPeriod);

    /** Notify the user of an error in the network or transaction handling code. */
    void error(const QString &title, const QString &message, bool modal);
    /** Asks the user whether to pay the transaction fee or to cancel the transaction.
       It is currently not possible to pass a return value to another thread through
       BlockingQueuedConnection, so an indirected pointer is used.
       http://bugreports.qt.nokia.com/browse/QTBUG-10440

      @param[in] nFeeRequired       the required fee
      @param[out] payFee            true to pay the fee, false to not pay the fee
    */
    void askFee(qint64 nFeeRequired, bool *payFee);
    void handleURI(QString strURI);

    void gotoMessagePage();
    void gotoMessagePage(QString);

private slots:
    /** Switch to overview (home) page */
    void gotoOverviewPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to address book page */
    void gotoAddressBookPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to send coins page */
    void gotoSendCoinsPage();

    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();
#ifndef Q_WS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif
    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void incomingTransaction(const QModelIndex & parent, int start, int end);
    /** Encrypt the wallet */
    void encryptWallet(bool status);
    /** Backup the wallet */
    void backupWallet();
    /** Change encrypted wallet passphrase */
    void changePassphrase();
    /** Ask for pass phrase to unlock wallet temporarily */
    void tempUnlockWallet();
    /** Ask for pass phrase to unlock wallet perminantly */
    void unlockWallet();
    /** Lock Wallet after Unlocked Manually */
    void lockWallet();

    /** Show window if hidden, unminimize when minimized */
    void showNormalIfMinimized();
    /** Hide window if visible, show if hidden */
    void toggleHidden();
};

#endif
