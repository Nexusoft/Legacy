/********************************************************************************
** Form generated from reading UI file 'messagepage.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MESSAGEPAGE_H
#define UI_MESSAGEPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "util/qvalidatedlineedit.h"

QT_BEGIN_NAMESPACE

class Ui_MessagePage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelExplanation;
    QHBoxLayout *horizontalLayout_3;
    QValidatedLineEdit *signFrom;
    QPushButton *addressBookButton;
    QPushButton *pasteButton;
    QTextEdit *message;
    QTextEdit *signature;
    QHBoxLayout *horizontalLayout;
    QPushButton *signMessage;
    QPushButton *verifyMessage;
    QPushButton *copyToClipboard;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *MessagePage)
    {
        if (MessagePage->objectName().isEmpty())
            MessagePage->setObjectName(QString::fromUtf8("MessagePage"));
        MessagePage->resize(627, 380);
        verticalLayout = new QVBoxLayout(MessagePage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        labelExplanation = new QLabel(MessagePage);
        labelExplanation->setObjectName(QString::fromUtf8("labelExplanation"));
        labelExplanation->setTextFormat(Qt::AutoText);
        labelExplanation->setWordWrap(true);

        verticalLayout->addWidget(labelExplanation);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        signFrom = new QValidatedLineEdit(MessagePage);
        signFrom->setObjectName(QString::fromUtf8("signFrom"));
        signFrom->setMaxLength(54);

        horizontalLayout_3->addWidget(signFrom);

        addressBookButton = new QPushButton(MessagePage);
        addressBookButton->setObjectName(QString::fromUtf8("addressBookButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/address-book"), QSize(), QIcon::Normal, QIcon::Off);
        addressBookButton->setIcon(icon);
        addressBookButton->setAutoDefault(false);
        addressBookButton->setFlat(false);

        horizontalLayout_3->addWidget(addressBookButton);

        pasteButton = new QPushButton(MessagePage);
        pasteButton->setObjectName(QString::fromUtf8("pasteButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/editpaste"), QSize(), QIcon::Normal, QIcon::Off);
        pasteButton->setIcon(icon1);
        pasteButton->setAutoDefault(false);

        horizontalLayout_3->addWidget(pasteButton);


        verticalLayout->addLayout(horizontalLayout_3);

        message = new QTextEdit(MessagePage);
        message->setObjectName(QString::fromUtf8("message"));
        message->setProperty("sizeHint", QVariant(QSize(0, 240)));

        verticalLayout->addWidget(message);

        signature = new QTextEdit(MessagePage);
        signature->setObjectName(QString::fromUtf8("signature"));
        QFont font;
        font.setItalic(true);
        signature->setFont(font);
        signature->setProperty("sizeHint", QVariant(QSize(0, 80)));
        signature->setReadOnly(false);

        verticalLayout->addWidget(signature);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        signMessage = new QPushButton(MessagePage);
        signMessage->setObjectName(QString::fromUtf8("signMessage"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/edit"), QSize(), QIcon::Normal, QIcon::Off);
        signMessage->setIcon(icon2);

        horizontalLayout->addWidget(signMessage);

        verifyMessage = new QPushButton(MessagePage);
        verifyMessage->setObjectName(QString::fromUtf8("verifyMessage"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/synced"), QSize(), QIcon::Normal, QIcon::Off);
        verifyMessage->setIcon(icon3);

        horizontalLayout->addWidget(verifyMessage);

        copyToClipboard = new QPushButton(MessagePage);
        copyToClipboard->setObjectName(QString::fromUtf8("copyToClipboard"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/editcopy"), QSize(), QIcon::Normal, QIcon::Off);
        copyToClipboard->setIcon(icon4);

        horizontalLayout->addWidget(copyToClipboard);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(MessagePage);

        QMetaObject::connectSlotsByName(MessagePage);
    } // setupUi

    void retranslateUi(QWidget *MessagePage)
    {
        MessagePage->setWindowTitle(QApplication::translate("MessagePage", "Message", 0, QApplication::UnicodeUTF8));
        labelExplanation->setText(QApplication::translate("MessagePage", "You can sign messages with your addresses to prove you own them. Be careful not to sign anything vague, as phishing attacks may try to trick you into signing your identity over to them. Only sign fully-detailed statements you agree to.", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        signFrom->setToolTip(QApplication::translate("MessagePage", "The address to sign the message with  (e.g. 1NS17iag9jJgTHD1VXjvLCEnZuQ3rJDE9L)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        addressBookButton->setToolTip(QApplication::translate("MessagePage", "Choose address from address book", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        addressBookButton->setText(QString());
        addressBookButton->setShortcut(QApplication::translate("MessagePage", "Alt+A", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pasteButton->setToolTip(QApplication::translate("MessagePage", "Paste address from clipboard", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pasteButton->setText(QString());
        pasteButton->setShortcut(QApplication::translate("MessagePage", "Alt+P", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message->setToolTip(QApplication::translate("MessagePage", "Enter the message you want to sign here", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        signature->setText(QApplication::translate("MessagePage", "Click \"Sign Message\" to get signature or Paste a signature here and click \"Verify Message\"", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        signMessage->setToolTip(QApplication::translate("MessagePage", "Sign a message to prove you own this address", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        signMessage->setText(QApplication::translate("MessagePage", "&Sign Message", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        verifyMessage->setToolTip(QApplication::translate("MessagePage", "Generate an Address from Signature and Message.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        verifyMessage->setText(QApplication::translate("MessagePage", "&Verify Message", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        copyToClipboard->setToolTip(QApplication::translate("MessagePage", "Copy the current signature to the system clipboard", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        copyToClipboard->setText(QApplication::translate("MessagePage", "&Copy to Clipboard", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MessagePage: public Ui_MessagePage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MESSAGEPAGE_H
