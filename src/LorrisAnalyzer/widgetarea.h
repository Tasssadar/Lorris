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
#include <QSignalMapper>
#include <QVariantAnimation>

#include "DataWidgets/datawidget.h"
#include "undostack.h"

class DataFileParser;
class Storage;
class LorrisAnalyzer;
class ChildTab;
class WidgetAreaPreview;
class QShortcut;

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
    void onScriptEvent(const QString &eventId, const QVariantList& args);

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

    void saveWidgets(DataFileParser *file);
    void loadWidgets(DataFileParser *file, bool skip);
    DataWidget *loadOneWidget(DataFileParser *file, bool skip = false);
    void saveSettings(DataFileParser *file);
    void loadSettings(DataFileParser *file);

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

    void setLastSearch(const QString& str) { m_lastSearch = str; }
    QString getLastSearch() const { return m_lastSearch; }

public slots:
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    void addBookmark();
    void lockAll();
    void unlockAll();
    void toggleGrid() { showGrid(!m_show_grid); }
    void toggleBookmarks();
    void alignWidgets();
    void togglePreview();
    void toggleWidgetTitles();
    
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
    void keyPressEvent(QKeyEvent *k);
    void keyReleaseEvent(QKeyEvent *k);

private slots:
    void enableGrid(bool enable);
    void showGrid(bool show);
    void setGridSize();
    void clearPlacementLines();
    void enableLines(bool enable);
    void titleVisibilityAct(bool toggled);
    void toggleSelection(bool select);
    void clearSelection();
    void setShowPreview(bool show);
    void jumpToBookmark(int id);
    void removeBookmark();
    void changeBookmarkSeq();
    void setShowBookmarks(bool show);
    void enableSearchClicked(bool enable);
    void enableSearchToggled(bool enable);

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
    QAction *m_actShowBookmk;
    QAction *m_actEnableGrid;
    QAction *m_actShowGrid;
    QAction *m_actTitleVisibility;
    QAction *m_actShowPreview;
    QAction *m_actEnableSearch;

    WidgetAreaPreview *m_prev;

    QVector<QLine> m_placementLines;
    bool m_enablePlacementLines;

    UndoStack m_undoStack;
    std::set<DataWidget*> m_selected;

    struct area_bookmark
    {
        int id;
        QPoint main;
        QPoint text;
        QString keyseq;
        QShortcut *shortcut;
    };

    std::vector<area_bookmark> m_bookmarks;
    QSignalMapper m_bookmk_mapper;
    area_bookmark *m_active_bookmk;
    QMenu *m_bookmk_menu;
    int m_bookmk_ids;
    bool m_show_bookmk;

    QString m_lastSearch;
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

class BookmarkMoveAnimation : public QVariantAnimation
{
    Q_OBJECT
public:
    BookmarkMoveAnimation(WidgetArea *area);

    void setStartValue(const QPoint &value);

protected:
    void updateCurrentValue(const QVariant& value);

private:
    WidgetArea *m_area;
    QPoint m_last_point;
};

#endif // WIDGETAREA_H
