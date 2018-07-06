/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef EDITADDRESSDIALOG_H
#define EDITADDRESSDIALOG_H
#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QDialog>
#else
 #include <QtWidgets/QDialog>
#endif

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class EditAddressDialog;
}
class AddressTableModel;

/** Dialog for editing an address and associated information.
 */
class EditAddressDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        NewReceivingAddress,
        NewSendingAddress,
        EditReceivingAddress,
        EditSendingAddress
    };

    explicit EditAddressDialog(Mode mode, QWidget *parent = 0);
    ~EditAddressDialog();

    void setModel(AddressTableModel *model);
    void loadRow(int row);

    void accept();

    QString getAddress() const;
    void setAddress(const QString &address);
private:
    bool saveCurrentRow();

    Ui::EditAddressDialog *ui;
    QDataWidgetMapper *mapper;
    Mode mode;
    AddressTableModel *model;

    QString address;
};

#endif // EDITADDRESSDIALOG_H
