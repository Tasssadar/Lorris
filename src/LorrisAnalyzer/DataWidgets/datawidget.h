/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QMenu>

#include "../packet.h"
#include "../analyzerdatafile.h"

enum WidgetTypes
{
    WIDGET_NUMBERS,
    WIDGET_BAR,
    WIDGET_COLOR,
    WIDGET_GRAPH,
    WIDGET_SCRIPT,
    WIDGET_INPUT,

    WIDGET_MAX
    //TODO: X Y mapa, rafickovej ukazatel, timestamp, bool, binarni cisla

};

enum NumberTypes
{
    NUM_UINT8,
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
    DRAG_MOVE       = 0x01,
    DRAG_RES_LEFT   = 0x02,
    DRAG_RES_RIGHT  = 0x04,
    //DRAG_RES_TOP    = 0x08, // Unused
    DRAG_RES_BOTTOM = 0x10
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
class AnalyzerDataFile;
class AnalyzerDataStorage;

class DataWidget : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    void SendData(const QByteArray& data);

public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();

    virtual void setUp(AnalyzerDataStorage *);

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

    virtual void saveWidgetInfo(AnalyzerDataFile *file);
    virtual void loadWidgetInfo(AnalyzerDataFile *file);

    static QVariant getNumFromPacket(analyzer_data *data, quint32 pos, quint8 type);

    void setUpdating(bool update)
    {
        m_updating = update;
    }

    void setWidgetControlled(qint32 widget);
    qint32 getWidgetControlled() { return m_widgetControlled; }

public slots:
    virtual void newData(analyzer_data *data, quint32);
    void setTitle(const QString& title);
    virtual void setValue(const QVariant &var);
    void lockTriggered();

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void contextMenuEvent ( QContextMenuEvent * event );
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    virtual void processData(analyzer_data *data);

    QString getTitle();
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
    inline bool iw(int w) { return w + width() > ((QWidget*)parent())->width(); }
    inline bool ih(int h) { return h + height() > ((QWidget*)parent())->height(); }

    inline int getWPosInside(int w)
    {
        if(ih(w))
            return ((QWidget*)parent())->width() - width();
        else if(w < 0)
            return 0;
        return w;
    }

    inline int getHPosInside(int h)
    {
        if(ih(h))
            return ((QWidget*)parent())->height() - height();
        else if(h < 0)
            return 0;
        return h;
    }

    quint8 getDragAction(const QPoint& clickPos);
    void dragResize(QMouseEvent* e);
    void dragMove(QMouseEvent* e);

    QPoint mOrigin;
    quint8 m_dragAction;
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
