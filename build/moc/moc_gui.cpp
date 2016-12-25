/****************************************************************************
** Meta object code from reading C++ file 'gui.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qt/core/gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NexusGUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   10,    9,    9, 0x0a,
      39,   10,    9,    9, 0x0a,
      64,   57,    9,    9, 0x0a,
     126,   89,    9,    9, 0x0a,
     178,  158,    9,    9, 0x0a,
     226,  206,    9,    9, 0x0a,
     254,  247,    9,    9, 0x0a,
     273,    9,    9,    9, 0x0a,
     291,    9,    9,    9, 0x0a,
     316,    9,    9,    9, 0x08,
     335,    9,    9,    9, 0x08,
     353,    9,    9,    9, 0x08,
     375,    9,    9,    9, 0x08,
     398,    9,    9,    9, 0x08,
     418,    9,    9,    9, 0x08,
     435,    9,    9,    9, 0x08,
     457,  450,    9,    9, 0x08,
     527,  510,    9,    9, 0x08,
     568,   57,    9,    9, 0x08,
     588,    9,    9,    9, 0x08,
     603,    9,    9,    9, 0x08,
     622,    9,    9,    9, 0x08,
     641,    9,    9,    9, 0x08,
     656,    9,    9,    9, 0x08,
     669,    9,    9,    9, 0x08,
     693,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NexusGUI[] = {
    "NexusGUI\0\0count\0setNumConnections(int)\0"
    "setNumBlocks(int)\0status\0"
    "setEncryptionStatus(int)\0"
    "trustWeight,blockWeight,interestRate\0"
    "setWeight(double,double,double)\0"
    "title,message,modal\0error(QString,QString,bool)\0"
    "nFeeRequired,payFee\0askFee(qint64,bool*)\0"
    "strURI\0handleURI(QString)\0gotoMessagePage()\0"
    "gotoMessagePage(QString)\0gotoOverviewPage()\0"
    "gotoHistoryPage()\0gotoAddressBookPage()\0"
    "gotoReceiveCoinsPage()\0gotoSendCoinsPage()\0"
    "optionsClicked()\0aboutClicked()\0reason\0"
    "trayIconActivated(QSystemTrayIcon::ActivationReason)\0"
    "parent,start,end\0"
    "incomingTransaction(QModelIndex,int,int)\0"
    "encryptWallet(bool)\0backupWallet()\0"
    "changePassphrase()\0tempUnlockWallet()\0"
    "unlockWallet()\0lockWallet()\0"
    "showNormalIfMinimized()\0toggleHidden()\0"
};

void NexusGUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NexusGUI *_t = static_cast<NexusGUI *>(_o);
        switch (_id) {
        case 0: _t->setNumConnections((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setNumBlocks((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setEncryptionStatus((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setWeight((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 4: _t->error((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 5: _t->askFee((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< bool*(*)>(_a[2]))); break;
        case 6: _t->handleURI((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->gotoMessagePage(); break;
        case 8: _t->gotoMessagePage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->gotoOverviewPage(); break;
        case 10: _t->gotoHistoryPage(); break;
        case 11: _t->gotoAddressBookPage(); break;
        case 12: _t->gotoReceiveCoinsPage(); break;
        case 13: _t->gotoSendCoinsPage(); break;
        case 14: _t->optionsClicked(); break;
        case 15: _t->aboutClicked(); break;
        case 16: _t->trayIconActivated((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        case 17: _t->incomingTransaction((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 18: _t->encryptWallet((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: _t->backupWallet(); break;
        case 20: _t->changePassphrase(); break;
        case 21: _t->tempUnlockWallet(); break;
        case 22: _t->unlockWallet(); break;
        case 23: _t->lockWallet(); break;
        case 24: _t->showNormalIfMinimized(); break;
        case 25: _t->toggleHidden(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NexusGUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NexusGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_NexusGUI,
      qt_meta_data_NexusGUI, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NexusGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NexusGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NexusGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NexusGUI))
        return static_cast<void*>(const_cast< NexusGUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int NexusGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
