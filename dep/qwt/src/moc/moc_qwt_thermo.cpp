/****************************************************************************
** Meta object code from reading C++ file 'qwt_thermo.h'
**
** Created: Mon May 30 19:22:26 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_thermo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_thermo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtThermo[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       9,   19, // properties
       1,   46, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   11,   10,   10, 0x0a,

 // properties: name, type, flags
      37,   32, 0x01095103,
      57,   50, 0x06095103,
      77,   68, 0x0009510b,
      95,   91, 0x02095103,
     103,   91, 0x02095103,
     115,   50, 0x06095103,
     124,   50, 0x06095103,
     133,   91, 0x02095103,
     143,   50, 0x06095103,

 // enums: name, flags, count, data
      68, 0x0,    5,   50,

 // enum data: key, value
     149, uint(QwtThermo::NoScale),
     157, uint(QwtThermo::LeftScale),
     167, uint(QwtThermo::RightScale),
     178, uint(QwtThermo::TopScale),
     187, uint(QwtThermo::BottomScale),

       0        // eod
};

static const char qt_meta_stringdata_QwtThermo[] = {
    "QwtThermo\0\0val\0setValue(double)\0bool\0"
    "alarmEnabled\0double\0alarmLevel\0ScalePos\0"
    "scalePosition\0int\0spacing\0borderWidth\0"
    "maxValue\0minValue\0pipeWidth\0value\0"
    "NoScale\0LeftScale\0RightScale\0TopScale\0"
    "BottomScale\0"
};

const QMetaObject QwtThermo::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QwtThermo,
      qt_meta_data_QwtThermo, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtThermo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtThermo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtThermo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtThermo))
        return static_cast<void*>(const_cast< QwtThermo*>(this));
    if (!strcmp(_clname, "QwtAbstractScale"))
        return static_cast< QwtAbstractScale*>(const_cast< QwtThermo*>(this));
    return QWidget::qt_metacast(_clname);
}

int QwtThermo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = alarmEnabled(); break;
        case 1: *reinterpret_cast< double*>(_v) = alarmLevel(); break;
        case 2: *reinterpret_cast< ScalePos*>(_v) = scalePosition(); break;
        case 3: *reinterpret_cast< int*>(_v) = spacing(); break;
        case 4: *reinterpret_cast< int*>(_v) = borderWidth(); break;
        case 5: *reinterpret_cast< double*>(_v) = maxValue(); break;
        case 6: *reinterpret_cast< double*>(_v) = minValue(); break;
        case 7: *reinterpret_cast< int*>(_v) = pipeWidth(); break;
        case 8: *reinterpret_cast< double*>(_v) = value(); break;
        }
        _id -= 9;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setAlarmEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 1: setAlarmLevel(*reinterpret_cast< double*>(_v)); break;
        case 2: setScalePosition(*reinterpret_cast< ScalePos*>(_v)); break;
        case 3: setSpacing(*reinterpret_cast< int*>(_v)); break;
        case 4: setBorderWidth(*reinterpret_cast< int*>(_v)); break;
        case 5: setMaxValue(*reinterpret_cast< double*>(_v)); break;
        case 6: setMinValue(*reinterpret_cast< double*>(_v)); break;
        case 7: setPipeWidth(*reinterpret_cast< int*>(_v)); break;
        case 8: setValue(*reinterpret_cast< double*>(_v)); break;
        }
        _id -= 9;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 9;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
