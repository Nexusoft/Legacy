/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QDialog>
#else
 #include <QtWidgets/QDialog>
#endif

namespace Ui {
    class MessagePage;
}
class WalletModel;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MessagePage : public QDialog
{
    Q_OBJECT

public:
    explicit MessagePage(QWidget *parent = 0);
    ~MessagePage();

    void setModel(WalletModel *model);

    void setAddress(QString);

private:
    Ui::MessagePage *ui;
    WalletModel *model;

private slots:
    void on_pasteButton_clicked();
    void on_addressBookButton_clicked();

    void on_signMessage_clicked();
    void on_verifyMessage_clicked();
    void on_copyToClipboard_clicked();
};

#endif // MESSAGEPAGE_H
