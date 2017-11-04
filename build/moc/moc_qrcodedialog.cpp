/****************************************************************************
** Meta object code from reading C++ file 'qrcodedialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qt/dialogs/qrcodedialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qrcodedialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QRCodeDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   49,   54,   54, 0x08,
      55,   49,   54,   54, 0x08,
      87,   49,   54,   54, 0x08,
     121,   54,   54,   54, 0x08,
     144,  175,   54,   54, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QRCodeDialog[] = {
    "QRCodeDialog\0on_lnReqAmount_textChanged(QString)\0"
    "arg1\0\0on_lnLabel_textChanged(QString)\0"
    "on_lnMessage_textChanged(QString)\0"
    "on_btnSaveAs_clicked()\0"
    "on_chkReqPayment_toggled(bool)\0checked\0"
};

void QRCodeDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QRCodeDialog *_t = static_cast<QRCodeDialog *>(_o);
        switch (_id) {
        case 0: _t->on_lnReqAmount_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->on_lnLabel_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->on_lnMessage_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_btnSaveAs_clicked(); break;
        case 4: _t->on_chkReqPayment_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QRCodeDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QRCodeDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_QRCodeDialog,
      qt_meta_data_QRCodeDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QRCodeDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QRCodeDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QRCodeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QRCodeDialog))
        return static_cast<void*>(const_cast< QRCodeDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int QRCodeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
