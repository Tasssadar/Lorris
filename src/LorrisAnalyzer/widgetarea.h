/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WIDGETAREA_H
#define WIDGETAREA_H

#include <QFrame>
#include <QHash>
#include <set>

#include "DataWidgets/datawidget.h"
#include "undostack.h"

class DataFileParser;
class Storage;
class LorrisAnalyzer;
class ChildTab;
class WidgetAreaPreview;

#define PLACEMENT_SHOW 20
#define PLACEMENT_STICK 7

class WidgetArea : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);
    void setTitleVisibility(bool visible);
    void setLocked(bool locked);

    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void onScriptEvent(const QString& eventId);

public:
    typedef QHash<quint32, DataWidget*> w_map;
    typedef QHash<quint32, QRect> mark_map;

    explicit WidgetArea(QWidget *parent = 0);
    ~WidgetArea();

    void setAnalyzerAndStorage(LorrisAnalyzer* analyzer, Storage* storage)
    {
        m_analyzer = analyzer;
        m_storage = storage;
    }

    void clear();

    void SaveWidgets(DataFileParser *file);
    void LoadWidgets(DataFileParser *file, bool skip);
    DataWidget *LoadOneWidget(DataFileParser *file, bool skip = false);
    void SaveSettings(DataFileParser *file);
    void LoadSettings(DataFileParser *file);

    DataWidget *addWidget(QPoint pos, quint8 type, bool show = true);
    void moveWidgets(QPoint diff);
    void correctWidgetName(QString& name, DataWidget *widget);

    void skipNextMove() { m_skipNextMove = true; }

    DataWidget *getWidget(quint32 id);
    const w_map& getWidgets() const { return m_widgets; }

    void setGridOffset(int x, int y) { m_grid_offset = QPoint(x, y); }
    const QPoint& getGridOffset() const { return m_grid_offset; }
    quint32 getGrid() const { return m_grid; }
    void setGrid(quint32 grid) { m_grid = grid; }

    Storage *getStorage() const { return m_storage; }

    QRegion getRegionWithWidgets();

    void updatePlacement(int x, int y, int w, int h, DataWidget *widget);
    const QVector<QLine>& getPlacementLines() const { return m_placementLines; }

    const std::set<DataWidget*>& getSelected() const { return m_selected; }

    DataFilter *getFilter(quint32 id) const;
    DataFilter *getFilterByOldInfo(const data_widget_infoV1& old_info) const;

public slots:
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dropEvent ( QDropEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );
    void paintEvent(QPaintEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *ev);

private slots:
    void enableGrid(bool enable);
    void showGrid(bool show);
    void setGridSize();
    void alignWidgets();
    void clearPlacementLines();
    void enableLines(bool enable);
    void lockAll();
    void unlockAll();
    void titleVisibilityAct(bool toggled);
    void toggleSelection(bool select);
    void clearSelection();
    void setShowPreview(bool show);

private:
    void getMarkPos(int &x, int &y, QSize &size);
    quint32 getNewId() { return m_widgetIdCounter++; }
    void addPlacementLine(int val, bool vertical, bool &changed);

    w_map m_widgets;
    mark_map m_marks;
    quint32 m_widgetIdCounter;
    Storage *m_storage;
    LorrisAnalyzer* m_analyzer;

    QPoint m_mouse_orig;
    bool m_draggin;

    bool m_skipNextMove;
    QPoint m_grid_offset;
    quint32 m_grid;
    bool m_show_grid;

    QMenu *m_menu;
    QAction *m_actEnableGrid;
    QAction *m_actShowGrid;
    QAction *m_titleVisibility;
    QAction *m_showPreview;
    WidgetAreaPreview *m_prev;

    QVector<QLine> m_placementLines;
    bool m_enablePlacementLines;

    UndoStack m_undoStack;
    std::set<DataWidget*> m_selected;
};

class WidgetAreaPreview : public QWidget
{
    Q_OBJECT
public:
    WidgetAreaPreview(WidgetArea *area, QWidget *parent);

    void prepareRender();

protected:
    void paintEvent(QPaintEvent *);

private:
    WidgetArea *m_widgetArea;
    QPixmap m_render;
    QRegion m_region;
    QRect m_visible;
    bool m_smooth;
};

#endif // WIDGETAREA_H
