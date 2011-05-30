/****************************************************************************
** Meta object code from reading C++ file 'qwt_plot_picker.h'
**
** Created: Mon May 30 19:22:11 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_plot_picker.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_plot_picker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtPlotPicker[] = {

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
      19,   15,   14,   14, 0x05,
      42,   37,   14,   14, 0x05,
      62,   59,   14,   14, 0x05,
      89,   15,   14,   14, 0x05,
     107,   15,   14,   14, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QwtPlotPicker[] = {
    "QwtPlotPicker\0\0pos\0selected(QPointF)\0"
    "rect\0selected(QRectF)\0pa\0"
    "selected(QVector<QPointF>)\0appended(QPointF)\0"
    "moved(QPointF)\0"
};

const QMetaObject QwtPlotPicker::staticMetaObject = {
    { &QwtPicker::staticMetaObject, qt_meta_stringdata_QwtPlotPicker,
      qt_meta_data_QwtPlotPicker, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtPlotPicker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtPlotPicker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtPlotPicker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPlotPicker))
        return static_cast<void*>(const_cast< QwtPlotPicker*>(this));
    return QwtPicker::qt_metacast(_clname);
}

int QwtPlotPicker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtPicker::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: selected((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 1: selected((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 2: selected((*reinterpret_cast< const QVector<QPointF>(*)>(_a[1]))); break;
        case 3: appended((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 4: moved((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QwtPlotPicker::selected(const QPointF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtPlotPicker::selected(const QRectF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QwtPlotPicker::selected(const QVector<QPointF> & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QwtPlotPicker::appended(const QPointF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QwtPlotPicker::moved(const QPointF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
