/****************************************************************************
** Meta object code from reading C++ file 'qwt_plot_zoomer.h'
**
** Created: Mon May 30 19:22:13 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_plot_zoomer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_plot_zoomer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtPlotZoomer[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,   35,   14,   14, 0x0a,
      61,   14,   14,   14, 0x0a,
      77,   14,   14,   14, 0x0a,
      93,   90,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QwtPlotZoomer[] = {
    "QwtPlotZoomer\0\0rect\0zoomed(QRectF)\0"
    "x,y\0moveBy(double,double)\0moveTo(QPointF)\0"
    "zoom(QRectF)\0up\0zoom(int)\0"
};

const QMetaObject QwtPlotZoomer::staticMetaObject = {
    { &QwtPlotPicker::staticMetaObject, qt_meta_stringdata_QwtPlotZoomer,
      qt_meta_data_QwtPlotZoomer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtPlotZoomer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtPlotZoomer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtPlotZoomer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPlotZoomer))
        return static_cast<void*>(const_cast< QwtPlotZoomer*>(this));
    return QwtPlotPicker::qt_metacast(_clname);
}

int QwtPlotZoomer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtPlotPicker::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: zoomed((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 1: moveBy((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 2: moveTo((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 3: zoom((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 4: zoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QwtPlotZoomer::zoomed(const QRectF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
