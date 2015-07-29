/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef RPCCONSOLE_H
#define RPCCONSOLE_H

#include <QDialog>

namespace Ui {
    class RPCConsole;
}
class ClientModel;

/** Local Nexus RPC console. */
class RPCConsole: public QDialog
{
    Q_OBJECT

public:
    explicit RPCConsole(QWidget *parent = 0);
    ~RPCConsole();

    void setClientModel(ClientModel *model);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
    virtual bool eventFilter(QObject* obj, QEvent *event);

private slots:
    void on_lineEdit_returnPressed();

    void on_tabWidget_currentChanged(int index);

public slots:
    void clear();
    void message(int category, const QString &message, bool html = false);
	
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
	
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);
	
	/** Set the Coin Supply Information. **/
	void SetCoinSupply(unsigned int nSupply, unsigned int nIdealSupply);
	
	/** Set the Information Associated with the Prime Channel. **/
	void SetPrimeInfo(double nDiff, unsigned int nReserves, unsigned int nHeight, double nRewards);
	
	/** Set the Information Associated with the Hashing Channel. **/
	void SetHashInfo(double nDiff, unsigned int nReserves, unsigned int nHeight, double nRewards);
	
	
    /** Go forward or back in history */
    void browseHistory(int offset);
	
    /** Scroll console view to end */
    void scrollToEnd();
signals:
    // For RPC command executor
    void stopExecutor();
    void cmdRequest(const QString &command);

private:
    Ui::RPCConsole *ui;
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();
};

#endif // RPCCONSOLE_H
