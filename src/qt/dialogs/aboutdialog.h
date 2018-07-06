/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QDialog>
#else
 #include <QtWidgets/QDialog>
#endif

namespace Ui { class AboutDialog; }
class ClientModel;

/** "About" dialog box */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

    void setModel(ClientModel *model);
private:
    Ui::AboutDialog *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // ABOUTDIALOG_H
