/****************************************************************************
** Meta object code from reading C++ file 'qwt_analog_clock.h'
**
** Created: Mon May 30 19:22:18 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_analog_clock.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_analog_clock.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtAnalogClock[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      33,   15,   15,   15, 0x0a,
      48,   15,   15,   15, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_QwtAnalogClock[] = {
    "QwtAnalogClock\0\0setCurrentTime()\0"
    "setTime(QTime)\0setTime()\0"
};

const QMetaObject QwtAnalogClock::staticMetaObject = {
    { &QwtDial::staticMetaObject, qt_meta_stringdata_QwtAnalogClock,
      qt_meta_data_QwtAnalogClock, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtAnalogClock::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtAnalogClock::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtAnalogClock::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtAnalogClock))
        return static_cast<void*>(const_cast< QwtAnalogClock*>(this));
    return QwtDial::qt_metacast(_clname);
}

int QwtAnalogClock::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtDial::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setCurrentTime(); break;
        case 1: setTime((*reinterpret_cast< const QTime(*)>(_a[1]))); break;
        case 2: setTime(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
