/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef DUMMYENGINE_H
#define DUMMYENGINE_H

#include <QObject>
#include <QSize>
#include <QHash>

class ScriptStorage;
class analyzer_data;
class DataWidget;
class WidgetArea;
class QTimer;
class Terminal;
class ScriptWidget;

enum ScriptEngines
{
    ENGINE_QTSCRIPT = 0,
    ENGINE_PYTHON,
    ENGINE_MAX
};

class ScriptEngine : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void clearTerm();
    void appendTerm(const QString& text);
    void appendTermRaw(const QByteArray& text);
    void SendData(const QByteArray& data);

    void stopUsingJoy(QObject *object);
    void error(const QString& text);

public:
    ScriptEngine(WidgetArea *area , quint32 w_id, ScriptWidget *parent);
    ~ScriptEngine();

    static QStringList getEngineList();
    static ScriptEngine *getEngine(int idx, WidgetArea *area, quint32 w_id, ScriptWidget *parent);

    virtual void setSource(const QString& source) = 0;
    virtual QString dataChanged(analyzer_data *data, quint32 index) = 0;
    virtual void onWidgetAdd(DataWidget *w) = 0;
    virtual void onWidgetRemove(DataWidget *w) = 0;
    virtual void callEventHandler(const QString& eventId) = 0;
    virtual void onSave() = 0;

    virtual const QString& getSource() { return m_source; }

    void setPos(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

    void setSize(const QSize& size)
    {
        m_width = size.width();
        m_height = size.height();
    }
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }

    ScriptStorage *getStorage() const
    {
        return m_storage;
    }

public slots:
    virtual void keyPressed(const QString &key) = 0;

protected:
    ScriptWidget *scriptWidget() const { return (ScriptWidget*)parent(); }
    QString sanitizeWidgetName(QString const & name);

    int m_x;
    int m_y;
    int m_width;
    int m_height;

    ScriptStorage *m_storage;
    QString m_source;
    WidgetArea *m_area;
    qint32 m_widget_id;

    QHash<quint32, DataWidget*> m_widgets;
    std::list<QTimer*> m_timers;
};

#endif // DUMMYENGINE_H
