/****************************************************************************
** Meta object code from reading C++ file 'knob.h'
**
** Created: Mon May 30 20:11:26 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../knob.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'knob.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Knob[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,    5,    6,    5, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Knob[] = {
    "Knob\0\0double\0valueChanged(double)\0"
};

const QMetaObject Knob::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Knob,
      qt_meta_data_Knob, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Knob::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Knob::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Knob::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Knob))
        return static_cast<void*>(const_cast< Knob*>(this));
    return QWidget::qt_metacast(_clname);
}

int Knob::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { double _r = valueChanged((*reinterpret_cast< double(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
double Knob::valueChanged(double _t1)
{
    double _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)), const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
    return _t0;
}
QT_END_MOC_NAMESPACE
