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
#include <QApplication>
#include <QShortcut>
#include <QDialog>
#include <QDialogButtonBox>

#include "widgetarea.h"
#include "lorrisanalyzer.h"
#include "../misc/datafileparser.h"
#include "storage.h"
#include "widgetfactory.h"
#include "../ui/shortcutinputbox.h"

QPoint& operator %=(QPoint& a, const int& b)
{
    a.rx() %= b;
    a.ry() %= b;
    return a;
}

WidgetArea::WidgetArea(QWidget *parent) :
    QFrame(parent), m_menu(new QMenu(this)), m_undoStack(this)
{
    m_widgetIdCounter = 0;
    m_skipNextMove = false;
    m_prev = NULL;
    m_show_grid = sConfig.get(CFG_BOOL_ANALYZER_SHOW_GRID);
    m_grid = sConfig.get(CFG_BOOL_ANALYZER_ENABLE_GRID) ? sConfig.get(CFG_QUINT32_ANALYZER_GRID_SIZE) : 1;
    m_enablePlacementLines = sConfig.get(CFG_BOOL_ANALYZER_PLACEMENT_LINES);
    m_active_bookmk = NULL;
    m_bookmk_ids = 0;
    m_show_bookmk = sConfig.get(CFG_BOOL_ANALYZER_SHOW_BOOKMARKS);

    setCursor(Qt::OpenHandCursor);
    m_draggin = false;

    QAction *addPoint = m_menu->addAction(tr("Add bookmark..."));
    m_actShowBookmk = m_menu->addAction(tr("Show bookmarks"));
    m_actShowBookmk->setCheckable(true);
    m_actShowBookmk->setChecked(m_show_bookmk);

    m_menu->addSeparator();

    QMenu *gridMenu = m_menu->addMenu(tr("Grid"));

    m_actEnableGrid = gridMenu->addAction(tr("Enable grid"));
    m_actShowGrid = gridMenu->addAction(tr("Show grid"));
    QAction *gridSize = gridMenu->addAction(tr("Set grid size..."));
    QAction *align = gridMenu->addAction(tr("Align widgets to the grid"));
    m_actEnableGrid->setCheckable(true);
    m_actEnableGrid->setChecked(m_grid != 1);
    m_actShowGrid->setCheckable(true);
    m_actShowGrid->setChecked(m_show_grid);

    QAction *linesAct = m_menu->addAction(tr("Enable placement lines"));
    linesAct->setCheckable(true);
    linesAct->setChecked(m_enablePlacementLines);

    m_menu->addSeparator();

    m_titleVisibility = m_menu->addAction(tr("Show widget's title bar"));
    m_showPreview = m_menu->addAction(tr("Show preview while moving the area"));
    QAction *lockAll = m_menu->addAction(tr("Lock all widgets"));
    QAction *unlockAll = m_menu->addAction(tr("Unlock all widgets"));

    m_titleVisibility->setCheckable(true);
    m_titleVisibility->setChecked(true);
    m_showPreview->setCheckable(true);
    m_showPreview->setChecked(sConfig.get(CFG_BOOL_ANALYZER_SHOW_PREVIEW));

    m_menu->addSeparator();

    QAction *undo = m_menu->addAction(tr("Undo"), &m_undoStack, SLOT(undo()), QKeySequence("Ctrl+Z"));
    QAction *redo = m_menu->addAction(tr("Redo"), &m_undoStack, SLOT(redo()), QKeySequence("Shift+Ctrl+Z"));
    undo->setEnabled(false);
    redo->setEnabled(false);
    addAction(undo);
    addAction(redo);

    m_bookmk_menu = new QMenu(this);
    QAction *keyseqAct = m_bookmk_menu->addAction(tr("Change shortcut"));
    QAction *rmPntAct = m_bookmk_menu->addAction(tr("Remove"));

    connect(m_actEnableGrid, SIGNAL(toggled(bool)),                SLOT(enableGrid(bool)));
    connect(m_actEnableGrid, SIGNAL(toggled(bool)), m_actShowGrid, SLOT(setEnabled(bool)));
    connect(m_actEnableGrid, SIGNAL(toggled(bool)), gridSize,      SLOT(setEnabled(bool)));
    connect(m_actShowGrid,   SIGNAL(toggled(bool)),                SLOT(showGrid(bool)));
    connect(gridSize,        SIGNAL(triggered()),                  SLOT(setGridSize()));
    connect(align,           SIGNAL(triggered()),                  SLOT(alignWidgets()));
    connect(linesAct,        SIGNAL(toggled(bool)),                SLOT(enableLines(bool)));
    connect(m_titleVisibility, SIGNAL(triggered(bool)),            SLOT(titleVisibilityAct(bool)));
    connect(m_showPreview,   SIGNAL(toggled(bool)),                SLOT(setShowPreview(bool)));
    connect(lockAll,         SIGNAL(triggered()),                  SLOT(lockAll()));
    connect(unlockAll,       SIGNAL(triggered()),                  SLOT(unlockAll()));
    connect(&m_undoStack,    SIGNAL(undoAvailable(bool)),    undo, SLOT(setEnabled(bool)));
    connect(&m_undoStack,    SIGNAL(redoAvailable(bool)),    redo, SLOT(setEnabled(bool)));
    connect(&m_bookmk_mapper,SIGNAL(mapped(int)),                  SLOT(jumpToBookmark(int)));
    connect(keyseqAct,       SIGNAL(triggered()),                  SLOT(changeBookmarkSeq()));
    connect(rmPntAct,        SIGNAL(triggered()),                  SLOT(removeBookmark()));
    connect(addPoint,        SIGNAL(triggered()),                  SLOT(addBookmark()));
    connect(m_actShowBookmk, SIGNAL(toggled(bool)),                SLOT(setShowBookmarks(bool)));
}

WidgetArea::~WidgetArea()
{
    clear();
}

void WidgetArea::clear()
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        delete *itr;
    m_widgets.clear();

    m_marks.clear();
    m_undoStack.clear();

    for(size_t i = 0; i < m_bookmarks.size(); ++i)
        delete m_bookmarks[i].shortcut;
    m_bookmarks.clear();
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

    connect(w,          SIGNAL(removeWidget(quint32)),              SLOT(removeWidget(quint32)));
    connect(w,          SIGNAL(updateMarker(DataWidget*)),          SLOT(updateMarker(DataWidget*)));
    connect(w,          SIGNAL(clearPlacementLines()),              SLOT(clearPlacementLines()));
    connect(w,          SIGNAL(updateData()),                       SIGNAL(updateData()));
    connect(w,   SIGNAL(mouseStatus(bool,data_widget_info,qint32)), SIGNAL(mouseStatus(bool,data_widget_info,qint32)));
    connect(w,          SIGNAL(SendData(QByteArray)),   m_analyzer, SIGNAL(SendData(QByteArray)));
    connect(w,          SIGNAL(toggleSelection(bool)),              SLOT(toggleSelection(bool)));
    connect(this,       SIGNAL(setTitleVisibility(bool)),        w, SLOT(setTitleVisibility(bool)));
    connect(w,  SIGNAL(addChildTab(ChildTab*,QString)), m_analyzer, SLOT(addChildTab(ChildTab*,QString)));
    connect(w,  SIGNAL(removeChildTab(ChildTab*)),      m_analyzer, SLOT(removeChildTab(ChildTab*)));
    connect(w,          SIGNAL(addUndoAct(UndoAction*)),&m_undoStack,SLOT(addAction(UndoAction*)));
    connect(m_analyzer, SIGNAL(rawData(QByteArray)),             w, SIGNAL(rawData(QByteArray)));
    connect(this,       SIGNAL(setLocked(bool)),                 w, SLOT(setLocked(bool)));

    //events
    connect(this,       SIGNAL(onWidgetAdd(DataWidget*)),        w, SLOT(onWidgetAdd(DataWidget*)));
    connect(this,       SIGNAL(onWidgetRemove(DataWidget*)),     w, SLOT(onWidgetRemove(DataWidget*)));
    connect(this,       SIGNAL(onScriptEvent(QString)),          w, SLOT(onScriptEvent(QString)));
    connect(w,          SIGNAL(scriptEvent(QString)),         this, SIGNAL(onScriptEvent(QString)));

    emit onWidgetAdd(w);

    m_analyzer->setDataChanged();

    w->setTitleVisibility(m_titleVisibility->isChecked());
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

    m_undoStack.addAction(new RestoreAction(*itr));

    emit onWidgetRemove(*itr);

    delete *itr;
    m_widgets.erase(itr);

    m_marks.remove(id);
    update();

    m_undoStack.checkValid();
}

DataWidget *WidgetArea::getWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return NULL;
    return *itr;
}

void WidgetArea::saveWidgets(DataFileParser *file)
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

void WidgetArea::loadWidgets(DataFileParser *file, bool skip)
{
    clear();

    quint32 count = 0;
    file->read((char*)&count, sizeof(quint32));

    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_WIDGET, 0))
            break;

        if(!loadOneWidget(file, skip))
            break;
    }
    update();
}

DataWidget* WidgetArea::loadOneWidget(DataFileParser *file, bool skip)
{
    // type
    if(!file->seekToNextBlock("widgetType", BLOCK_WIDGET))
        return NULL;

    quint8 type = 0;
    file->read((char*)&type, sizeof(quint8));

    // pos and size
    if(!file->seekToNextBlock("widgetPosSize", BLOCK_WIDGET))
        return NULL;

    int val[4];
    file->read((char*)&val, sizeof(val));

    DataWidget *w = addWidget(QPoint(val[0], val[1]), type, !skip);
    if(!w)
        return NULL;

    w->resize(val[2], val[3]);
    w->loadWidgetInfo(file);

    if(skip)
        removeWidget(w->getId());
    else
        updateMarker(w);
    return w;
}

void WidgetArea::saveSettings(DataFileParser *file)
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

    file->writeBlockIdentifier("areaBookmark");
    file->writeVal(m_show_bookmk);
    file->writeVal((quint32)m_bookmarks.size());
    for(size_t i = 0; i < m_bookmarks.size(); ++i)
    {
        const area_bookmark& b = m_bookmarks[i];
        file->writeVal(b.main.x());
        file->writeVal(b.main.y());

        file->writeVal(b.text.x());
        file->writeVal(b.text.y());

        file->writeString(b.keyseq);
    }
}

void WidgetArea::loadSettings(DataFileParser *file)
{
    if(file->seekToNextBlock("areaGridSettings", BLOCK_DATA_INDEX))
    {
        m_grid = file->readVal<quint32>();
        m_show_grid = file->readVal<bool>();

        m_actEnableGrid->setChecked(m_grid != 1);
        m_actShowGrid->setChecked(m_show_grid);
    }

    if(file->seekToNextBlock("areaGridOffset", BLOCK_DATA_INDEX))
    {
        int x = file->readVal<int>();
        int y = file->readVal<int>();

        m_grid_offset = QPoint(x, y);
        update();
    }

    if(file->seekToNextBlock("areaBookmark", BLOCK_DATA_INDEX))
    {
        m_show_bookmk = file->readVal<bool>();
        quint32 cnt = file->readVal<quint32>();
        for(quint32 i = 0; i < cnt; ++i)
        {
            area_bookmark b;
            b.id = m_bookmk_ids++;

            b.main.rx() = file->readVal<int>();
            b.main.ry() = file->readVal<int>();

            b.text.rx() = file->readVal<int>();
            b.text.ry() = file->readVal<int>();

            b.keyseq = file->readString();

            b.shortcut = new QShortcut(QKeySequence(b.keyseq), this);
            m_bookmk_mapper.setMapping(b.shortcut, b.id);
            connect(b.shortcut, SIGNAL(activated()),            &m_bookmk_mapper, SLOT(map()));
            connect(b.shortcut, SIGNAL(activatedAmbiguously()), &m_bookmk_mapper, SLOT(map()));

            m_bookmarks.push_back(b);
        }
    }
}

void WidgetArea::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if (!m_show_grid && m_grid == 1 && !m_show_bookmk && m_marks.empty() &&
        m_placementLines.isEmpty() && m_bookmarks.empty())
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

    if(m_show_bookmk && !m_bookmarks.empty())
    {
        QPen penBlack(Qt::black);
        QPen penYellow(QColor("#FFFFA3"));
        penYellow.setWidth(2);

        QBrush noBrush(Qt::NoBrush);
        QBrush fill(penYellow.color(), Qt::SolidPattern);

        QRect r = rect();
        r.setWidth(r.width() - penYellow.width()*2);
        r.setHeight(r.height() - penYellow.width()*2);

        QFont f = painter.font();
        f.setBold(true);
        painter.setFont(f);

        for(size_t i = 0; i < m_bookmarks.size(); ++i)
        {
            area_bookmark& p = m_bookmarks[i];

            if(p.text.x() == -1)
            {
                p.text.rx() = painter.fontMetrics().width(p.keyseq)*1.5;
                p.text.ry() = painter.fontMetrics().height();
            }

            QPoint base(p.main.x()+penYellow.width(), p.main.y()+penYellow.width());

            painter.setPen(penYellow);
            painter.setBrush(noBrush);
            painter.drawRect(base.x(), base.y(), r.width(), r.height());

            painter.setBrush(fill);
            painter.drawRect(base.x(), base.y(), p.text.x(), p.text.y());

            painter.setPen(penBlack);
            painter.drawText(base.x(), base.y(), p.text.x(), p.text.y(), Qt::AlignCenter, p.keyseq);
        }
    }
}

void WidgetArea::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
        return;

    m_active_bookmk = NULL;
    for(size_t i = 0; !m_active_bookmk && i < m_bookmarks.size(); ++i)
    {
        area_bookmark& pnt = m_bookmarks[i];
        if(!Utils::isInRect(event->pos(), pnt.main, pnt.text))
            continue;

        m_active_bookmk = &pnt;
    }

    switch(event->button())
    {
        case Qt::LeftButton:
            m_mouse_orig = event->globalPos();
            setCursor(Qt::ClosedHandCursor);
            m_draggin = false;
            // drag type is selected by m_active_bookmk
            break;
        case Qt::RightButton:
            if(m_active_bookmk)
                m_bookmk_menu->exec(event->globalPos());
            else
                m_menu->exec(event->globalPos());
            break;
        default:
            break;
    }
}

void WidgetArea::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() != Qt::LeftButton)
        return;

    if(!m_draggin)
    {
        if ((event->globalPos() - m_mouse_orig).manhattanLength() < QApplication::startDragDistance())
              return;
        m_draggin = true;
    }

    QPoint n = event->globalPos() - m_mouse_orig;

    if(!m_active_bookmk)
    {
        moveWidgets(n);

        if(m_prev)
            m_prev->prepareRender();
        else if(m_showPreview->isChecked())
            m_prev = new WidgetAreaPreview(this, (QWidget*)parent());
    }
    else
    {
        m_active_bookmk->main += n;
        update();
    }

    m_mouse_orig = event->globalPos();
}

void WidgetArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(!m_draggin)
            clearSelection();
        m_draggin = false;
        delete m_prev;
        m_prev = NULL;
        m_active_bookmk = NULL;
        setCursor(Qt::OpenHandCursor);
    }
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

    for(size_t i = 0; i < m_bookmarks.size(); ++i)
        m_bookmarks[i].main += diff;

    m_undoStack.areaMoved(diff);

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
            w->updateForThis();
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

    static const QRegExp regexp(".*_\\d*");
    if(regexp.exactMatch(original))
        original = original.left(original.lastIndexOf('_'));

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

void WidgetArea::titleVisibilityAct(bool toggled)
{
    m_titleVisibility->setChecked(toggled);
    emit setTitleVisibility(toggled);
}

void WidgetArea::lockAll()
{
    emit setLocked(true);
}

void WidgetArea::unlockAll()
{
    emit setLocked(false);
}

void WidgetArea::toggleSelection(bool select)
{
    Q_ASSERT(sender() && sender()->inherits("DataWidget"));

    DataWidget *w = (DataWidget*)sender();
    if(select)
        m_selected.insert(w);
    else
        m_selected.erase(w);
}

void WidgetArea::clearSelection()
{
    std::set<DataWidget*> sel;
    m_selected.swap(sel);

    for(std::set<DataWidget*>::iterator itr = sel.begin(); itr != sel.end(); ++itr)
        (*itr)->setSelected(false);
}

void WidgetArea::setShowPreview(bool show)
{
    sConfig.set(CFG_BOOL_ANALYZER_SHOW_PREVIEW, show);
}

DataFilter *WidgetArea::getFilter(quint32 id) const
{
    return m_analyzer->getFilter(id);
}

DataFilter *WidgetArea::getFilterByOldInfo(const data_widget_infoV1 &old_info) const
{
    return m_analyzer->getFilterByOldInfo(old_info);
}

void WidgetArea::wheelEvent(QWheelEvent *ev)
{
    // Move area only if the mouse is not in some of the widgets
    QPoint pos = mapFromGlobal(QCursor::pos());
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        if((*itr)->geometry().contains(pos))
            return;

    int move = qApp->wheelScrollLines()*10*(float(ev->delta())/120);
    switch(ev->orientation())
    {
        case Qt::Vertical:
            moveWidgets(QPoint(0, move));
            break;
        case Qt::Horizontal:
            moveWidgets(QPoint(move, 0));
            break;
    }
}

void WidgetArea::jumpToBookmark(int id)
{
    for(size_t i = 0; i < m_bookmarks.size(); ++i)
    {
        const area_bookmark& b = m_bookmarks[i];
        if(b.id != id)
            continue;

        BookmarkMoveAnimation *anim = new BookmarkMoveAnimation(this);
        anim->setStartValue(QPoint(0, 0));
        anim->setEndValue(QPoint(0, 0) - b.main);
        anim->setDuration(100);
        anim->start();

        break;
    }
}

void WidgetArea::removeBookmark()
{
    if(!m_active_bookmk)
        return;

    for(std::vector<area_bookmark>::iterator itr = m_bookmarks.begin(); itr != m_bookmarks.end(); ++itr)
    {
        if((*itr).id != m_active_bookmk->id)
            continue;

        delete m_active_bookmk->shortcut;

        m_bookmarks.erase(itr);
        update();
        m_active_bookmk = NULL;
        break;
    }
}

void WidgetArea::changeBookmarkSeq()
{
    if(!m_active_bookmk)
        return;

    QDialog d(this);
    d.setWindowFlags(d.windowFlags() & ~(Qt::WindowMaximizeButtonHint | Qt::WindowContextHelpButtonHint));
    d.setWindowTitle(tr("Set bookmark shortcut"));

    QVBoxLayout *l = new QVBoxLayout(&d);

    ShortcutInputBox *box = new ShortcutInputBox(QKeySequence(m_active_bookmk->keyseq), &d);
    QDialogButtonBox *btn = new QDialogButtonBox((QDialogButtonBox::Ok |QDialogButtonBox::Cancel), Qt::Horizontal, &d);

    l->addWidget(box);
    l->addWidget(btn);

    connect(btn, SIGNAL(accepted()), &d, SLOT(accept()));
    connect(btn, SIGNAL(rejected()), &d, SLOT(reject()));

    if(d.exec() == QDialog::Accepted)
    {
        QKeySequence s = box->getKeySequence();
        m_active_bookmk->shortcut->setKey(s);
        m_active_bookmk->keyseq = s.toString(QKeySequence::NativeText);
        m_active_bookmk->text.rx() = -1;
        update();
    }

    m_active_bookmk = NULL;
}

void WidgetArea::addBookmark()
{
    area_bookmark b;
    b.id = m_bookmk_ids++;
    b.text.rx() = -1;
    b.keyseq = "(None)";

    b.shortcut = new QShortcut(this);
    m_bookmk_mapper.setMapping(b.shortcut, b.id);
    connect(b.shortcut, SIGNAL(activated()),            &m_bookmk_mapper, SLOT(map()));
    connect(b.shortcut, SIGNAL(activatedAmbiguously()), &m_bookmk_mapper, SLOT(map()));

    std::vector<area_bookmark>::iterator itr = m_bookmarks.insert(m_bookmarks.end(), b);
    m_active_bookmk = &(*itr);

    changeBookmarkSeq();

    update();
}

void WidgetArea::setShowBookmarks(bool show)
{
    m_show_bookmk = show;
    sConfig.set(CFG_BOOL_ANALYZER_SHOW_BOOKMARKS, m_show_bookmk);
    update();
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

BookmarkMoveAnimation::BookmarkMoveAnimation(WidgetArea *area) : QVariantAnimation(area)
{
    m_area = area;
}

void BookmarkMoveAnimation::setStartValue(const QPoint &value)
{
    m_last_point = value;
    QVariantAnimation::setStartValue(value);
}

void BookmarkMoveAnimation::updateCurrentValue(const QVariant& value)
{
    QPoint cur = value.value<QPoint>();

    QPoint diff = cur - m_last_point;
    m_area->moveWidgets(diff);

    m_last_point = cur;
}
