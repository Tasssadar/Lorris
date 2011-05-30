/****************************************************************************
** Meta object code from reading C++ file 'qwt_picker.h'
**
** Created: Mon May 30 19:22:02 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_picker.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_picker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtPicker[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       7,   49, // properties
       3,   70, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   11,   10,   10, 0x05,
      38,   30,   10,   10, 0x05,
      61,   57,   10,   10, 0x05,
      78,   57,   10,   10, 0x05,
      92,   57,   10,   10, 0x05,
     118,  108,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     136,   10,   10,   10, 0x0a,

 // properties: name, type, flags
     158,  153, 0x01095003,
     179,  168, 0x0009510b,
     202,  190, 0x0009510b,
     219,  214, 0x4d095103,
     236,  230, 0x40095103,
     259,  248, 0x0009510b,
     270,  214, 0x4d095103,

 // enums: name, flags, count, data
     248, 0x0,    8,   82,
     190, 0x0,    3,   98,
     168, 0x0,    2,  104,

 // enum data: key, value
     284, uint(QwtPicker::NoRubberBand),
     297, uint(QwtPicker::HLineRubberBand),
     313, uint(QwtPicker::VLineRubberBand),
     329, uint(QwtPicker::CrossRubberBand),
     345, uint(QwtPicker::RectRubberBand),
     360, uint(QwtPicker::EllipseRubberBand),
     378, uint(QwtPicker::PolygonRubberBand),
     396, uint(QwtPicker::UserRubberBand),
     411, uint(QwtPicker::AlwaysOff),
     421, uint(QwtPicker::AlwaysOn),
     430, uint(QwtPicker::ActiveOnly),
     441, uint(QwtPicker::Stretch),
     449, uint(QwtPicker::KeepSize),

       0        // eod
};

static const char qt_meta_stringdata_QwtPicker[] = {
    "QwtPicker\0\0on\0activated(bool)\0polygon\0"
    "selected(QPolygon)\0pos\0appended(QPoint)\0"
    "moved(QPoint)\0removed(QPoint)\0selection\0"
    "changed(QPolygon)\0setEnabled(bool)\0"
    "bool\0isEnabled\0ResizeMode\0resizeMode\0"
    "DisplayMode\0trackerMode\0QPen\0trackerPen\0"
    "QFont\0trackerFont\0RubberBand\0rubberBand\0"
    "rubberBandPen\0NoRubberBand\0HLineRubberBand\0"
    "VLineRubberBand\0CrossRubberBand\0"
    "RectRubberBand\0EllipseRubberBand\0"
    "PolygonRubberBand\0UserRubberBand\0"
    "AlwaysOff\0AlwaysOn\0ActiveOnly\0Stretch\0"
    "KeepSize\0"
};

const QMetaObject QwtPicker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QwtPicker,
      qt_meta_data_QwtPicker, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtPicker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtPicker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtPicker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPicker))
        return static_cast<void*>(const_cast< QwtPicker*>(this));
    if (!strcmp(_clname, "QwtEventPattern"))
        return static_cast< QwtEventPattern*>(const_cast< QwtPicker*>(this));
    return QObject::qt_metacast(_clname);
}

int QwtPicker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activated((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: selected((*reinterpret_cast< const QPolygon(*)>(_a[1]))); break;
        case 2: appended((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 3: moved((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 4: removed((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 5: changed((*reinterpret_cast< const QPolygon(*)>(_a[1]))); break;
        case 6: setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = isEnabled(); break;
        case 1: *reinterpret_cast< ResizeMode*>(_v) = resizeMode(); break;
        case 2: *reinterpret_cast< DisplayMode*>(_v) = trackerMode(); break;
        case 3: *reinterpret_cast< QPen*>(_v) = trackerPen(); break;
        case 4: *reinterpret_cast< QFont*>(_v) = trackerFont(); break;
        case 5: *reinterpret_cast< RubberBand*>(_v) = rubberBand(); break;
        case 6: *reinterpret_cast< QPen*>(_v) = rubberBandPen(); break;
        }
        _id -= 7;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 1: setResizeMode(*reinterpret_cast< ResizeMode*>(_v)); break;
        case 2: setTrackerMode(*reinterpret_cast< DisplayMode*>(_v)); break;
        case 3: setTrackerPen(*reinterpret_cast< QPen*>(_v)); break;
        case 4: setTrackerFont(*reinterpret_cast< QFont*>(_v)); break;
        case 5: setRubberBand(*reinterpret_cast< RubberBand*>(_v)); break;
        case 6: setRubberBandPen(*reinterpret_cast< QPen*>(_v)); break;
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

// SIGNAL 0
void QwtPicker::activated(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtPicker::selected(const QPolygon & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QwtPicker::appended(const QPoint & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QwtPicker::moved(const QPoint & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QwtPicker::removed(const QPoint & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void QwtPicker::changed(const QPolygon & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
