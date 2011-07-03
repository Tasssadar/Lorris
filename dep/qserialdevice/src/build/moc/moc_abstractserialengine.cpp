/****************************************************************************
** Meta object code from reading C++ file 'abstractserialengine.h'
**
** Created: Sat May 14 15:06:22 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qserialdevice/abstractserialengine.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'abstractserialengine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AbstractSerialEngine[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x0a,
      41,   21,   21,   21, 0x0a,
      61,   21,   21,   21, 0x0a,
      85,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AbstractSerialEngine[] = {
    "AbstractSerialEngine\0\0readNotification()\0"
    "writeNotification()\0exceptionNotification()\0"
    "lineNotification()\0"
};

const QMetaObject AbstractSerialEngine::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AbstractSerialEngine,
      qt_meta_data_AbstractSerialEngine, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AbstractSerialEngine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AbstractSerialEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AbstractSerialEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AbstractSerialEngine))
        return static_cast<void*>(const_cast< AbstractSerialEngine*>(this));
    return QObject::qt_metacast(_clname);
}

int AbstractSerialEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readNotification(); break;
        case 1: writeNotification(); break;
        case 2: exceptionNotification(); break;
        case 3: lineNotification(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
