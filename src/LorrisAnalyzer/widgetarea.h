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

#include "DataWidgets/datawidget.h"

class DataFileParser;
class Storage;
class LorrisAnalyzer;

class WidgetArea : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);

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
    void SaveSettings(DataFileParser *file);
    void LoadSettings(DataFileParser *file);

    DataWidget *addWidget(QPoint pos, quint8 type, bool show = true);
    void moveWidgets(QPoint diff);

    void skipNextMove() { m_skipNextMove = true; }

    DataWidget *getWidget(quint32 id);
    const w_map& getWidgets() const { return m_widgets; }

    void setGridOffset(int x, int y) { m_grid_offset = QPoint(x, y); }
    const QPoint& getGridOffset() const { return m_grid_offset; }
    quint32 getGrid() const { return m_grid; }
    void setGrid(quint32 grid) { m_grid = grid; }

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

private slots:
    void enableGrid(bool enable);
    void showGrid(bool show);
    void setGridSize();
    void alignWidgets();

private:
    void getMarkPos(int &x, int &y, QSize &size);
    quint32 getNewId() { return m_widgetIdCounter++; }

    w_map m_widgets;
    mark_map m_marks;
    quint32 m_widgetIdCounter;
    Storage *m_storage;
    LorrisAnalyzer* m_analyzer;

    QPoint m_mouse_orig;

    bool m_skipNextMove;
    QPoint m_grid_offset;
    quint32 m_grid;
    bool m_show_grid;

    QMenu *m_menu;
};

#endif // WIDGETAREA_H
