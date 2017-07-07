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
#include <QPointer>

#include "../packet.h"
#include "../../misc/datafileparser.h"
#include "../widgetfactory.h"
#include "../../misc/gestureidentifier.h"
#include "../undoactions.h"
#include "../datafilter.h"
#include "../../misc/qtobjectpointer.h"

class WidgetArea;

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

    NUM_COUNT,
    NUM_STRING = NUM_COUNT,
    NUM_COUNT_WITH_STRING,
};

enum DragActions
{
    DRAG_NONE       = 0x00,
    DRAG_MOVE       = 0x01,
    DRAG_RES_LEFT   = 0x02,
    DRAG_RES_RIGHT  = 0x04,
    DRAG_RES_TOP    = 0x08,
    DRAG_RES_BOTTOM = 0x10,
    DRAG_RESIZE     = 0x20,
    DRAG_COPY       = 0x40
};

#define RESIZE_BORDER 8 // number of pixels from every side which counts as resize drag

struct data_widget_infoV1 // for loading old datafiles
{
    quint32 pos;
    qint16 device;
    qint16 command;
};

struct data_widget_info
{
    quint32 pos;
    QtObjectPointer<DataFilter> filter;

    const data_widget_info& operator =(const data_widget_info& other)
    {
        pos = other.pos;
        filter = other.filter.data();
        return *this;
    }

    bool operator==(const data_widget_info& other)
    {
        return (other.pos == pos && other.filter.data() == filter.data());
    }

    bool operator!=(const data_widget_info& other)
    {
        return (other.pos != pos || other.filter.data() != filter.data());
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

    friend class WidgetFactory;

protected:
    enum widgetStates
    {
        STATE_NONE       = 0x00,
        STATE_LOCKED     = 0x01,
        STATE_ASSIGNED   = 0x02,
        STATE_UPDATING   = 0x04,
        STATE_MOUSE_IN   = 0x08,
        STATE_BLOCK_MOVE = 0x10,
        STATE_SCALED_UP  = 0x20,
        STATE_SELECTED   = 0x40,
        STATE_HIGHLIGHTED= 0x80
    };

Q_SIGNALS:
    void updateData();
    void updateForMe();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    void clearPlacementLines();
    void SendData(const QByteArray& data);

    void titleChanged(const QString& newTitle);
    void scriptEvent(const QString& eventId);
    void scriptEvent(const QString &eventId, const QVariantList& args);

    void addChildTab(ChildTab *tab, const QString& name);
    void removeChildTab(ChildTab *tab);
    void rawData(const QByteArray& data);

    void addUndoAct(UndoAction *act);
    void toggleSelection(bool selected);

    void resized(int w, int h, int oldW, int oldH);
    void moved(int x, int y, int oldX, int oldY);

public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();

    virtual void setUp(Storage *);

    void setId(quint32 id);
    quint32 getId() { return m_id; }

    bool isMouseIn() { return (m_state & STATE_MOUSE_IN); }
    void setHighlighted(bool highlight);

    void setInfo(DataFilter *f, quint32 pos)
    {
        m_info.filter = f;
        m_info.pos = pos;
    }
    const data_widget_info& getInfo() { return m_info; }

    virtual void saveWidgetInfo(DataFileParser *file);
    virtual void loadWidgetInfo(DataFileParser *file);

    static QVariant getNumFromPacket(analyzer_data *data, quint32 pos, quint8 type);

    void setUpdating(bool update)
    {
        if(update)
            m_state |= STATE_UPDATING;
        else
            m_state &= ~(STATE_UPDATING);
    }

    inline bool isScaledUp() const { return (m_state & STATE_SCALED_UP); }
    void setScaledUp(bool scaled);

    void setWidgetControlled(qint32 widget);
    qint32 getWidgetControlled() { return m_widgetControlled; }

    QString getTitle();
    quint8 getWidgetType() const { return m_widgetType; }
    void align();

    void updateForThis()
    {
        emit updateForMe();
    }

    void showSelectFrame(bool show);
    void dragMove(QMouseEvent* e, DataWidget *widget);

public slots:
    virtual void newData(analyzer_data *data, quint32);
    void setTitle(QString title);
    void lockTriggered();
    void remove();
    void setTitleVisibility(bool visible);
    void setLocked(bool locked);
    bool isLocked() const { return (m_state & STATE_LOCKED); }
    void setError(bool error, QString tooltip = QString());
    void setSelected(bool selected);
    void move(int x, int y) { QFrame::move(x, y); }
    void move(const QPoint& p) { QFrame::move(p); }
    void resize(int w, int h) { QFrame::resize(w, h); }
    void resize(const QSize& s) { QFrame::resize(s); }

    //events
    virtual void onWidgetAdd(DataWidget *w);
    virtual void onWidgetRemove(DataWidget *w);
    virtual void onScriptEvent(const QString& eventId);
    virtual void onScriptEvent(const QString& eventId, const QVariantList& args);

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
    void focusInEvent(QFocusEvent *event);
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *ev);
    void moveEvent(QMoveEvent *ev);

    virtual void titleDoubleClick();

    virtual void processData(analyzer_data *data);

    void setIcon(QString path);
    void setType(quint32 widgetType);

    inline WidgetArea *widgetArea() const
    {
        Q_ASSERT(parent()->inherits("WidgetArea"));
        return (WidgetArea*)parent();
    }

    void saveDataInfo(DataFileParser *file, data_widget_info& info);
    void loadDataInfo(DataFileParser *file, data_widget_info& info);
    void loadOldDataInfo(DataFileParser *file, data_widget_info& info);

    bool isAssigned() const { return (m_state & STATE_ASSIGNED); }
    bool isUpdating() const { return (m_state & STATE_UPDATING); }
    bool isMoveBlocked() const { return (m_state & STATE_BLOCK_MOVE); }
    bool isSelected() const { return (m_state & STATE_SELECTED); }
    bool isHighlighted() const { return (m_state & STATE_HIGHLIGHTED); }

    void setUseErrorLabel(bool use);

    quint8 m_widgetType;
    data_widget_info m_info;
    qint32 m_widgetControlled;

    QVBoxLayout *layout;
    QLabel *m_error_label;
    QTimer *m_error_blink_timer;
    QMenu *contextMenu;

    quint8 m_state;

private slots:
    void setTitleTriggered();
    void gestureCompleted(int gesture);
    void blinkErrorLabel();

private:
    inline void mapToGrid(int &val);
    void mapXYToGrid(QPoint& point);
    void mapXYToGrid(int& x, int& y);

    quint8 getDragAction(QMouseEvent* ev);
    void dragResize(QMouseEvent* e);
    static Qt::CursorShape getCursor(quint8 act);
    void startAnimation(const QRect& target);

    void copyWidget(QMouseEvent *ev);

    QPoint m_clickPos;
    quint8 m_dragAction;
    DataWidget *m_copy_widget;

    QAction *m_lockAction;
    CloseLabel *m_closeLabel;
    QLabel *m_title_label;
    QLabel *m_icon_widget;
    QFrame *m_sep_line;
    quint32 m_id;
    GestureIdentifier m_gestures;
    QRect m_orig_geometry;
};

class DataWidgetAddBtn : public QPushButton
{
    Q_OBJECT
public:
    DataWidgetAddBtn(quint32 type, const QString &name, QWidget *parent);
    ~DataWidgetAddBtn();

    void setText(const QString &text);

public slots:
    void setTiny(bool tiny);

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
