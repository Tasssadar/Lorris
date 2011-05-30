/****************************************************************************
** Meta object code from reading C++ file 'qwt_knob.h'
**
** Created: Mon May 30 19:22:23 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_knob.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_knob.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtKnob[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       7,   14, // properties
       2,   35, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      18,    8, 0x0009510b,
      40,   28, 0x0009510b,
      56,   52, 0x02095103,
      66,   52, 0x02095103,
      85,   78, 0x06095103,
      96,   52, 0x02095103,
      66,   52, 0x02095103,

 // enums: name, flags, count, data
       8, 0x0,    3,   43,
      28, 0x0,    5,   49,

 // enum data: key, value
     107, uint(QwtKnob::NoStyle),
     115, uint(QwtKnob::Raised),
     122, uint(QwtKnob::Sunken),
     129, uint(QwtKnob::NoMarker),
     138, uint(QwtKnob::Tick),
     143, uint(QwtKnob::Dot),
     147, uint(QwtKnob::Nub),
     151, uint(QwtKnob::Notch),

       0        // eod
};

static const char qt_meta_stringdata_QwtKnob[] = {
    "QwtKnob\0KnobStyle\0knobStyle\0MarkerStyle\0"
    "markerStyle\0int\0knobWidth\0borderWidth\0"
    "double\0totalAngle\0markerSize\0NoStyle\0"
    "Raised\0Sunken\0NoMarker\0Tick\0Dot\0Nub\0"
    "Notch\0"
};

const QMetaObject QwtKnob::staticMetaObject = {
    { &QwtAbstractSlider::staticMetaObject, qt_meta_stringdata_QwtKnob,
      qt_meta_data_QwtKnob, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtKnob::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtKnob::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtKnob::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtKnob))
        return static_cast<void*>(const_cast< QwtKnob*>(this));
    if (!strcmp(_clname, "QwtAbstractScale"))
        return static_cast< QwtAbstractScale*>(const_cast< QwtKnob*>(this));
    return QwtAbstractSlider::qt_metacast(_clname);
}

int QwtKnob::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtAbstractSlider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< KnobStyle*>(_v) = knobStyle(); break;
        case 1: *reinterpret_cast< MarkerStyle*>(_v) = markerStyle(); break;
        case 2: *reinterpret_cast< int*>(_v) = knobWidth(); break;
        case 3: *reinterpret_cast< int*>(_v) = borderWidth(); break;
        case 4: *reinterpret_cast< double*>(_v) = totalAngle(); break;
        case 5: *reinterpret_cast< int*>(_v) = markerSize(); break;
        case 6: *reinterpret_cast< int*>(_v) = borderWidth(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setKnobStyle(*reinterpret_cast< KnobStyle*>(_v)); break;
        case 1: setMarkerStyle(*reinterpret_cast< MarkerStyle*>(_v)); break;
        case 2: setKnobWidth(*reinterpret_cast< int*>(_v)); break;
        case 3: setBorderWidth(*reinterpret_cast< int*>(_v)); break;
        case 4: setTotalAngle(*reinterpret_cast< double*>(_v)); break;
        case 5: setMarkerSize(*reinterpret_cast< int*>(_v)); break;
        case 6: setBorderWidth(*reinterpret_cast< int*>(_v)); break;
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
