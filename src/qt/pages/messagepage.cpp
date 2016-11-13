/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include <string>
#include <vector>

#include <QClipboard>
#include <QInputDialog>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>

#include "../../main.h"
#include "../../wallet/wallet.h"
#include "../../LLU/util.h"

#include "messagepage.h"
#include "ui_messagepage.h"

#include "addressbookpage.h"
#include "../LLU/guiutil.h"
#include "../models/walletmodel.h"

MessagePage::MessagePage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessagePage)
{
    ui->setupUi(this);

    GUIUtil::setupAddressWidget(ui->signFrom, this);
}

MessagePage::~MessagePage()
{
    delete ui;
}

void MessagePage::setModel(WalletModel *model)
{
    this->model = model;
}

void MessagePage::setAddress(QString addr)
{
    ui->signFrom->setText(addr);
    ui->message->setFocus();
}

void MessagePage::on_pasteButton_clicked()
{
    setAddress(QApplication::clipboard()->text());
}

void MessagePage::on_addressBookButton_clicked()
{
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        setAddress(dlg.getReturnValue());
    }
}

void MessagePage::on_copyToClipboard_clicked()
{
    QApplication::clipboard()->setText(ui->signature->toPlainText());
}

void MessagePage::on_signMessage_clicked()
{
    QString address = ui->signFrom->text();

    Wallet::NexusAddress addr(address.toStdString());
    if (!addr.IsValid())
    {
        QMessageBox::critical(this, tr("Error signing"), tr("%1 is not a valid address.").arg(address),
                              QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        return;
    }

    Wallet::CKey key;
    if (!pwalletMain->GetKey(addr, key))
    {
        QMessageBox::critical(this, tr("Error signing"), tr("Private key for %1 is not available.").arg(address),
                              QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << Core::strMessageMagic;
    ss << ui->message->document()->toPlainText().toStdString();

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(SK256(ss.begin(), ss.end()), vchSig))
    {
        QMessageBox::critical(this, tr("Error signing"), tr("Sign failed"),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
	
	ui->signFrom->setStyleSheet("background-color: #F7F7F7");
    ui->signature->setText(QString::fromStdString(EncodeBase64(&vchSig[0], vchSig.size())));
    ui->signature->setFont(GUIUtil::NexusAddressFont());
}

void MessagePage::on_verifyMessage_clicked()
{
	std::string strSign     = ui->signature->toPlainText().toStdString();

	bool fInvalid = false;
	std::vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

	if (fInvalid)
	{
	    QMessageBox::critical(this, tr("Error verifying"), tr("Malformed Base64 Encoding"),
                            QMessageBox::Abort, QMessageBox::Abort);
							
		ui->signFrom->setStyleSheet("background-color: #F20F26");
        return;
	}

    CDataStream ss(SER_GETHASH, 0);
    ss << Core::strMessageMagic;
    ss << ui->message->document()->toPlainText().toStdString();

	Wallet::CKey key;
	if (!key.SetCompactSignature(SK256(ss.begin(), ss.end()), vchSig))
	{
	    QMessageBox::critical(this, tr("Error verifying"), tr("Invalid Signature"),
                            QMessageBox::Abort, QMessageBox::Abort);
		
		ui->signFrom->setStyleSheet("background-color: #F20F26");
        return;
	}
	
	Wallet::NexusAddress addressCheck(ui->signFrom->text().toStdString());
	Wallet::NexusAddress address(key.GetPubKey());
	if(!address.IsValid() || !(address == addressCheck))
	{
		ui->signFrom->setStyleSheet("background-color: #F20F26");
        return;
	}

	ui->signFrom->setStyleSheet("background-color: #0CF03A");
	ui->signFrom->setFont(GUIUtil::NexusAddressFont());
}
