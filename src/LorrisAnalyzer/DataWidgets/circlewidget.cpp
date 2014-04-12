/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <math.h>
#include <QSignalMapper>

#include "circlewidget.h"
#include "../../ui/floatinginputdialog.h"

static const double pi = 3.1415926535897932384626433832795;

void CircleWidget::addEnum()
{
    REGISTER_ENUM(ANG_RAD);
    REGISTER_ENUM(ANG_DEG);
    REGISTER_ENUM(ANG_RANGE);
}

REGISTER_DATAWIDGET(WIDGET_CIRCLE, Circle, &CircleWidget::addEnum)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Circle"))

CircleWidget::CircleWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_circle = new CircleDraw(this);
    layout->addWidget(m_circle, 1);
    resize(150, 150);
}

CircleWidget::~CircleWidget()
{

}

void CircleWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_range_min = 0;
    m_range_max = 4095;
    m_num_type = NUM_UINT8;

    QMenu *bitsMenu = contextMenu->addMenu(tr("Data type"));

    static const QString dataTypes[] =
    {
        tr("unsigned 8bit"),
        tr("unsigned 16bit"),
        tr("unsigned 32bit"),
        tr("unsigned 64bit"),

        tr("signed 8bit"),
        tr("signed 16bit"),
        tr("signed 32bit"),
        tr("signed 64bit"),

        tr("float (4 bytes)"),
        tr("double (8 bytes)")
    };

    QSignalMapper *signalMapBits = new QSignalMapper(this);
    for(quint8 i = 0; i < NUM_COUNT; ++i)
    {
        if(i%4 == 0 && i != 0)
            bitsMenu->addSeparator();

        m_bits_act[i] = bitsMenu->addAction(dataTypes[i]);
        m_bits_act[i]->setCheckable(true);
        signalMapBits->setMapping(m_bits_act[i], i);
        connect(m_bits_act[i], SIGNAL(triggered()), signalMapBits, SLOT(map()));
    }
    m_bits_act[0]->setChecked(true);
    connect(signalMapBits, SIGNAL(mapped(int)), SLOT(setDataType(int)));

    static const QString title[ANG_MAX] = {
        tr("Radians"),
        tr("Degrees"),
        tr("Range...")
    };

    QSignalMapper *map = new QSignalMapper(this);
    QMenu *inMenu = contextMenu->addMenu(tr("Input"));
    for(int i = 0; i < ANG_MAX; ++i)
    {
        m_type_act[i] = inMenu->addAction(title[i]);
        m_type_act[i]->setCheckable(true);
        map->setMapping(m_type_act[i], i);
        connect(m_type_act[i], SIGNAL(triggered()), map, SLOT(map()));
    }
    setAngType(ANG_RAD);
    connect(map, SIGNAL(mapped(int)), SLOT(angTypeChanged(int)));

    m_clockwiseAct = contextMenu->addAction(tr("Clockwise"));
    m_clockwiseAct->setCheckable(true);
    m_clockwiseAct->setChecked(true);

    m_drawAngAct = contextMenu->addAction(tr("Show angle as text"));
    m_drawAngAct->setCheckable(true);
    m_drawAngAct->setChecked(true);

    contextMenu->addAction(tr("Set formula..."), this, SLOT(showFormulaDialog()));

    connect(m_clockwiseAct, SIGNAL(triggered(bool)), SLOT(setClockwise(bool)));
    connect(m_drawAngAct, SIGNAL(triggered(bool)),   SLOT(drawAngle(bool)));
}

void CircleWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("circleWtype");
    *file << m_ang_type << m_num_type;

    file->writeBlockIdentifier("circleWrangeDouble");
    *file << m_range_min << m_range_max;

    file->writeBlockIdentifier("circleWclockwise");
    *file << m_clockwiseAct->isChecked();

    file->writeBlockIdentifier("circleWdrawAngText");
    *file << m_drawAngAct->isChecked();

    file->writeBlockIdentifier("circleWformula");
    *file << m_eval.getFormula();
}

void CircleWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("circleWtype", BLOCK_WIDGET))
        *file >> m_ang_type >> m_num_type;

    if(file->seekToNextBlock("circleWrangeDouble", BLOCK_WIDGET))
        *file >> m_range_min >> m_range_max;
    else if(file->seekToNextBlock("circleWrange", BLOCK_WIDGET))
    {
        m_range_min = file->readVal<quint32>();
        m_range_max = file->readVal<quint32>();
    }

    if(file->seekToNextBlock("circleWclockwise", BLOCK_WIDGET))
    {
        bool clockwise = file->readVal<bool>();
        m_clockwiseAct->setChecked(clockwise);
        m_circle->setClockwise(clockwise);
    }

    if(file->seekToNextBlock("circleWdrawAngText", BLOCK_WIDGET))
        drawAngle(file->readVal<bool>());

    if(file->seekToNextBlock("circleWformula", BLOCK_WIDGET))
        m_eval.setFormula(file->readString());

    setAngType(m_ang_type);
    setDataType(m_num_type);
}

void CircleWidget::setValue(QVariant var)
{
    if(m_eval.isActive())
    {
        // .toString() on char type makes one
        // character string instead of value transcript
        if ((int)var.type() == QMetaType::QChar ||
            (int)var.type() == QMetaType::UChar)
            var.convert(QVariant::Int);

        QVariant res = m_eval.evaluate(var.toString());
        if(res.isValid())
            var.swap(res);
    }

    float rad = toRad(var);
    if(!m_clockwiseAct->isChecked())
        rad = (2*pi) - rad;
    m_circle->setAngle(rad);
}

void CircleWidget::processData(analyzer_data *data)
{
    QVariant var = DataWidget::getNumFromPacket(data, m_info.pos, m_num_type);
    setValue(var);
}

void CircleWidget::angTypeChanged(int i)
{
    if(i == ANG_RANGE)
        FloatingInputDialog::getDoubleRange(tr("Circle's range:"), m_range_min, m_range_max, 3);
    setAngType(i, m_range_min, m_range_max);
}

void CircleWidget::setAngType(int i, double min, double max)
{
    for(int y = 0; y < ANG_MAX; ++y)
        m_type_act[y]->setChecked(y == i);

    m_ang_type = i;
    m_range_min = min;
    m_range_max = max;

    if(i != ANG_RANGE)
        m_circle->userVal() = QVariant();

    emit updateForMe();
}

float CircleWidget::toRad(const QVariant& var)
{
    float ret = 0.0f;
    switch(m_ang_type)
    {
        case ANG_RAD:
            ret = var.toFloat();
            break;
        case ANG_DEG:
        {
            ret = var.toFloat();
            ret = (ret * pi*2) / 360.f;
            break;
        }
        case ANG_RANGE:
        {
            m_circle->userVal() = var;
            ret = var.toDouble()-m_range_min;
            ret = (ret * pi*2) / (m_range_max-m_range_min);
            break;
        }
    }
    ret = std::min(float(pi*2), ret);
    ret = std::max(ret, 0.f);
    return ret;
}

void CircleWidget::setDataType(int i)
{
    for(quint8 y = 0; y < NUM_COUNT; ++y)
        m_bits_act[y]->setChecked(y == i);

    m_num_type = i;
    emit updateForMe();
}

void CircleWidget::setClockwise(bool clockwise)
{
    m_clockwiseAct->setChecked(clockwise);
    m_circle->setClockwise(clockwise);
    emit updateForMe();
}

void CircleWidget::drawAngle(bool draw)
{
    m_drawAngAct->setChecked(draw);
    m_circle->setDrawAngle(draw);
}

void CircleWidget::setFormula(const QString &formula)
{
    m_eval.setFormula(formula);
    emit updateForMe();
}

void CircleWidget::showFormulaDialog()
{
    m_eval.showFormulaDialog();
    emit updateForMe();
}

CircleDraw::CircleDraw(QWidget *parent) : QWidget(parent)
{
    m_angle = 0;
    m_clockwise = true;
    m_draw_angle = true;
}

void CircleDraw::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPen pen(Qt::black);
    pen.setWidth(4);
    p.setBrush(QBrush(Qt::white));
    p.setPen(pen);

    int r = (std::min(width(), height()) / 2) - 5;
    QPoint center(width()/2, height()/2);

    p.drawEllipse(center, r, r);

    QPoint rot(center);
    rot.rx() += sin(m_angle)*r;
    rot.ry() -= cos(m_angle)*r;
    p.drawLine(center, rot);

    p.setPen(Qt::black);
    p.drawLine(center.x(), center.y()-r, center.x(), center.y()+r);
    p.drawLine(center.x()-r, center.y(), center.x()+r, center.y());

    if(m_draw_angle)
    {
        p.drawText(rect(), QString("R%1\n%2\x00b0").arg(m_angle, -1, 'f', 3).arg(m_angle*57.2958, -1, 'f', 1));

        if(m_userVal.isValid())
        {
            QString str = m_userVal.toString();
            int w = fontMetrics().width(str)+5;
            p.drawText(QRect(width()-w, 0, w, height()), str);
        }
    }

    pen.setColor(Qt::red);
    pen.setWidth(5);
    p.setPen(pen);
    p.drawPoint(center);

    QRect angRect(center.x()-r, center.y()-r, r*2, r*2);
    int arcAngle = m_angle*916.732; // (1/(pi*2))*5760 = 916.732
    p.drawArc(angRect, 1440, m_clockwise ? -arcAngle : 5760 - arcAngle);
}

void CircleDraw::setAngle(float ang)
{
    m_angle = ang;
    update();
}

void CircleDraw::setClockwise(bool clockwise)
{
    m_clockwise = clockwise;
    update();
}
