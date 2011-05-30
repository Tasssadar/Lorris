/****************************************************************************
** Meta object code from reading C++ file 'qwt_scale_widget.h'
**
** Created: Mon May 30 19:22:15 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_scale_widget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_scale_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtScaleWidget[] = {

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
      16,   15,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QwtScaleWidget[] = {
    "QwtScaleWidget\0\0scaleDivChanged()\0"
};

const QMetaObject QwtScaleWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QwtScaleWidget,
      qt_meta_data_QwtScaleWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtScaleWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtScaleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtScaleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtScaleWidget))
        return static_cast<void*>(const_cast< QwtScaleWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int QwtScaleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: scaleDivChanged(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void QwtScaleWidget::scaleDivChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
