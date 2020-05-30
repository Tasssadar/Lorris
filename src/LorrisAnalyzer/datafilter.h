/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef DATAFILTER_H
#define DATAFILTER_H

#include <QString>
#include <vector>
#include <QScriptEngine>

#include "../misc/datafileparser.h"
#include "packet.h"

class QScrollArea;
class analyzer_data;
class ScrollDataLayout;
class DataWidget;
struct data_widget_info;

enum filterCondition
{
    COND_DEV = 0,
    COND_CMD,
    COND_BYTE,
    COND_SCRIPT,

    COND_MAX
};

class FilterCondition
{
public:
    FilterCondition(quint8 type) { m_type = type; }
    virtual ~FilterCondition() { }

    static FilterCondition *createCondition(quint8 type);

    quint8 getType() const { return m_type; }
    virtual bool isOkay(analyzer_data *data) = 0;
    virtual QString getDesc() const = 0;

    virtual void save(DataFileParser *file);
    virtual void load(DataFileParser *file);

private:
    quint8 m_type;
};

class DevFilterCondition : public FilterCondition
{
public:
    DevFilterCondition(qint16 dev) : FilterCondition(COND_DEV)
    {
        m_dev = dev;
    }

    bool isOkay(analyzer_data *data);
    void save(DataFileParser *file);
    void load(DataFileParser *file);

    QString getDesc() const;

    qint16 getDev() const { return m_dev; }
    void setDev(qint16 dev) { m_dev = dev; }

private:
    qint16 m_dev;
};

class CmdFilterCondition : public FilterCondition
{
public:
    CmdFilterCondition(qint16 cmd) : FilterCondition(COND_CMD)
    {
        m_cmd = cmd;
    }

    bool isOkay(analyzer_data *data);
    void save(DataFileParser *file);
    void load(DataFileParser *file);

    QString getDesc() const;

    qint16 getCmd() const { return m_cmd; }
    void setCmd(qint16 cmd) { m_cmd = cmd; }

private:
    qint16 m_cmd;
};

class ByteFilterCondition : public FilterCondition
{
public:
    ByteFilterCondition(quint32 pos, qint8 byte) : FilterCondition(COND_BYTE)
    {
        m_pos = pos;
        m_byte = byte;
    }

    bool isOkay(analyzer_data *data);
    void save(DataFileParser *file);
    void load(DataFileParser *file);

    QString getDesc() const;

    quint8 getByte() const { return m_byte; }
    quint32 getPos() const { return m_pos; }
    void setByte(quint8 byte) { m_byte = byte; }
    void setPos(quint32 pos) { m_pos = pos; }

private:
    quint8 m_byte;
    quint32 m_pos;
};

class ScriptFilterCondition : public FilterCondition
{
public:
    ScriptFilterCondition(int engine);

    bool isOkay(analyzer_data *data);
    void save(DataFileParser *file);
    void load(DataFileParser *file);

    QString getDesc() const;

    QString getScript() const { return m_script; }
    int getEngine() const { return m_lang; }
    void setScript(const QString& script);
    void setEngine(int lang) { m_lang = lang; }
    QString getError() const { return m_error; }

private:
    QString m_script;
    int m_lang;
    QScriptEngine m_engine;
    QScriptValue m_func;
    QString m_error;
};

enum dataFilterType
{
    FILTER_CONDITION = 0,
    FILTER_EMPTY,

    FILTER_MAX
};

class DataFilter : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void newData(analyzer_data *data, quint32 idx);
    void activateTab();

public:
    DataFilter(quint8 type, quint32 id, QString name, QObject *parent);
    virtual ~DataFilter();

    static DataFilter *createFilter(quint8 type, quint32 id, const QString& name, QObject *parent);

    virtual bool isOkay(analyzer_data *data) = 0;

    virtual void save(DataFileParser *file);
    virtual void load(DataFileParser *file);

    void setHeader(analyzer_header *header);
    void setAreaAndLayout(QScrollArea *a, ScrollDataLayout *l);
    void handleData(analyzer_data *data, quint32 idx);

    quint8 getType() const { return m_type; }
    QString getName() const { return m_name; }
    quint32 getId() const { return m_id; }
    void setName(const QString& name) { m_name = name; }
    quint32 getLastIdx() const { return m_lastIdx; }

    void sendLastData();
    void clearLastData();
    void connectWidget(DataWidget *w, bool exclusive = true);

    void setDividers(const std::vector<int>& dividers);
    const std::vector<int>& getDividers() const { return m_dividers; }

protected slots:
    void updateForWidget();
    void widgetMouseStatus(bool in, const data_widget_info &info, qint32 parent);
    void layoutContextMenu(const QPoint& pos);

protected:
    QString m_name;
    quint32 m_id;
    quint8 m_type;

    analyzer_data m_lastData;
    quint32 m_lastIdx;

    ScrollDataLayout *m_layout;
    QScrollArea *m_area;
    std::vector<int> m_dividers;
};

class ConditionFilter : public DataFilter
{
public:
    ConditionFilter(quint32 id, QString name, QObject *parent);
    ~ConditionFilter();

    bool isOkay(analyzer_data *data);
    void save(DataFileParser *file);
    void load(DataFileParser *file);

    void addCondition(FilterCondition *c)
    {
        m_conditions.push_back(c);
    }
    void removeCondition(FilterCondition *c);

    const std::vector<FilterCondition*>& getConditions() const { return m_conditions; }
    std::vector<FilterCondition*>& getConditions() { return m_conditions; }

private:
    std::vector<FilterCondition*> m_conditions;
};

class EmptyFilter : public DataFilter
{
public:
    EmptyFilter(quint32 id, QString name, QObject *parent);

    bool isOkay(analyzer_data *) { return true; }
};


#endif // DATAFILTER_H
