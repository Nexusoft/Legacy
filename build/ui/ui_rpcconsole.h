/********************************************************************************
** Form generated from reading UI file 'rpcconsole.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RPCCONSOLE_H
#define UI_RPCCONSOLE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RPCConsole
{
public:
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab_info;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLabel *clientName;
    QLabel *label_6;
    QLabel *clientVersion;
    QLabel *label_9;
    QLabel *label_11;
    QLabel *label_7;
    QLabel *numberOfConnections;
    QLabel *label_8;
    QCheckBox *isTestNet;
    QLabel *label_10;
    QLabel *label_3;
    QLabel *numberOfBlocks;
    QLabel *label_4;
    QLabel *totalBlocks;
    QLabel *label_2;
    QLabel *lastBlockTime;
    QLabel *label_n11;
    QLabel *primeHeight;
    QLabel *label_n12;
    QLabel *primeDifficulty;
    QLabel *label_n13;
    QLabel *primeReserves;
    QLabel *label_n14;
    QLabel *hashingHeight;
    QLabel *label_n15;
    QLabel *hashingDifficulty;
    QLabel *label_n16;
    QLabel *hashingReserves;
    QLabel *label_n17;
    QLabel *label_n171;
    QLabel *label_n18;
    QLabel *coinSupply;
    QSpacerItem *verticalSpacer;
    QLabel *label_12;
    QLabel *buildDate;
    QWidget *tab_console;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *messagesWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *clearButton;

    void setupUi(QDialog *RPCConsole)
    {
        if (RPCConsole->objectName().isEmpty())
            RPCConsole->setObjectName(QString::fromUtf8("RPCConsole"));
        RPCConsole->resize(706, 446);
        verticalLayout_2 = new QVBoxLayout(RPCConsole);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tabWidget = new QTabWidget(RPCConsole);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab_info = new QWidget();
        tab_info->setObjectName(QString::fromUtf8("tab_info"));
        gridLayout = new QGridLayout(tab_info);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(12);
        label_5 = new QLabel(tab_info);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 1, 0, 1, 1);

        clientName = new QLabel(tab_info);
        clientName->setObjectName(QString::fromUtf8("clientName"));
        clientName->setTextFormat(Qt::PlainText);
        clientName->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(clientName, 1, 1, 1, 1);

        label_6 = new QLabel(tab_info);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 2, 0, 1, 1);

        clientVersion = new QLabel(tab_info);
        clientVersion->setObjectName(QString::fromUtf8("clientVersion"));
        clientVersion->setTextFormat(Qt::PlainText);
        clientVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(clientVersion, 2, 1, 1, 1);

        label_9 = new QLabel(tab_info);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_9->setFont(font);

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        label_11 = new QLabel(tab_info);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setFont(font);

        gridLayout->addWidget(label_11, 4, 0, 1, 1);

        label_7 = new QLabel(tab_info);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 5, 0, 1, 1);

        numberOfConnections = new QLabel(tab_info);
        numberOfConnections->setObjectName(QString::fromUtf8("numberOfConnections"));
        numberOfConnections->setTextFormat(Qt::PlainText);
        numberOfConnections->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(numberOfConnections, 5, 1, 1, 1);

        label_8 = new QLabel(tab_info);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 6, 0, 1, 1);

        isTestNet = new QCheckBox(tab_info);
        isTestNet->setObjectName(QString::fromUtf8("isTestNet"));
        isTestNet->setEnabled(false);

        gridLayout->addWidget(isTestNet, 6, 1, 1, 1);

        label_10 = new QLabel(tab_info);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font);

        gridLayout->addWidget(label_10, 7, 0, 1, 1);

        label_3 = new QLabel(tab_info);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 8, 0, 1, 1);

        numberOfBlocks = new QLabel(tab_info);
        numberOfBlocks->setObjectName(QString::fromUtf8("numberOfBlocks"));
        numberOfBlocks->setTextFormat(Qt::PlainText);
        numberOfBlocks->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(numberOfBlocks, 8, 1, 1, 1);

        label_4 = new QLabel(tab_info);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 9, 0, 1, 1);

        totalBlocks = new QLabel(tab_info);
        totalBlocks->setObjectName(QString::fromUtf8("totalBlocks"));
        totalBlocks->setTextFormat(Qt::PlainText);
        totalBlocks->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(totalBlocks, 9, 1, 1, 1);

        label_2 = new QLabel(tab_info);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 10, 0, 1, 1);

        lastBlockTime = new QLabel(tab_info);
        lastBlockTime->setObjectName(QString::fromUtf8("lastBlockTime"));
        lastBlockTime->setTextFormat(Qt::PlainText);
        lastBlockTime->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(lastBlockTime, 10, 1, 1, 1);

        label_n11 = new QLabel(tab_info);
        label_n11->setObjectName(QString::fromUtf8("label_n11"));
        label_n11->setFont(font);

        gridLayout->addWidget(label_n11, 11, 0, 1, 1);

        primeHeight = new QLabel(tab_info);
        primeHeight->setObjectName(QString::fromUtf8("primeHeight"));
        QFont font1;
        font1.setItalic(true);
        font1.setWeight(75);
        primeHeight->setFont(font1);

        gridLayout->addWidget(primeHeight, 11, 1, 1, 1);

        label_n12 = new QLabel(tab_info);
        label_n12->setObjectName(QString::fromUtf8("label_n12"));

        gridLayout->addWidget(label_n12, 12, 0, 1, 1);

        primeDifficulty = new QLabel(tab_info);
        primeDifficulty->setObjectName(QString::fromUtf8("primeDifficulty"));
        primeDifficulty->setTextFormat(Qt::PlainText);
        primeDifficulty->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(primeDifficulty, 12, 1, 1, 1);

        label_n13 = new QLabel(tab_info);
        label_n13->setObjectName(QString::fromUtf8("label_n13"));

        gridLayout->addWidget(label_n13, 13, 0, 1, 1);

        primeReserves = new QLabel(tab_info);
        primeReserves->setObjectName(QString::fromUtf8("primeReserves"));
        primeReserves->setTextFormat(Qt::PlainText);
        primeReserves->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(primeReserves, 13, 1, 1, 1);

        label_n14 = new QLabel(tab_info);
        label_n14->setObjectName(QString::fromUtf8("label_n14"));
        label_n14->setFont(font);

        gridLayout->addWidget(label_n14, 14, 0, 1, 1);

        hashingHeight = new QLabel(tab_info);
        hashingHeight->setObjectName(QString::fromUtf8("hashingHeight"));
        hashingHeight->setFont(font1);

        gridLayout->addWidget(hashingHeight, 14, 1, 1, 1);

        label_n15 = new QLabel(tab_info);
        label_n15->setObjectName(QString::fromUtf8("label_n15"));

        gridLayout->addWidget(label_n15, 15, 0, 1, 1);

        hashingDifficulty = new QLabel(tab_info);
        hashingDifficulty->setObjectName(QString::fromUtf8("hashingDifficulty"));
        hashingDifficulty->setTextFormat(Qt::PlainText);
        hashingDifficulty->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(hashingDifficulty, 15, 1, 1, 1);

        label_n16 = new QLabel(tab_info);
        label_n16->setObjectName(QString::fromUtf8("label_n16"));

        gridLayout->addWidget(label_n16, 16, 0, 1, 1);

        hashingReserves = new QLabel(tab_info);
        hashingReserves->setObjectName(QString::fromUtf8("hashingReserves"));
        hashingReserves->setTextFormat(Qt::PlainText);
        hashingReserves->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(hashingReserves, 16, 1, 1, 1);

        label_n17 = new QLabel(tab_info);
        label_n17->setObjectName(QString::fromUtf8("label_n17"));
        label_n17->setFont(font);

        gridLayout->addWidget(label_n17, 17, 0, 1, 1);

        label_n171 = new QLabel(tab_info);
        label_n171->setObjectName(QString::fromUtf8("label_n171"));
        QFont font2;
        font2.setItalic(true);
        label_n171->setFont(font2);

        gridLayout->addWidget(label_n171, 17, 1, 1, 1);

        label_n18 = new QLabel(tab_info);
        label_n18->setObjectName(QString::fromUtf8("label_n18"));

        gridLayout->addWidget(label_n18, 18, 0, 1, 1);

        coinSupply = new QLabel(tab_info);
        coinSupply->setObjectName(QString::fromUtf8("coinSupply"));
        coinSupply->setTextFormat(Qt::PlainText);
        coinSupply->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(coinSupply, 18, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 19, 0, 1, 1);

        label_12 = new QLabel(tab_info);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout->addWidget(label_12, 3, 0, 1, 1);

        buildDate = new QLabel(tab_info);
        buildDate->setObjectName(QString::fromUtf8("buildDate"));

        gridLayout->addWidget(buildDate, 3, 1, 1, 1);

        gridLayout->setColumnStretch(1, 1);
        tabWidget->addTab(tab_info, QString());
        tab_console = new QWidget();
        tab_console->setObjectName(QString::fromUtf8("tab_console"));
        verticalLayout_3 = new QVBoxLayout(tab_console);
        verticalLayout_3->setSpacing(3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        messagesWidget = new QTextEdit(tab_console);
        messagesWidget->setObjectName(QString::fromUtf8("messagesWidget"));
        messagesWidget->setMinimumSize(QSize(0, 100));
        messagesWidget->setReadOnly(true);
        messagesWidget->setProperty("tabKeyNavigation", QVariant(false));
        messagesWidget->setProperty("columnCount", QVariant(2));

        verticalLayout_3->addWidget(messagesWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(3);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(tab_console);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(tab_console);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        clearButton = new QPushButton(tab_console);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));
        clearButton->setMaximumSize(QSize(24, 24));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/remove"), QSize(), QIcon::Normal, QIcon::Off);
        clearButton->setIcon(icon);
        clearButton->setShortcut(QString::fromUtf8("Ctrl+L"));
        clearButton->setAutoDefault(false);

        horizontalLayout->addWidget(clearButton);


        verticalLayout_3->addLayout(horizontalLayout);

        tabWidget->addTab(tab_console, QString());

        verticalLayout_2->addWidget(tabWidget);


        retranslateUi(RPCConsole);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(RPCConsole);
    } // setupUi

    void retranslateUi(QDialog *RPCConsole)
    {
        RPCConsole->setWindowTitle(QApplication::translate("RPCConsole", "Nexus Debug Window", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("RPCConsole", "Client name", 0, QApplication::UnicodeUTF8));
        clientName->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("RPCConsole", "Client version", 0, QApplication::UnicodeUTF8));
        clientVersion->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("RPCConsole", "Version", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("RPCConsole", "Network", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("RPCConsole", "Number of connections", 0, QApplication::UnicodeUTF8));
        numberOfConnections->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("RPCConsole", "On testnet", 0, QApplication::UnicodeUTF8));
        isTestNet->setText(QString());
        label_10->setText(QApplication::translate("RPCConsole", "Block chain", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("RPCConsole", "Current number of blocks", 0, QApplication::UnicodeUTF8));
        numberOfBlocks->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("RPCConsole", "Estimated total blocks", 0, QApplication::UnicodeUTF8));
        totalBlocks->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("RPCConsole", "Last block time", 0, QApplication::UnicodeUTF8));
        lastBlockTime->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_n11->setText(QApplication::translate("RPCConsole", "Prime Channel", 0, QApplication::UnicodeUTF8));
        primeHeight->setText(QApplication::translate("RPCConsole", "Height [N/A]", 0, QApplication::UnicodeUTF8));
        label_n12->setText(QApplication::translate("RPCConsole", "Difficulty", 0, QApplication::UnicodeUTF8));
        primeDifficulty->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_n13->setText(QApplication::translate("RPCConsole", "Reserves", 0, QApplication::UnicodeUTF8));
        primeReserves->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_n14->setText(QApplication::translate("RPCConsole", "Hashing Channel", 0, QApplication::UnicodeUTF8));
        hashingHeight->setText(QApplication::translate("RPCConsole", "Height [N/A]", 0, QApplication::UnicodeUTF8));
        label_n15->setText(QApplication::translate("RPCConsole", "Difficulty", 0, QApplication::UnicodeUTF8));
        hashingDifficulty->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_n16->setText(QApplication::translate("RPCConsole", "Reserves", 0, QApplication::UnicodeUTF8));
        hashingReserves->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        label_n17->setText(QApplication::translate("RPCConsole", "Distribution", 0, QApplication::UnicodeUTF8));
        label_n171->setText(QApplication::translate("RPCConsole", "Total Nexus in Circulation", 0, QApplication::UnicodeUTF8));
        label_n18->setText(QApplication::translate("RPCConsole", "Total to Date", 0, QApplication::UnicodeUTF8));
        coinSupply->setText(QApplication::translate("RPCConsole", "N/A [N/A %]", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("RPCConsole", "Build date", 0, QApplication::UnicodeUTF8));
        buildDate->setText(QApplication::translate("RPCConsole", "N/A", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_info), QApplication::translate("RPCConsole", "Information", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("RPCConsole", ">", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        clearButton->setToolTip(QApplication::translate("RPCConsole", "Clear console", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        clearButton->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_console), QApplication::translate("RPCConsole", "Console", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RPCConsole: public Ui_RPCConsole {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RPCCONSOLE_H
