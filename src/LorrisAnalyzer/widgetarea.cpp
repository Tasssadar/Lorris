/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDropEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QVarLengthArray>
#include <QMenu>
#include <QInputDialog>

#include "widgetarea.h"
#include "lorrisanalyzer.h"
#include "../misc/datafileparser.h"
#include "storage.h"
#include "widgetfactory.h"

QPoint& operator %=(QPoint& a, const int& b)
{
    a.rx() %= b;
    a.ry() %= b;
    return a;
}

WidgetArea::WidgetArea(QWidget *parent) :
    QFrame(parent), m_menu(new QMenu(this))
{
    m_widgetIdCounter = 0;
    m_skipNextMove = false;
    m_prev = NULL;
    m_show_grid = sConfig.get(CFG_BOOL_ANALYZER_SHOW_GRID);
    m_grid = sConfig.get(CFG_BOOL_ANALYZER_ENABLE_GRID) ? sConfig.get(CFG_QUINT32_ANALYZER_GRID_SIZE) : 1;
    m_enablePlacementLines = sConfig.get(CFG_BOOL_ANALYZER_PLACEMENT_LINES);

    setCursor(Qt::OpenHandCursor);

    // update enum AreaMenuActions when changing these
    QAction *enableGrid = m_menu->addAction(tr("Enable grid"));
    QAction *showGrid = m_menu->addAction(tr("Show grid"));
    QAction *linesAct = m_menu->addAction(tr("Enable placement lines"));
    QAction *gridSize = m_menu->addAction(tr("Set grid size..."));
    QAction *align = m_menu->addAction(tr("Align widgets to the grid"));
    enableGrid->setCheckable(true);
    enableGrid->setChecked(m_grid != 1);
    showGrid->setCheckable(true);
    showGrid->setChecked(m_show_grid);
    linesAct->setCheckable(true);
    linesAct->setChecked(m_enablePlacementLines);

    Q_ASSERT(ACT_MAX == m_menu->actions().size());

    connect(enableGrid, SIGNAL(toggled(bool)),           SLOT(enableGrid(bool)));
    connect(enableGrid, SIGNAL(toggled(bool)), showGrid, SLOT(setEnabled(bool)));
    connect(enableGrid, SIGNAL(toggled(bool)), gridSize, SLOT(setEnabled(bool)));
    connect(showGrid,   SIGNAL(toggled(bool)),           SLOT(showGrid(bool)));
    connect(gridSize,   SIGNAL(triggered()),             SLOT(setGridSize()));
    connect(align,      SIGNAL(triggered()),             SLOT(alignWidgets()));
    connect(linesAct,   SIGNAL(toggled(bool)),           SLOT(enableLines(bool)));
}

WidgetArea::~WidgetArea()
{
    clear();
}

void WidgetArea::clear()
{
    w_map::iterator itr = m_widgets.begin();
    while(itr != m_widgets.end())
    {
        delete *itr;
        itr = m_widgets.erase(itr);
    }
    m_marks.clear();
}

void WidgetArea::dropEvent(QDropEvent *event)
{
    QString data = event->mimeData()->text().remove(0, 1);
    quint8 type = data.toInt();

    event->acceptProposedAction();

    DataWidget *w = addWidget(event->pos(), type);
    if(m_grid != 1)
        w->align();
}

DataWidget *WidgetArea::addWidget(QPoint pos, quint8 type, bool show)
{
    DataWidget *w = sWidgetFactory.getWidget(type, this);
    if(!w)
        return NULL;

    quint32 id = getNewId();
    w->setId(id);

    w->setUp(m_storage);

    w->move(pos);
    if(show)
        w->show();

    m_widgets.insert(id, w);

    connect(m_analyzer, SIGNAL(newData(analyzer_data*,quint32)), w, SLOT(newData(analyzer_data*,quint32)));
    connect(w,          SIGNAL(removeWidget(quint32)),              SLOT(removeWidget(quint32)));
    connect(w,          SIGNAL(updateMarker(DataWidget*)),          SLOT(updateMarker(DataWidget*)));
    connect(w,          SIGNAL(clearPlacementLines()),              SLOT(clearPlacementLines()));
    connect(w,          SIGNAL(updateForMe()),          m_analyzer, SLOT(updateForWidget()));
    connect(w,          SIGNAL(updateData()),                       SIGNAL(updateData()));
    connect(w,   SIGNAL(mouseStatus(bool,data_widget_info,qint32)), SIGNAL(mouseStatus(bool,data_widget_info,qint32)));
    connect(w,          SIGNAL(SendData(QByteArray)),   m_analyzer, SIGNAL(SendData(QByteArray)));
    connect(m_analyzer, SIGNAL(setTitleVisibility(bool)),        w, SLOT(setTitleVisibility(bool)));
    connect(w,  SIGNAL(addChildTab(ChildTab*,QString)), m_analyzer, SLOT(addChildTab(ChildTab*,QString)));
    connect(w,  SIGNAL(removeChildTab(ChildTab*)),      m_analyzer, SLOT(removeChildTab(ChildTab*)));

    //events
    connect(this,       SIGNAL(onWidgetAdd(DataWidget*)),        w, SLOT(onWidgetAdd(DataWidget*)));
    connect(this,       SIGNAL(onWidgetRemove(DataWidget*)),     w, SLOT(onWidgetRemove(DataWidget*)));
    connect(this,       SIGNAL(onScriptEvent(QString)),          w, SLOT(onScriptEvent(QString)));
    connect(w,          SIGNAL(scriptEvent(QString)),         this, SIGNAL(onScriptEvent(QString)));

    emit onWidgetAdd(w);

    w->setTitleVisibility(m_analyzer->showTitleBars());
    return w;
}

void WidgetArea::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->source() && event->mimeData()->hasText() && event->mimeData()->text().at(0) == 'w')
        event->acceptProposedAction();
    else
        QFrame::dragEnterEvent(event);
}

void WidgetArea::removeWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return;

    emit onWidgetRemove(*itr);

    delete *itr;
    m_widgets.erase(itr);

    m_marks.remove(id);
    update();
}

DataWidget *WidgetArea::getWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return NULL;
    return *itr;
}

void WidgetArea::SaveWidgets(DataFileParser *file)
{
    // We want widgets saved in same order as they were created. It does not have to be super-fast,
    // so I am using std::map to sort them.
    std::map<quint32, DataWidget*> widgets;
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        if((*itr)->getWidgetControlled() == -1)
            widgets[itr.key()] = *itr;

    // write widget count
    quint32 count = widgets.size();
    file->write((char*)&count, sizeof(quint32));

    for(std::map<quint32, DataWidget*>::iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
    {
        file->writeBlockIdentifier(BLOCK_WIDGET);
        itr->second->saveWidgetInfo(file);
    }
}

void WidgetArea::LoadWidgets(DataFileParser *file, bool skip)
{
    clear();

    quint32 count = 0;
    file->read((char*)&count, sizeof(quint32));

    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_WIDGET, 0))
            break;

        // type
        if(!file->seekToNextBlock("widgetType", BLOCK_WIDGET))
            break;

        quint8 type = 0;
        file->read((char*)&type, sizeof(quint8));

        // pos and size
        if(!file->seekToNextBlock("widgetPosSize", BLOCK_WIDGET))
            break;

        int val[4];
        file->read((char*)&val, sizeof(val));

        DataWidget *w = addWidget(QPoint(val[0], val[1]), type, !skip);
        if(!w)
            continue;

        w->resize(val[2], val[3]);
        w->loadWidgetInfo(file);

        if(skip)
            removeWidget(w->getId());
        else
            updateMarker(w);
    }
    update();
}

void WidgetArea::SaveSettings(DataFileParser *file)
{
    file->writeBlockIdentifier("areaGridSettings");
    file->write((char*)&m_grid, sizeof(m_grid));
    file->write((char*)&m_show_grid, sizeof(m_show_grid));

    file->writeBlockIdentifier("areaGridOffset");
    {
        int x = m_grid_offset.x();
        int y = m_grid_offset.y();
        file->write((char*)&x, sizeof(x));
        file->write((char*)&y, sizeof(y));
    }
}

void WidgetArea::LoadSettings(DataFileParser *file)
{
    if(file->seekToNextBlock("areaGridSettings", BLOCK_DATA_INDEX))
    {
        file->read((char*)&m_grid, sizeof(m_grid));
        file->read((char*)&m_show_grid, sizeof(m_show_grid));

        m_menu->actions()[ACT_ENABLE_GRID]->setChecked(m_grid != 1);
        m_menu->actions()[ACT_SHOW_GRID]->setChecked(m_show_grid);
    }

    if(file->seekToNextBlock("areaGridOffset", BLOCK_DATA_INDEX))
    {
        int x, y;
        file->read((char*)&x, sizeof(x));
        file->read((char*)&y, sizeof(y));
        m_grid_offset = QPoint(x, y);
        update();
    }
}

void WidgetArea::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
        case Qt::LeftButton:
            m_mouse_orig = event->globalPos();
            setCursor(Qt::ClosedHandCursor);
            m_prev = new WidgetAreaPreview(this, (QWidget*)parent());
            break;
        case Qt::RightButton:
            m_menu->exec(event->globalPos());
            break;
        default:
            break;
    }
}

void WidgetArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        delete m_prev;
        m_prev = NULL;
        setCursor(Qt::OpenHandCursor);
    }
}

void WidgetArea::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if(!m_show_grid && m_grid == 1 && m_marks.empty() && m_placementLines.isEmpty())
        return;

    QPainter painter(this);

    if(m_show_grid && m_grid > 1)
    {
        QVarLengthArray<QPoint, 8000> points((width()/m_grid)*(height()/m_grid));
        for(QPoint p; p.y() < height(); p.ry() += m_grid)
            for(p.rx() = 0; p.x() < width(); p.rx() += m_grid)
                points.append(p + m_grid_offset);

        painter.drawPoints(points.data(), points.size());
    }

    if(!m_marks.empty())
    {
        painter.setPen(Qt::red);
        painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

        for(mark_map::iterator itr = m_marks.begin(); itr != m_marks.end(); ++itr)
            painter.drawRect(*itr);
    }

    if(!m_placementLines.isEmpty())
    {
        painter.setPen(Qt::blue);
        painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        painter.drawLines(m_placementLines);
    }
}

void WidgetArea::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() != Qt::LeftButton)
        return;

    QPoint n = event->globalPos() - m_mouse_orig;
    moveWidgets(n);

    m_prev->prepareRender();
    m_mouse_orig = event->globalPos();
}

void WidgetArea::moveWidgets(QPoint diff)
{
    m_grid_offset += diff;
    m_grid_offset %= m_grid;

    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        QPoint pos = (*itr)->pos() + diff;
        (*itr)->move(pos);

        updateMarker(*itr);
    }
    update();
}

void WidgetArea::getMarkPos(int &x, int &y, QSize &size)
{
    size = (y < 0 || y > height()) ? QSize(20, 5) : QSize(5, 20);

    if     (x < 0)       x = 0;
    else if(x > width()) x = width() - 5;

    if     (y < 0)        y = 0;
    else if(y > height()) y = height() - 5;
}

void WidgetArea::updateMarker(DataWidget *w)
{
    bool do_update = false;

    if(!rect().intersects(w->geometry()))
    {
        QPoint markPos(w->pos().x(), w->pos().y());
        markPos.rx() += w->width()/2;
        markPos.ry() += w->height()/2;

        QSize size;
        getMarkPos(markPos.rx(), markPos.ry(), size);

        m_marks[w->getId()] = QRect(markPos, size);

        do_update = true;

        w->setUpdating(false);
    }
    else
    {
        do_update = m_marks.remove(w->getId());
        if(do_update)
        {
            w->setUpdating(true);

            quint32 idx = 0;
            analyzer_data *data = m_analyzer->getLastData(idx);
            if(data)
                w->newData(data, idx);
        }
    }

    if(do_update)
        update();
}

void WidgetArea::moveEvent(QMoveEvent *event)
{
    if(m_skipNextMove)
        m_skipNextMove = false;
    else
        moveWidgets(event->oldPos() - pos());
}

void WidgetArea::resizeEvent(QResizeEvent *)
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        updateMarker(*itr);
}

void WidgetArea::enableGrid(bool enable)
{
    if(enable) m_grid = sConfig.get(CFG_QUINT32_ANALYZER_GRID_SIZE);
    else       m_grid = 1;

    sConfig.set(CFG_BOOL_ANALYZER_ENABLE_GRID, enable);

    update();
}

void WidgetArea::showGrid(bool show)
{
    m_show_grid = show;
    sConfig.set(CFG_BOOL_ANALYZER_SHOW_GRID, show);

    update();
}

void WidgetArea::setGridSize()
{
    m_grid = QInputDialog::getInt(this, tr("Grid size"), tr("Enter grid size in pixels"), m_grid, 2);
    sConfig.set(CFG_QUINT32_ANALYZER_GRID_SIZE, m_grid);
    update();
}

void WidgetArea::alignWidgets()
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        (*itr)->align();
}

QRegion WidgetArea::getRegionWithWidgets()
{
    QPoint p;
    QSize s = size();
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        DataWidget *w = *itr;
        p.rx() = std::min(w->x(), p.x());
        p.ry() = std::min(w->y(), p.y());

        s.rwidth() = std::max(s.width(), w->x() + w->width());
        s.rheight() = std::max(s.height(), w->y() + w->height());
    }
    s.rwidth() += abs(p.x());
    s.rheight() += abs(p.y());
    return QRegion(QRect(p, s));
}

void WidgetArea::correctWidgetName(QString &name, DataWidget *widget)
{
    int add = 1;
    QString original = name;
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        if(widget != *itr && name == (*itr)->getTitle())
        {
            name = original + QString("_%1").arg(add++);
            itr = m_widgets.begin();
        }
    }
}

void WidgetArea::updatePlacement(int x, int y, int w, int h, DataWidget *widget)
{
    if(!m_enablePlacementLines)
        return;

    int wx, wy;
    int sx = x + w;
    int sy = y + h;
    bool changed = false;

    for(QVector<QLine>::iterator itr = m_placementLines.begin(); itr != m_placementLines.end();)
    {
        wx = (*itr).x1();
        wy = (*itr).y1();

        if(abs(wx - x) > PLACEMENT_SHOW && abs(wx - sx) > PLACEMENT_SHOW &&
           abs(wy - y) > PLACEMENT_SHOW && abs(wy - sy) > PLACEMENT_SHOW)
        {
            itr = m_placementLines.erase(itr);
            changed = true;
            continue;
        }
        ++itr;
    }

    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        if(*itr == widget)
            continue;

        wx = (*itr)->x();
        wy = (*itr)->y();

        if(abs(wx - x) < PLACEMENT_SHOW || abs(wx - sx) < PLACEMENT_SHOW)
            addPlacementLine(wx, true, changed);

        if(abs(wy - y) < PLACEMENT_SHOW || abs(wy - sy) < PLACEMENT_SHOW)
            addPlacementLine(wy, false, changed);

        wx += (*itr)->width();
        wy += (*itr)->height();

        if(abs(wx - x) < PLACEMENT_SHOW || abs(wx - sx) < PLACEMENT_SHOW)
            addPlacementLine(wx, true, changed);

        if(abs(wy - y) < PLACEMENT_SHOW || abs(wy - sy) < PLACEMENT_SHOW)
            addPlacementLine(wy, false, changed);
    }

    if(changed)
        update();
}

void WidgetArea::addPlacementLine(int val, bool vertical, bool& changed)
{
    QLine l;
    if(vertical)
        l.setLine(val, -PLACEMENT_SHOW, val, height()+PLACEMENT_SHOW);
    else
        l.setLine(-PLACEMENT_SHOW, val, width()+PLACEMENT_SHOW, val);

    if(m_placementLines.contains(l))
        return;

    m_placementLines.push_back(l);
    changed = true;
}

void WidgetArea::clearPlacementLines()
{
    m_placementLines.clear();
    update();
}

void WidgetArea::enableLines(bool enable)
{
    sConfig.set(CFG_BOOL_ANALYZER_PLACEMENT_LINES, enable);
    m_enablePlacementLines = enable;
}

WidgetAreaPreview::WidgetAreaPreview(WidgetArea *area, QWidget *parent) : QWidget(parent)
{
    m_widgetArea = area;
    m_smooth = sConfig.get(CFG_BOOL_SMOOTH_SCALING);

    move(area->pos());
    resize(area->width()/3, area->height()/3);
    prepareRender();
    show();
}

void WidgetAreaPreview::prepareRender()
{
    m_region = m_widgetArea->getRegionWithWidgets();
    m_render = QPixmap(m_region.boundingRect().size());
    m_widgetArea->render(&m_render, QPoint(), m_region);
    m_render = m_render.scaled(size(), Qt::KeepAspectRatio,
                       m_smooth ? Qt::SmoothTransformation : Qt::FastTransformation);

    // TODO: do not update region size?
    //if(m_visible.isNull())
    {
        float scale = float(m_render.width())/m_region.boundingRect().width();
        m_visible = m_widgetArea->rect();
        m_visible.setX(abs(m_region.boundingRect().x())*scale);
        m_visible.setY(abs(m_region.boundingRect().y())*scale);
        m_visible.setWidth(m_widgetArea->width()*scale);
        m_visible.setHeight(m_widgetArea->height()*scale);
    }
    update();
}

void WidgetAreaPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBackgroundMode(Qt::TransparentMode);
    p.setBackground(QBrush(Qt::transparent));
    p.setOpacity(0.8);

    p.drawPixmap(QPoint(), m_render);

    QPen pen(Qt::black);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawRect(m_visible);
}
