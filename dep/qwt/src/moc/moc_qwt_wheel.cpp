/****************************************************************************
** Meta object code from reading C++ file 'qwt_wheel.h'
**
** Created: Mon May 30 19:22:27 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_wheel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_wheel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtWheel[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       7,   24, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,
      32,    9,    9,    9, 0x0a,

 // properties: name, type, flags
      60,   53, 0x06095103,
      71,   53, 0x06095103,
      85,   81, 0x02095103,
      93,   81, 0x02095103,
     104,   81, 0x02095103,
     116,   81, 0x02095103,
     133,   53, 0x06095103,

       0        // eod
};

static const char qt_meta_stringdata_QwtWheel[] = {
    "QwtWheel\0\0setTotalAngle(double)\0"
    "setViewAngle(double)\0double\0totalAngle\0"
    "viewAngle\0int\0tickCnt\0wheelWidth\0"
    "borderWidth\0wheelBorderWidth\0mass\0"
};

const QMetaObject QwtWheel::staticMetaObject = {
    { &QwtAbstractSlider::staticMetaObject, qt_meta_stringdata_QwtWheel,
      qt_meta_data_QwtWheel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtWheel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtWheel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtWheel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtWheel))
        return static_cast<void*>(const_cast< QwtWheel*>(this));
    return QwtAbstractSlider::qt_metacast(_clname);
}

int QwtWheel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtAbstractSlider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setTotalAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: setViewAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< double*>(_v) = totalAngle(); break;
        case 1: *reinterpret_cast< double*>(_v) = viewAngle(); break;
        case 2: *reinterpret_cast< int*>(_v) = tickCnt(); break;
        case 3: *reinterpret_cast< int*>(_v) = wheelWidth(); break;
        case 4: *reinterpret_cast< int*>(_v) = borderWidth(); break;
        case 5: *reinterpret_cast< int*>(_v) = wheelBorderWidth(); break;
        case 6: *reinterpret_cast< double*>(_v) = mass(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setTotalAngle(*reinterpret_cast< double*>(_v)); break;
        case 1: setViewAngle(*reinterpret_cast< double*>(_v)); break;
        case 2: setTickCnt(*reinterpret_cast< int*>(_v)); break;
        case 3: setWheelWidth(*reinterpret_cast< int*>(_v)); break;
        case 4: setBorderWidth(*reinterpret_cast< int*>(_v)); break;
        case 5: setWheelBorderWidth(*reinterpret_cast< int*>(_v)); break;
        case 6: setMass(*reinterpret_cast< double*>(_v)); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
