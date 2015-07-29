/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "transactiondescdialog.h"
#include "ui_transactiondescdialog.h"

#include "../models/transactiontablemodel.h"

#include <QModelIndex>

TransactionDescDialog::TransactionDescDialog(const QModelIndex &idx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionDescDialog)
{
    ui->setupUi(this);
    QString desc = idx.data(TransactionTableModel::LongDescriptionRole).toString();
    ui->detailText->setHtml(desc);
	
	setStyleSheet("selection-background-color: #084B8A; background-color: #F7F7F7");
}

TransactionDescDialog::~TransactionDescDialog()
{
    delete ui;
}
