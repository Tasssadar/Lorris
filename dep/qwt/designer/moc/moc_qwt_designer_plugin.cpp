/****************************************************************************
** Meta object code from reading C++ file 'qwt_designer_plugin.h'
**
** Created: Mon May 30 19:23:07 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_designer_plugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_designer_plugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QwtDesignerPlugin__CustomWidgetInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetInterface[] = {
    "QwtDesignerPlugin::CustomWidgetInterface\0"
};

const QMetaObject QwtDesignerPlugin::CustomWidgetInterface::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetInterface,
      qt_meta_data_QwtDesignerPlugin__CustomWidgetInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::CustomWidgetInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::CustomWidgetInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::CustomWidgetInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetInterface))
        return static_cast<void*>(const_cast< CustomWidgetInterface*>(this));
    if (!strcmp(_clname, "QDesignerCustomWidgetInterface"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< CustomWidgetInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< CustomWidgetInterface*>(this));
    return QObject::qt_metacast(_clname);
}

int QwtDesignerPlugin::CustomWidgetInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__CustomWidgetCollectionInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetCollectionInterface[] = {
    "QwtDesignerPlugin::CustomWidgetCollectionInterface\0"
};

const QMetaObject QwtDesignerPlugin::CustomWidgetCollectionInterface::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetCollectionInterface,
      qt_meta_data_QwtDesignerPlugin__CustomWidgetCollectionInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::CustomWidgetCollectionInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::CustomWidgetCollectionInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::CustomWidgetCollectionInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__CustomWidgetCollectionInterface))
        return static_cast<void*>(const_cast< CustomWidgetCollectionInterface*>(this));
    if (!strcmp(_clname, "QDesignerCustomWidgetCollectionInterface"))
        return static_cast< QDesignerCustomWidgetCollectionInterface*>(const_cast< CustomWidgetCollectionInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidgetCollection"))
        return static_cast< QDesignerCustomWidgetCollectionInterface*>(const_cast< CustomWidgetCollectionInterface*>(this));
    return QObject::qt_metacast(_clname);
}

int QwtDesignerPlugin::CustomWidgetCollectionInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__PlotInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__PlotInterface[] = {
    "QwtDesignerPlugin::PlotInterface\0"
};

const QMetaObject QwtDesignerPlugin::PlotInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__PlotInterface,
      qt_meta_data_QwtDesignerPlugin__PlotInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::PlotInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::PlotInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::PlotInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__PlotInterface))
        return static_cast<void*>(const_cast< PlotInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< PlotInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::PlotInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__AnalogClockInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__AnalogClockInterface[] = {
    "QwtDesignerPlugin::AnalogClockInterface\0"
};

const QMetaObject QwtDesignerPlugin::AnalogClockInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__AnalogClockInterface,
      qt_meta_data_QwtDesignerPlugin__AnalogClockInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::AnalogClockInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::AnalogClockInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::AnalogClockInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__AnalogClockInterface))
        return static_cast<void*>(const_cast< AnalogClockInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< AnalogClockInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::AnalogClockInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__CompassInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__CompassInterface[] = {
    "QwtDesignerPlugin::CompassInterface\0"
};

const QMetaObject QwtDesignerPlugin::CompassInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__CompassInterface,
      qt_meta_data_QwtDesignerPlugin__CompassInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::CompassInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::CompassInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::CompassInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__CompassInterface))
        return static_cast<void*>(const_cast< CompassInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< CompassInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::CompassInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__CounterInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__CounterInterface[] = {
    "QwtDesignerPlugin::CounterInterface\0"
};

const QMetaObject QwtDesignerPlugin::CounterInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__CounterInterface,
      qt_meta_data_QwtDesignerPlugin__CounterInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::CounterInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::CounterInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::CounterInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__CounterInterface))
        return static_cast<void*>(const_cast< CounterInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< CounterInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::CounterInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__DialInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__DialInterface[] = {
    "QwtDesignerPlugin::DialInterface\0"
};

const QMetaObject QwtDesignerPlugin::DialInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__DialInterface,
      qt_meta_data_QwtDesignerPlugin__DialInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::DialInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::DialInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::DialInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__DialInterface))
        return static_cast<void*>(const_cast< DialInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< DialInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::DialInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__KnobInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__KnobInterface[] = {
    "QwtDesignerPlugin::KnobInterface\0"
};

const QMetaObject QwtDesignerPlugin::KnobInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__KnobInterface,
      qt_meta_data_QwtDesignerPlugin__KnobInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::KnobInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::KnobInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::KnobInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__KnobInterface))
        return static_cast<void*>(const_cast< KnobInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< KnobInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::KnobInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__ScaleWidgetInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__ScaleWidgetInterface[] = {
    "QwtDesignerPlugin::ScaleWidgetInterface\0"
};

const QMetaObject QwtDesignerPlugin::ScaleWidgetInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__ScaleWidgetInterface,
      qt_meta_data_QwtDesignerPlugin__ScaleWidgetInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::ScaleWidgetInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::ScaleWidgetInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::ScaleWidgetInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__ScaleWidgetInterface))
        return static_cast<void*>(const_cast< ScaleWidgetInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< ScaleWidgetInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::ScaleWidgetInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__SliderInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__SliderInterface[] = {
    "QwtDesignerPlugin::SliderInterface\0"
};

const QMetaObject QwtDesignerPlugin::SliderInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__SliderInterface,
      qt_meta_data_QwtDesignerPlugin__SliderInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::SliderInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::SliderInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::SliderInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__SliderInterface))
        return static_cast<void*>(const_cast< SliderInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< SliderInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::SliderInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__TextLabelInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__TextLabelInterface[] = {
    "QwtDesignerPlugin::TextLabelInterface\0"
};

const QMetaObject QwtDesignerPlugin::TextLabelInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__TextLabelInterface,
      qt_meta_data_QwtDesignerPlugin__TextLabelInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::TextLabelInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::TextLabelInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::TextLabelInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__TextLabelInterface))
        return static_cast<void*>(const_cast< TextLabelInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< TextLabelInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::TextLabelInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__ThermoInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__ThermoInterface[] = {
    "QwtDesignerPlugin::ThermoInterface\0"
};

const QMetaObject QwtDesignerPlugin::ThermoInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__ThermoInterface,
      qt_meta_data_QwtDesignerPlugin__ThermoInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::ThermoInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::ThermoInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::ThermoInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__ThermoInterface))
        return static_cast<void*>(const_cast< ThermoInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< ThermoInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::ThermoInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__WheelInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__WheelInterface[] = {
    "QwtDesignerPlugin::WheelInterface\0"
};

const QMetaObject QwtDesignerPlugin::WheelInterface::staticMetaObject = {
    { &CustomWidgetInterface::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__WheelInterface,
      qt_meta_data_QwtDesignerPlugin__WheelInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::WheelInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::WheelInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::WheelInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__WheelInterface))
        return static_cast<void*>(const_cast< WheelInterface*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.CustomWidget"))
        return static_cast< QDesignerCustomWidgetInterface*>(const_cast< WheelInterface*>(this));
    return CustomWidgetInterface::qt_metacast(_clname);
}

int QwtDesignerPlugin::WheelInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CustomWidgetInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__TaskMenuFactory[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__TaskMenuFactory[] = {
    "QwtDesignerPlugin::TaskMenuFactory\0"
};

const QMetaObject QwtDesignerPlugin::TaskMenuFactory::staticMetaObject = {
    { &QExtensionFactory::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__TaskMenuFactory,
      qt_meta_data_QwtDesignerPlugin__TaskMenuFactory, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::TaskMenuFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::TaskMenuFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::TaskMenuFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__TaskMenuFactory))
        return static_cast<void*>(const_cast< TaskMenuFactory*>(this));
    return QExtensionFactory::qt_metacast(_clname);
}

int QwtDesignerPlugin::TaskMenuFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QExtensionFactory::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QwtDesignerPlugin__TaskMenuExtension[] = {

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
      38,   37,   37,   37, 0x08,
      55,   37,   37,   37, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__TaskMenuExtension[] = {
    "QwtDesignerPlugin::TaskMenuExtension\0"
    "\0editProperties()\0applyProperties(QString)\0"
};

const QMetaObject QwtDesignerPlugin::TaskMenuExtension::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__TaskMenuExtension,
      qt_meta_data_QwtDesignerPlugin__TaskMenuExtension, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QwtDesignerPlugin::TaskMenuExtension::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QwtDesignerPlugin::TaskMenuExtension::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QwtDesignerPlugin::TaskMenuExtension::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__TaskMenuExtension))
        return static_cast<void*>(const_cast< TaskMenuExtension*>(this));
    if (!strcmp(_clname, "QDesignerTaskMenuExtension"))
        return static_cast< QDesignerTaskMenuExtension*>(const_cast< TaskMenuExtension*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.Designer.TaskMenu"))
        return static_cast< QDesignerTaskMenuExtension*>(const_cast< TaskMenuExtension*>(this));
    return QObject::qt_metacast(_clname);
}

int QwtDesignerPlugin::TaskMenuExtension::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: editProperties(); break;
        case 1: applyProperties((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
