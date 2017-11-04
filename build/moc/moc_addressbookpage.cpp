/****************************************************************************
** Meta object code from reading C++ file 'addressbookpage.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qt/pages/addressbookpage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'addressbookpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AddressBookPage[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   26,   33,   33, 0x0a,
      34,   33,   33,   33, 0x0a,
      50,   33,   33,   33, 0x08,
      76,   33,   33,   33, 0x08,
     106,   33,   33,   33, 0x08,
     135,   33,   33,   33, 0x08,
     160,   33,   33,   33, 0x08,
     179,   33,   33,   33, 0x08,
     203,  226,   33,   33, 0x08,
     232,   33,   33,   33, 0x08,
     252,   33,   33,   33, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AddressBookPage[] = {
    "AddressBookPage\0done(int)\0retval\0\0"
    "exportClicked()\0on_deleteButton_clicked()\0"
    "on_newAddressButton_clicked()\0"
    "on_copyToClipboard_clicked()\0"
    "on_signMessage_clicked()\0selectionChanged()\0"
    "on_showQRCode_clicked()\0contextualMenu(QPoint)\0"
    "point\0onCopyLabelAction()\0onEditAction()\0"
};

void AddressBookPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AddressBookPage *_t = static_cast<AddressBookPage *>(_o);
        switch (_id) {
        case 0: _t->done((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->exportClicked(); break;
        case 2: _t->on_deleteButton_clicked(); break;
        case 3: _t->on_newAddressButton_clicked(); break;
        case 4: _t->on_copyToClipboard_clicked(); break;
        case 5: _t->on_signMessage_clicked(); break;
        case 6: _t->selectionChanged(); break;
        case 7: _t->on_showQRCode_clicked(); break;
        case 8: _t->contextualMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 9: _t->onCopyLabelAction(); break;
        case 10: _t->onEditAction(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AddressBookPage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AddressBookPage::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_AddressBookPage,
      qt_meta_data_AddressBookPage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AddressBookPage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AddressBookPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AddressBookPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AddressBookPage))
        return static_cast<void*>(const_cast< AddressBookPage*>(this));
    return QDialog::qt_metacast(_clname);
}

int AddressBookPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
