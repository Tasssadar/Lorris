/****************************************************************************
** Meta object code from reading C++ file 'qwt_plot.h'
**
** Created: Mon May 30 19:22:07 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_plot.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_plot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtPlot[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       1,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,    9,    8,    8, 0x05,
      58,   46,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      91,    8,    8,    8, 0x0a,
     100,    8,    8,    8, 0x0a,
     114,    8,    8,    8, 0x09,
     134,    8,    8,    8, 0x09,

 // properties: name, type, flags
     166,  158, 0x0a095003,

       0        // eod
};

static const char qt_meta_stringdata_QwtPlot[] = {
    "QwtPlot\0\0plotItem\0legendClicked(QwtPlotItem*)\0"
    "plotItem,on\0legendChecked(QwtPlotItem*,bool)\0"
    "replot()\0autoRefresh()\0legendItemClicked()\0"
    "legendItemChecked(bool)\0QString\0"
    "propertiesDocument\0"
};

const QMetaObject QwtPlot::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_QwtPlot,
      qt_meta_data_QwtPlot, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtPlot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPlot))
        return static_cast<void*>(const_cast< QwtPlot*>(this));
    if (!strcmp(_clname, "QwtPlotDict"))
        return static_cast< QwtPlotDict*>(const_cast< QwtPlot*>(this));
    return QFrame::qt_metacast(_clname);
}

int QwtPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: legendClicked((*reinterpret_cast< QwtPlotItem*(*)>(_a[1]))); break;
        case 1: legendChecked((*reinterpret_cast< QwtPlotItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: replot(); break;
        case 3: autoRefresh(); break;
        case 4: legendItemClicked(); break;
        case 5: legendItemChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = grabProperties(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: applyProperties(*reinterpret_cast< QString*>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QwtPlot::legendClicked(QwtPlotItem * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtPlot::legendChecked(QwtPlotItem * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
