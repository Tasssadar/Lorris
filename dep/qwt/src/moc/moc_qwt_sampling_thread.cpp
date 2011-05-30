/****************************************************************************
** Meta object code from reading C++ file 'qwt_sampling_thread.h'
**
** Created: Mon May 30 19:22:14 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_sampling_thread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_sampling_thread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtSamplingThread[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   19,   18,   18, 0x0a,
      48,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QwtSamplingThread[] = {
    "QwtSamplingThread\0\0interval\0"
    "setInterval(double)\0stop()\0"
};

const QMetaObject QwtSamplingThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_QwtSamplingThread,
      qt_meta_data_QwtSamplingThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtSamplingThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtSamplingThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtSamplingThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtSamplingThread))
        return static_cast<void*>(const_cast< QwtSamplingThread*>(this));
    return QThread::qt_metacast(_clname);
}

int QwtSamplingThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setInterval((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: stop(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
