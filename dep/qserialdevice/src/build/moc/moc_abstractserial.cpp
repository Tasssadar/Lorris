/****************************************************************************
** Meta object code from reading C++ file 'abstractserial.h'
**
** Created: Sat May 14 15:06:19 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qserialdevice/abstractserial.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'abstractserial.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AbstractSerial[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   16,   15,   15, 0x05,
      63,   15,   15,   15, 0x05,
      81,   75,   15,   15, 0x05,
      98,   75,   15,   15, 0x05,
     115,   75,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_AbstractSerial[] = {
    "AbstractSerial\0\0status,current\0"
    "signalStatus(QString,QDateTime)\0"
    "exception()\0value\0ctsChanged(bool)\0"
    "dsrChanged(bool)\0ringChanged(bool)\0"
};

const QMetaObject AbstractSerial::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_AbstractSerial,
      qt_meta_data_AbstractSerial, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AbstractSerial::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AbstractSerial::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AbstractSerial::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AbstractSerial))
        return static_cast<void*>(const_cast< AbstractSerial*>(this));
    return QIODevice::qt_metacast(_clname);
}

int AbstractSerial::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: signalStatus((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QDateTime(*)>(_a[2]))); break;
        case 1: exception(); break;
        case 2: ctsChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: dsrChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: ringChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void AbstractSerial::signalStatus(const QString & _t1, QDateTime _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AbstractSerial::exception()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void AbstractSerial::ctsChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AbstractSerial::dsrChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AbstractSerial::ringChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
