#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "../packet.h"

enum WidgetTypes
{
    WIDGET_NUMBERS
};

struct data_widget_info
{
    qint16 device;
    qint16 command;
    quint32 pos;
};

class DataWidget : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();

public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();

    virtual void setUp();

    void setId(quint32 id) { m_id = id; }
    quint32 getId() { return m_id; }

    void setInfo(qint16 device, qint16 command, quint32 pos)
    {
        m_info.device = device;
        m_info.command = command;
        m_info.pos = pos;
    }
    data_widget_info getInfo() { return m_info; }

public slots:
    void newData(analyzer_data *data);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

    virtual void processData(analyzer_data *data);

    void setTitle(QString title);
    void addLayout(QLayout *layout)
    {
        layout->addItem(layout);
    }
    void addWidget(QWidget *w)
    {
        layout->addWidget(w);
    }

    quint8 m_widgetType;
    data_widget_info m_info;
    bool m_assigned;

private:
    inline bool iw(int w) { return w + width() > ((QWidget*)parent())->width(); }
    inline bool ih(int h) { return h + height() > ((QWidget*)parent())->height(); }

    QPoint mOrigin;
    QVBoxLayout *layout;
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

    virtual QPixmap getRender();

    quint8 m_widgetType;
};

class CloseLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CloseLabel(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event);

};

#endif // DATAWIDGET_H
