/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <QMouseEvent>

#include "canvaswidget.h"
#include "../../misc/utils.h"

REGISTER_DATAWIDGET(WIDGET_CANVAS, Canvas)

CanvasWidget::CanvasWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Canvas"));
    setIcon(":/dataWidgetIcons/canvas.png");

    m_widgetType = WIDGET_CANVAS;

    m_canvas = new Canvas(this);
    layout->addWidget(m_canvas);

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

        file->writeVal(m_canvas->lineWidth());

        file->writeString(m_canvas->lineColor().name());
    }

    file->writeBlockIdentifier("canvasWfillClr");
    file->writeColor(m_canvas->fillColor());

    m_canvas->save(file);
}

void CanvasWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("canvasWLineSett", BLOCK_WIDGET))
    {
        m_lastLine.setX(file->readVal<int>());
        m_lastLine.setY(file->readVal<int>());

        m_canvas->lineWidth() = file->readVal<int>();
        m_canvas->lineColor() = QColor(file->readString());
    }

    if(file->seekToNextBlock("canvasWfillClr", BLOCK_WIDGET))
        m_canvas->fillColor() = file->readColor();

    m_canvas->load(file);
}

void CanvasWidget::setBackground(const QString &color)
{
    m_canvas->setBackground(QColor(color));
}

void CanvasWidget::drawLine(int x1, int y1, int x2, int y2)
{
    m_canvas->addLine(QLine(x1, y1, x2, y2));
    m_lastLine = QPoint(x2, y2);
}

void CanvasWidget::drawLine(int x, int y)
{
    m_canvas->addLine(QLine(m_lastLine, QPoint(x, y)));
    m_lastLine = QPoint(x, y);
}

void CanvasWidget::drawRect(int x, int y, int w, int h)
{
    m_canvas->addRect(QRect(x, y, w, h));
}

void CanvasWidget::drawEllipse(int x, int y, int w, int h)
{
    m_canvas->addCircle(QPoint(x+w/2, y+h/2), w/2, h/2);
}

void CanvasWidget::drawCircle(int x, int y, int r)
{
    m_canvas->addCircle(QPoint(x, y), r, r);
}

void CanvasWidget::setLineSize(int width)
{
    m_canvas->lineWidth() = width;
}

void CanvasWidget::setLineColor(const QString &color)
{
    m_canvas->lineColor() = QColor(color);
}

void CanvasWidget::setFillColor(const QString &color)
{
    m_canvas->fillColor() = QColor(color);
}

void CanvasWidget::clear()
{
    m_lastLine = QPoint();
    m_canvas->clear();
}

int CanvasWidget::getCanvasWidth() const
{
    return m_canvas->width();
}

int CanvasWidget::getCanvasHeight() const
{
    return m_canvas->height();
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

    m_lineWidth = 2;
    m_lineColor = Qt::black;
}

void Canvas::save(DataFileParser *file)
{
    file->writeBlockIdentifier("canvasWcanvas");
    file->writeString(palette().color(QPalette::Window).name());

    file->writeVal((quint32)m_lines.size());
    for(quint32 i = 0; i < m_lines.size(); ++i)
    {
        const line& l = m_lines[i];

        file->writeVal(l.l.x1());
        file->writeVal(l.l.y1());
        file->writeVal(l.l.x2());
        file->writeVal(l.l.y2());

        file->writeVal(l.width);
        file->writeString(l.color.name());
    }

    // offset
    file->writeBlockIdentifier("canvasWoffset");
    file->writeVal(m_offset.x());
    file->writeVal(m_offset.y());

    // Rectangles
    file->writeBlockIdentifier("canvasWrects");
    file->writeVal((quint32)m_rects.size());
    for(quint32 i = 0; i < m_rects.size(); ++i)
    {
        const rect& r = m_rects[i];

        int vals[4] = { r.r.x(), r.r.y(), r.r.width(), r.r.height() };
        qDebug("size %d", sizeof(vals));
        file->write((char*)vals, sizeof(vals));

        file->writeVal(r.width);
        file->writeColor(r.color);
        file->writeColor(r.fillColor);
    }

    // circles
    file->writeBlockIdentifier("canvasWcircles");
    file->writeVal((quint32)m_circles.size());
    for(quint32 i = 0; i < m_circles.size(); ++i)
    {
        const circle& c = m_circles[i];

        int vals[4] = { c.center.x(), c.center.y(), c.r1, c.r2 };
        file->write((char*)vals, sizeof(vals));

        file->writeVal(c.width);
        file->writeColor(c.color);
        file->writeColor(c.fillColor);
    }
}

void Canvas::load(DataFileParser *file)
{
    if(file->seekToNextBlock("canvasWcanvas", BLOCK_WIDGET))
    {
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
    }

    if(file->seekToNextBlock("canvasWoffset", BLOCK_WIDGET))
    {
        m_offset.setX(file->readVal<int>());
        m_offset.setY(file->readVal<int>());
    }

    if(file->seekToNextBlock("canvasWrects", BLOCK_WIDGET))
    {
        quint32 count = file->readVal<quint32>();
        int vals[4];
        for(quint32 i = 0; i < count; ++i)
        {
            rect r;
            qDebug("size %d", sizeof(vals));
            file->read((char*)vals, sizeof(vals));
            r.r = QRect(vals[0], vals[1], vals[2], vals[3]);

            r.width = file->readVal<int>();
            r.color = file->readColor();
            r.fillColor = file->readColor();

            m_rects.push_back(r);
        }
    }

    if(file->seekToNextBlock("canvasWcircles", BLOCK_WIDGET))
    {
        quint32 count = file->readVal<quint32>();
        int vals[4];
        for(quint32 i = 0; i < count; ++i)
        {
            circle c;

            file->read((char*)vals, sizeof(vals));
            c.center = QPoint(vals[0], vals[1]);
            c.r1 = vals[2];
            c.r2 = vals[3];

            c.width = file->readVal<int>();
            c.color = file->readColor();
            c.fillColor = file->readColor();

            m_circles.push_back(c);
        }
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

void Canvas::addLine(const QLine &l)
{
    line lin = {l, m_lineColor, m_lineWidth };
    m_lines.push_back(lin);

    update();
}

void Canvas::addRect(const QRect &r)
{
    rect rec = {r, m_lineColor, m_fillColor, m_lineWidth };
    m_rects.push_back(rec);

    update();
}

void Canvas::addCircle(const QPoint& center, int r1, int r2)
{
    circle cir = { center, r1, r2, m_lineColor, m_fillColor, m_lineWidth };
    m_circles.push_back(cir);

    update();
}

void Canvas::clear()
{
    m_lines.clear();
    m_rects.clear();
    m_circles.clear();

    m_offset = QPoint();
    m_fillColor = QColor();
    m_lineWidth = 2;
    m_lineColor = Qt::black;

    setBackground(Qt::white);

    update();
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.translate(m_offset);

    QPen pen;
    QBrush brush;

    for(quint32 i = 0; i < m_lines.size(); ++i)
    {
        const line& l = m_lines[i];

        pen.setColor(l.color);
        pen.setWidth(l.width);
        p.setPen(pen);

        p.drawLine(l.l);
    }

    for(quint32 i = 0; i < m_rects.size(); ++i)
    {
        const rect& r = m_rects[i];

        pen.setColor(r.color);
        pen.setWidth(r.width);
        p.setPen(pen);

        if(r.fillColor.isValid())
            brush = QBrush(r.fillColor);
        else
            brush = QBrush();
        p.setBrush(brush);

        p.drawRect(r.r);
    }

    for(quint32 i = 0; i < m_circles.size(); ++i)
    {
        const circle& c = m_circles[i];

        pen.setColor(c.color);
        pen.setWidth(c.width);
        p.setPen(pen);

        if(c.fillColor.isValid())
            brush = QBrush(c.fillColor);
        else
            brush = QBrush();
        p.setBrush(brush);

        p.drawEllipse(c.center, c.r1, c.r2);
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
    update();
}
