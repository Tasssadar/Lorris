/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <QMouseEvent>

#include "canvaswidget.h"

CanvasWidget::CanvasWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Canvas"));
    setIcon(":/dataWidgetIcons/canvas.png");

    m_widgetType = WIDGET_CANVAS;

    m_canvas = new Canvas(this);
    layout->addWidget(m_canvas);

    m_lineWidth = 2;
    m_lineColor = Qt::black;

    resize(200, 200);
}

CanvasWidget::~CanvasWidget()
{

}

void CanvasWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);
}

void CanvasWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("canvasWLineSett");
    {
        file->writeVal(m_lastLine.x());
        file->writeVal(m_lastLine.y());

        file->writeVal(m_lineWidth);

        file->writeString(m_lineColor.name());
    }

    m_canvas->save(file);
}

void CanvasWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("canvasWLineSett", BLOCK_WIDGET))
    {
        m_lastLine.setX(file->readVal<int>());
        m_lastLine.setY(file->readVal<int>());

        m_lineWidth = file->readVal<int>();

        m_lineColor = QColor(file->readString());
    }

    m_canvas->load(file);
}

void CanvasWidget::setBackground(const QString &color)
{
    m_canvas->setBackground(QColor(color));
}

void CanvasWidget::drawLine(int x1, int y1, int x2, int y2)
{
    m_canvas->addLine(QLine(x1, y1, x2, y2), m_lineColor, m_lineWidth);
    m_lastLine = QPoint(x2, y2);
}

void CanvasWidget::drawLine(int x, int y)
{
    m_canvas->addLine(QLine(m_lastLine, QPoint(x, y)), m_lineColor, m_lineWidth);
    m_lastLine = QPoint(x, y);
}

void CanvasWidget::setLineSize(int width)
{
    m_lineWidth = width;
}

void CanvasWidget::setLineColor(const QString &color)
{
    m_lineColor = QColor(color);
}

void CanvasWidget::clear()
{
    m_canvas->clear();
}

CanvasWidgetAddBtn::CanvasWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Canvas"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/canvas.png"));

    m_widgetType = WIDGET_CANVAS;
}

Canvas::Canvas(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);
    setBackground(Qt::white);

    setCursor(Qt::OpenHandCursor);
}

void Canvas::save(DataFileParser *file)
{
    file->writeBlockIdentifier("canvasWcanvas");
    file->writeString(palette().color(QPalette::Window).name());

    file->writeVal((quint32)m_lines.size());
    for(quint32 i = 0; i < m_lines.size(); ++i)
    {
        line& l = m_lines[i];

        file->writeVal(l.l.x1());
        file->writeVal(l.l.y1());
        file->writeVal(l.l.x2());
        file->writeVal(l.l.y2());

        file->writeVal(l.width);
        file->writeString(l.color.name());
    }
}

void Canvas::load(DataFileParser *file)
{
    if(!file->seekToNextBlock("canvasWcanvas", BLOCK_WIDGET))
        return;

    setBackground(QColor(file->readString()));

    quint32 count = file->readVal<quint32>();
    for(quint32 i = 0; i < count; ++i)
    {
        line l;

        l.l.setP1(QPoint(file->readVal<int>(), file->readVal<int>()));
        l.l.setP2(QPoint(file->readVal<int>(), file->readVal<int>()));

        l.width = file->readVal<int>();
        l.color = QColor(file->readString());

        m_lines.push_back(l);
    }
    update();
}

void Canvas::setBackground(QColor bg)
{
    QPalette p(palette());
    p.setColor(QPalette::Window, bg);
    setPalette(p);

    update();
}

void Canvas::addLine(const QLine &l, const QColor &c, int width)
{
    line lin(l, c, width);
    lin.l.setPoints(lin.l.p1() + m_offset, lin.l.p2() + m_offset);
    m_lines.push_back(lin);

    update();
}

void Canvas::clear()
{
    m_lines.clear();
    m_offset = QPoint();
    update();
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    for(quint32 i = 0; i < m_lines.size(); ++i)
    {
        line l = m_lines[i];

        QPen pen(l.color);
        pen.setWidth(l.width);
        p.setPen(pen);

        p.drawLine(l.l);
    }
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        setCursor(Qt::ClosedHandCursor);
        m_mouse = event->globalPos();
    }
    QWidget::mousePressEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        setCursor(Qt::OpenHandCursor);
    QWidget::mouseReleaseEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton))
        return QWidget::mouseMoveEvent(event);

    QPoint diff = event->globalPos() - m_mouse;
    m_mouse = event->globalPos();

    m_offset += diff;
    for(quint32 i = 0; i < m_lines.size(); ++i)
    {
        QLine &l = m_lines[i].l;
        l.setPoints(l.p1()+diff, l.p2()+diff);
    }
    update();
}
