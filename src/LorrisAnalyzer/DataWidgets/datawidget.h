/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QMenu>

#include "../packet.h"
#include "../../misc/datafileparser.h"
#include "../widgetfactory.h"

enum WidgetTypes
{
    WIDGET_NUMBERS,
    WIDGET_BAR,
    WIDGET_COLOR,
    WIDGET_GRAPH,
    WIDGET_SCRIPT,
    WIDGET_INPUT,
    WIDGET_TERMINAL,
    WIDGET_BUTTON,
    WIDGET_CIRCLE,
    WIDGET_SLIDER,
    WIDGET_CANVAS,

    WIDGET_MAX
    //TODO: X Y mapa, rafickovej ukazatel, timestamp, bool, binarni cisla
};

enum NumberTypes
{
    NUM_UINT8 = 0,
    NUM_UINT16,
    NUM_UINT32,
    NUM_UINT64,

    NUM_INT8,
    NUM_INT16,
    NUM_INT32,
    NUM_INT64,

    NUM_FLOAT,
    NUM_DOUBLE,

    NUM_COUNT
};

enum DragActions
{
    DRAG_NONE       = 0x00,
    DRAG_MOVE       = 0x01,
    DRAG_RES_LEFT   = 0x02,
    DRAG_RES_RIGHT  = 0x04,
    //DRAG_RES_TOP    = 0x08, // Unused
    DRAG_RES_BOTTOM = 0x10,
    DRAG_COPY       = 0x20
};

#define RESIZE_BORDER 15 // number of pixels from every side which counts as resize drag

struct data_widget_info
{
    quint32 pos;
    qint16 device;
    qint16 command;

    bool operator==(const data_widget_info& other)
    {
        return (other.pos == pos && other.device == device && other.command == command);
    }

    bool operator!=(const data_widget_info& other)
    {
        return (other.pos != pos || other.device != device || other.command != command);
    }
};

class CloseLabel;
class DataFileParser;
class Storage;
class ChildTab;

class DataWidget : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(quint8 widgetType READ getWidgetType)

Q_SIGNALS:
    void updateData();
    void updateForMe();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    void SendData(const QByteArray& data);

    void titleChanged(const QString& newTitle);
    void scriptEvent(const QString& eventId);

    void addChildTab(ChildTab *tab, const QString& name);
    void removeChildTab(ChildTab *tab);

public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();

    virtual void setUp(Storage *);

    void setId(quint32 id);
    quint32 getId() { return m_id; }

    bool isMouseIn() { return m_mouseIn; }

    void setInfo(qint16 device, qint16 command, quint32 pos)
    {
        m_info.device = device;
        m_info.command = command;
        m_info.pos = pos;
    }
    const data_widget_info& getInfo() { return m_info; }

    virtual void saveWidgetInfo(DataFileParser *file);
    virtual void loadWidgetInfo(DataFileParser *file);

    static QVariant getNumFromPacket(analyzer_data *data, quint32 pos, quint8 type);

    void setUpdating(bool update)
    {
        m_updating = update;
    }

    void setWidgetControlled(qint32 widget);
    qint32 getWidgetControlled() { return m_widgetControlled; }

    QString getTitle();

    quint8 getWidgetType() const { return m_widgetType; }

    void align();

public slots:
    virtual void newData(analyzer_data *data, quint32);
    void setTitle(QString title);
    void lockTriggered();
    void remove();
    void setTitleVisibility(bool visible);

    //events
    virtual void onWidgetAdd(DataWidget *w);
    virtual void onWidgetRemove(DataWidget *w);
    virtual void onScriptEvent(const QString& eventId);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void contextMenuEvent ( QContextMenuEvent * event );
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void childEvent(QChildEvent *event);
    bool eventFilter(QObject *, QEvent *ev);

    virtual void processData(analyzer_data *data);

    void setIcon(QString path);

    quint8 m_widgetType;
    data_widget_info m_info;
    bool m_assigned;
    bool m_updating;
    qint32 m_widgetControlled;

    QVBoxLayout *layout;
    QMenu *contextMenu;

private slots:
    void setTitleTriggered();

private:
    inline void mapToGrid(int &val);
    void mapXYToGrid(QPoint& point);
    void mapXYToGrid(int& x, int& y);

    quint8 getDragAction(QMouseEvent* ev);
    void dragResize(QMouseEvent* e);
    void dragMove(QMouseEvent* e, DataWidget *widget);

    void copyWidget(QMouseEvent *ev);

    QPoint mOrigin;
    quint8 m_dragAction;
    DataWidget *m_copy_widget;
    bool m_locked;
    bool m_mouseIn;

    QAction *m_lockAction;
    CloseLabel *m_closeLabel;
    QLabel *m_icon_widget;
    QLabel *m_title_label;
    quint32 m_id;
};

class DataWidgetAddBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit DataWidgetAddBtn(QWidget *parent);
    ~DataWidgetAddBtn();

protected:
    void mousePressEvent(QMouseEvent *event);

    virtual const QPixmap &getRender();

    quint8 m_widgetType;
    QPixmap *m_pixmap;
};

enum CloseLabelState
{
    CLOSE_NONE   = 0x00,
    CLOSE_LOCKED = 0x01,
    CLOSE_SCRIPT = 0x02
};

class CloseLabel : public QLabel
{
    Q_OBJECT

Q_SIGNALS:
    void removeWidget(quint32 id);

public:
    explicit CloseLabel(QWidget *parent);

    void setLocked(bool locked);
    void setScript(bool script);
    void setId(quint32 id) { m_id = id; }

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QString getTextByState();

    quint8 m_state;
    quint32 m_id;
};

#endif // DATAWIDGET_H
