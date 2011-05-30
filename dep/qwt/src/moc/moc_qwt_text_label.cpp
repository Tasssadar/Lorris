/****************************************************************************
** Meta object code from reading C++ file 'qwt_text_label.h'
**
** Created: Mon May 30 19:22:03 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_text_label.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_text_label.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtTextLabel[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       2,   34, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   14,   13,   13, 0x0a,
      63,   13,   13,   13, 0x2a,
      80,   13,   13,   13, 0x0a,
      97,   13,   13,   13, 0x0a,

 // properties: name, type, flags
     109,  105, 0x02095103,
     116,  105, 0x02095103,

       0        // eod
};

static const char qt_meta_stringdata_QwtTextLabel[] = {
    "QwtTextLabel\0\0,textFormat\0"
    "setText(QString,QwtText::TextFormat)\0"
    "setText(QString)\0setText(QwtText)\0"
    "clear()\0int\0indent\0margin\0"
};

const QMetaObject QwtTextLabel::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_QwtTextLabel,
      qt_meta_data_QwtTextLabel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtTextLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtTextLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtTextLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtTextLabel))
        return static_cast<void*>(const_cast< QwtTextLabel*>(this));
    return QFrame::qt_metacast(_clname);
}

int QwtTextLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setText((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QwtText::TextFormat(*)>(_a[2]))); break;
        case 1: setText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: setText((*reinterpret_cast< const QwtText(*)>(_a[1]))); break;
        case 3: clear(); break;
        default: ;
        }
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = indent(); break;
        case 1: *reinterpret_cast< int*>(_v) = margin(); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setIndent(*reinterpret_cast< int*>(_v)); break;
        case 1: setMargin(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
