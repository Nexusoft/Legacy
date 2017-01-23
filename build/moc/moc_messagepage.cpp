/****************************************************************************
** Meta object code from reading C++ file 'messagepage.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qt/pages/messagepage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'messagepage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MessagePage[] = {

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
      13,   12,   12,   12, 0x08,
      38,   12,   12,   12, 0x08,
      69,   12,   12,   12, 0x08,
      94,   12,   12,   12, 0x08,
     121,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MessagePage[] = {
    "MessagePage\0\0on_pasteButton_clicked()\0"
    "on_addressBookButton_clicked()\0"
    "on_signMessage_clicked()\0"
    "on_verifyMessage_clicked()\0"
    "on_copyToClipboard_clicked()\0"
};

void MessagePage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MessagePage *_t = static_cast<MessagePage *>(_o);
        switch (_id) {
        case 0: _t->on_pasteButton_clicked(); break;
        case 1: _t->on_addressBookButton_clicked(); break;
        case 2: _t->on_signMessage_clicked(); break;
        case 3: _t->on_verifyMessage_clicked(); break;
        case 4: _t->on_copyToClipboard_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MessagePage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MessagePage::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_MessagePage,
      qt_meta_data_MessagePage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MessagePage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MessagePage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MessagePage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MessagePage))
        return static_cast<void*>(const_cast< MessagePage*>(this));
    return QDialog::qt_metacast(_clname);
}

int MessagePage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
