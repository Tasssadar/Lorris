/****************************************************************************
** Meta object code from reading C++ file 'qwt_slider.h'
**
** Created: Mon May 30 19:22:25 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_slider.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_slider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtSlider[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       5,   14, // properties
       2,   29, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      19,   10, 0x0009510b,
      50,   33, 0x0009510b,
      72,   66, 0x15095103,
      87,   83, 0x02095103,
      99,   83, 0x02095103,

 // enums: name, flags, count, data
      10, 0x0,    5,   37,
     107, 0x0,    2,   47,

 // enum data: key, value
     123, uint(QwtSlider::NoScale),
     131, uint(QwtSlider::LeftScale),
     141, uint(QwtSlider::RightScale),
     152, uint(QwtSlider::TopScale),
     161, uint(QwtSlider::BottomScale),
     173, uint(QwtSlider::Trough),
     180, uint(QwtSlider::Groove),

       0        // eod
};

static const char qt_meta_stringdata_QwtSlider[] = {
    "QwtSlider\0ScalePos\0scalePosition\0"
    "BackgroundStyles\0backgroundStyle\0QSize\0"
    "handleSize\0int\0borderWidth\0spacing\0"
    "BackgroundStyle\0NoScale\0LeftScale\0"
    "RightScale\0TopScale\0BottomScale\0Trough\0"
    "Groove\0"
};

const QMetaObject QwtSlider::staticMetaObject = {
    { &QwtAbstractSlider::staticMetaObject, qt_meta_stringdata_QwtSlider,
      qt_meta_data_QwtSlider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtSlider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtSlider))
        return static_cast<void*>(const_cast< QwtSlider*>(this));
    if (!strcmp(_clname, "QwtAbstractScale"))
        return static_cast< QwtAbstractScale*>(const_cast< QwtSlider*>(this));
    return QwtAbstractSlider::qt_metacast(_clname);
}

int QwtSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtAbstractSlider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< ScalePos*>(_v) = scalePosition(); break;
        case 1: *reinterpret_cast< BackgroundStyles*>(_v) = backgroundStyle(); break;
        case 2: *reinterpret_cast< QSize*>(_v) = handleSize(); break;
        case 3: *reinterpret_cast< int*>(_v) = borderWidth(); break;
        case 4: *reinterpret_cast< int*>(_v) = spacing(); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setScalePosition(*reinterpret_cast< ScalePos*>(_v)); break;
        case 1: setBackgroundStyle(*reinterpret_cast< BackgroundStyles*>(_v)); break;
        case 2: setHandleSize(*reinterpret_cast< QSize*>(_v)); break;
        case 3: setBorderWidth(*reinterpret_cast< int*>(_v)); break;
        case 4: setSpacing(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
