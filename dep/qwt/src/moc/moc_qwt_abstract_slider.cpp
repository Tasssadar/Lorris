/****************************************************************************
** Meta object code from reading C++ file 'qwt_abstract_slider.h'
**
** Created: Mon May 30 19:22:17 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_abstract_slider.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_abstract_slider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtAbstractSlider[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       4,   54, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   19,   18,   18, 0x05,
      46,   18,   18,   18, 0x05,
      62,   18,   18,   18, 0x05,
      79,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     103,   99,   18,   18, 0x0a,
     120,   99,   18,   18, 0x0a,
     143,  137,   18,   18, 0x0a,
     157,   18,   18,   18, 0x0a,

 // properties: name, type, flags
     180,  175, 0x01095103,
     189,  175, 0x01095103,
     202,  195, 0x06095103,
     223,  207, 0x0009510b,

       0        // eod
};

static const char qt_meta_stringdata_QwtAbstractSlider[] = {
    "QwtAbstractSlider\0\0value\0valueChanged(double)\0"
    "sliderPressed()\0sliderReleased()\0"
    "sliderMoved(double)\0val\0setValue(double)\0"
    "fitValue(double)\0steps\0incValue(int)\0"
    "setReadOnly(bool)\0bool\0readOnly\0valid\0"
    "double\0mass\0Qt::Orientation\0orientation\0"
};

const QMetaObject QwtAbstractSlider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QwtAbstractSlider,
      qt_meta_data_QwtAbstractSlider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtAbstractSlider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtAbstractSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtAbstractSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtAbstractSlider))
        return static_cast<void*>(const_cast< QwtAbstractSlider*>(this));
    if (!strcmp(_clname, "QwtDoubleRange"))
        return static_cast< QwtDoubleRange*>(const_cast< QwtAbstractSlider*>(this));
    return QWidget::qt_metacast(_clname);
}

int QwtAbstractSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: sliderPressed(); break;
        case 2: sliderReleased(); break;
        case 3: sliderMoved((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: setValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: fitValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: incValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: setReadOnly((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 8;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = isReadOnly(); break;
        case 1: *reinterpret_cast< bool*>(_v) = isValid(); break;
        case 2: *reinterpret_cast< double*>(_v) = mass(); break;
        case 3: *reinterpret_cast< Qt::Orientation*>(_v) = orientation(); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setReadOnly(*reinterpret_cast< bool*>(_v)); break;
        case 1: setValid(*reinterpret_cast< bool*>(_v)); break;
        case 2: setMass(*reinterpret_cast< double*>(_v)); break;
        case 3: setOrientation(*reinterpret_cast< Qt::Orientation*>(_v)); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QwtAbstractSlider::valueChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtAbstractSlider::sliderPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QwtAbstractSlider::sliderReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void QwtAbstractSlider::sliderMoved(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
