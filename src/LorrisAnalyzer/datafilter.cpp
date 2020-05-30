/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScrollArea>
#include <QApplication>

#include "datafilter.h"
#include "../misc/utils.h"
#include "labellayout.h"
#include "DataWidgets/datawidget.h"

DataFilter::DataFilter(quint8 type, quint32 id, QString name, QObject *parent) : QObject(parent)
{
    m_type = type;
    m_id = id;
    m_name = name;
    m_layout = NULL;
    m_area = NULL;
    m_lastIdx = 0;
}

DataFilter::~DataFilter()
{
    //delete m_layout; layout is child of area
    delete m_area;
}

DataFilter *DataFilter::createFilter(quint8 type, quint32 id, const QString &name, QObject *parent)
{
    switch(type)
    {
        case FILTER_CONDITION:
            return new ConditionFilter(id, name, parent);
        case FILTER_EMPTY:
            return new EmptyFilter(id, name, parent);
        default:
            return NULL;
    }
}

void DataFilter::connectWidget(DataWidget *w, bool exclusive)
{
    if(exclusive)
    {
        const data_widget_info& i = w->getInfo();
        if(!i.filter.isNull())
        {
            disconnect(i.filter.data(), 0, w, 0);
            disconnect(w, 0, i.filter.data(), 0);
        }
    }

    disconnect(this, 0, w, 0); // prevent double-connect
    disconnect(w, 0, this, 0);

    connect(this, SIGNAL(newData(analyzer_data*,quint32)), w, SLOT(newData(analyzer_data*,quint32)));
    connect(w,    SIGNAL(updateForMe()),                      SLOT(updateForWidget()));
    connect(w,    SIGNAL(mouseStatus(bool,data_widget_info,qint32)), SLOT(widgetMouseStatus(bool,data_widget_info,qint32)));
}

void DataFilter::updateForWidget()
{
    Q_ASSERT(sender() && sender()->inherits("DataWidget"));

    if(m_lastData.hasData())
        ((DataWidget*)sender())->newData(&m_lastData, m_lastIdx);
}

void DataFilter::sendLastData()
{
    if(m_lastData.hasData())
        emit newData(&m_lastData, m_lastIdx);
}

void DataFilter::clearLastData()
{
    m_lastData.setData(NULL);
}

void DataFilter::handleData(analyzer_data *data, quint32 idx)
{
    if(!m_layout || !isOkay(data))
        return;

    m_layout->SetData(data);
    m_lastData.copy(data);
    m_lastIdx = idx;

    emit newData(data, idx);
}

void DataFilter::setHeader(analyzer_header *header)
{
    if(m_layout)
        m_layout->setHeader(header);
}

void DataFilter::setAreaAndLayout(QScrollArea *a, ScrollDataLayout *l)
{
    delete m_area;
    //delete m_layout; layout is child of area

    m_area = a;
    m_layout = l;

    if(m_layout)
    {
        m_area->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_area, SIGNAL(customContextMenuRequested(QPoint)), SLOT(layoutContextMenu(QPoint)));
        m_layout->setDividers(m_dividers);
    }
}

void DataFilter::setDividers(const std::vector<int>& dividers) {
    m_dividers = dividers;
    if(m_layout)
        m_layout->setDividers(m_dividers);
}

void DataFilter::save(DataFileParser *file)
{
    file->writeBlockIdentifier("dataFilter");
    file->writeVal(m_type);
    file->writeVal(m_id);
    file->writeString(m_name);
    file->writeVal(m_lastIdx);
}

void DataFilter::load(DataFileParser *file)
{
    m_lastIdx = file->readVal<quint32>();
}

void DataFilter::widgetMouseStatus(bool in, const data_widget_info& info, qint32 parent)
{
    if(parent != -1)
        return;

    if(m_layout)
        m_layout->setHightlightLabel(info.pos, in);

    if(in && (qApp->keyboardModifiers() & Qt::ShiftModifier))
        emit activateTab();
}

void DataFilter::layoutContextMenu(const QPoint &pos)
{
    QMenu menu;

    QAction *title = menu.addAction(tr("Data format"));
    title->setEnabled(false);
    menu.addSeparator();

    QAction *act[] = {
        menu.addAction(tr("Hexadecimal")),
        menu.addAction(tr("Decimal")),
        menu.addAction(tr("ASCII"))
    };

    size_t curr = m_layout->getCurrentFmt();
    for(size_t i = 0; i < sizeof_array(act); ++i)
    {
        act[i]->setCheckable(true);
        act[i]->setChecked(i == curr);
    }

    QAction *res = menu.exec(m_area->mapToGlobal(pos));
    if(!res)
        return;

    for(size_t i = 0; i < sizeof_array(act); ++i)
    {
        if(act[i] != res)
            continue;

        if(i == curr)
            break;

        m_layout->fmtChanged(i);
        if(m_lastData.hasData())
            m_layout->SetData(&m_lastData);
        break;
    }
}

ConditionFilter::ConditionFilter(quint32 id, QString name, QObject *parent) :
    DataFilter(FILTER_CONDITION, id, name, parent)
{
}

ConditionFilter::~ConditionFilter()
{
    delete_vect(m_conditions);
}

bool ConditionFilter::isOkay(analyzer_data *data)
{
    for(quint32 i = 0; i < m_conditions.size(); ++i)
        if(!m_conditions[i]->isOkay(data))
            return false;
    return !m_conditions.empty();
}

void ConditionFilter::removeCondition(FilterCondition *c)
{
    for(std::vector<FilterCondition*>::iterator itr = m_conditions.begin(); itr != m_conditions.end(); ++itr)
    {
        if(*itr == c)
        {
            delete *itr;
            m_conditions.erase(itr);
            return;
        }
    }
}

void ConditionFilter::save(DataFileParser *file)
{
    DataFilter::save(file);

    file->writeBlockIdentifier("conditionFilter");
    file->writeVal((quint32)m_conditions.size());

    for(quint32 i = 0; i < m_conditions.size(); ++i)
        m_conditions[i]->save(file);
}

void ConditionFilter::load(DataFileParser *file)
{
    DataFilter::load(file);

    if(file->seekToNextBlock("conditionFilter", "dataFilter"))
    {
        quint32 count = file->readVal<quint32>();
        for(quint32 i = 0; i < count; ++i)
        {
            if(!file->seekToNextBlock("filterCondition", "dataFilter"))
                break;

            FilterCondition *c = FilterCondition::createCondition(file->readVal<quint8>());
            if(c)
            {
                c->load(file);
                m_conditions.push_back(c);
            }
        }
    }
}

EmptyFilter::EmptyFilter(quint32 id, QString name, QObject *parent) :
    DataFilter(FILTER_EMPTY, id, name, parent)
{
}

FilterCondition *FilterCondition::createCondition(quint8 type)
{
    switch(type)
    {
        case COND_DEV:
            return new DevFilterCondition(0);
        case COND_CMD:
            return new CmdFilterCondition(0);
        case COND_BYTE:
            return new ByteFilterCondition(0, 0);
        case COND_SCRIPT:
            return new ScriptFilterCondition(0);
        default:
            return NULL;
    }
}

void FilterCondition::save(DataFileParser *file)
{
    file->writeBlockIdentifier("filterCondition");
    file->writeVal(m_type);
}

void FilterCondition::load(DataFileParser* /*file*/)
{

}

bool DevFilterCondition::isOkay(analyzer_data *data)
{
    quint8 id;
    return data->getDeviceId(id) && id == m_dev;
}

QString DevFilterCondition::getDesc() const
{
    return QObject::tr("Device == 0x%1").arg(m_dev, 2, 16, QChar('0'));
}

void DevFilterCondition::save(DataFileParser *file)
{
    FilterCondition::save(file);

    file->writeBlockIdentifier("devCondition");
    file->writeVal(m_dev);
}

void DevFilterCondition::load(DataFileParser *file)
{
    FilterCondition::load(file);

    if(file->seekToNextBlock("devCondition", "filterCondition"))
        m_dev = file->readVal<qint16>();
}

bool CmdFilterCondition::isOkay(analyzer_data *data)
{
    quint8 cmd;
    return data->getCmd(cmd) && cmd == m_cmd;
}

QString CmdFilterCondition::getDesc() const
{
    return QObject::tr("Command == 0x%1").arg(m_cmd, 2, 16, QChar('0'));
}

void CmdFilterCondition::save(DataFileParser *file)
{
    FilterCondition::save(file);

    file->writeBlockIdentifier("cmdCondition");
    file->writeVal(m_cmd);
}

void CmdFilterCondition::load(DataFileParser *file)
{
    FilterCondition::load(file);

    if(file->seekToNextBlock("cmdCondition", "filterCondition"))
        m_cmd = file->readVal<qint16>();
}

bool ByteFilterCondition::isOkay(analyzer_data *data)
{
    try {
        if(m_pos < (quint32)data->getData().length())
            return data->getUInt8(m_pos) == m_byte;
    }
    catch(const char*) {
    }
    return false;
}

QString ByteFilterCondition::getDesc() const
{
    return QObject::tr("Byte at idx %1 == 0x%2").arg(m_pos).arg(m_byte, 2, 16, QChar('0'));
}

void ByteFilterCondition::save(DataFileParser *file)
{
    FilterCondition::save(file);

    file->writeBlockIdentifier("byteCondition");
    file->writeVal(m_byte);
    file->writeVal(m_pos);
}

void ByteFilterCondition::load(DataFileParser *file)
{
    FilterCondition::load(file);

    if(file->seekToNextBlock("byteCondition", "filterCondition"))
    {
        m_byte = file->readVal<quint8>();
        m_pos = file->readVal<quint32>();
    }
}

ScriptFilterCondition::ScriptFilterCondition(int engine) : FilterCondition(COND_SCRIPT)
{
    m_lang = engine;
    m_script = QObject::tr("// Return true if okay, false to filter out\n"
                  "function dataPass(data, dev, cmd) {\n"
                  "    return false;\n"
                  "}\n");
    m_engine.pushContext();
}

void ScriptFilterCondition::setScript(const QString &script)
{
    m_error.clear();
    m_script = script;

    m_engine.popContext();
    QScriptContext *ctx = m_engine.pushContext();

    m_engine.evaluate(script);
    if(m_engine.hasUncaughtException())
    {
        m_error = QString("%1: %2").arg(m_engine.uncaughtExceptionLineNumber()).arg(m_engine.uncaughtException().toString());
        return;
    }

    m_func = ctx->activationObject().property("dataPass");
    if(!m_func.isFunction())
    {
        m_error = QObject::tr("Could not find dataPass function!");
        return;
    }

    QScriptValueList args;
    args.push_back(m_engine.newArray());
    args << -1 << -1;

    m_func.call(QScriptValue(), args);

    if(m_engine.hasUncaughtException())
    {
        m_error = QString("%1: %2").arg(m_engine.uncaughtExceptionLineNumber()).arg(m_engine.uncaughtException().toString());
        m_func = QScriptValue();
        return;
    }
}

bool ScriptFilterCondition::isOkay(analyzer_data *data)
{
    if(!m_func.isFunction())
        return false;

    const QByteArray& pkt_data = data->getData();

    QScriptValue jsData = m_engine.newArray(pkt_data.size());
    for(qint32 i = 0; i < pkt_data.size(); ++i)
        jsData.setProperty(i, QScriptValue(&m_engine, (quint8)pkt_data[i]));

    QScriptValueList args;
    args.push_back(jsData);

    quint8 res = 0;
    if(data->getDeviceId(res)) args << res;
    else                       args << -1;

    if(data->getCmd(res))  args << res;
    else                   args << -1;

    QScriptValue val = m_func.call(QScriptValue(), args);
    return val.toBool();
}

QString ScriptFilterCondition::getDesc() const
{
    return QObject::tr("Script");
}

void ScriptFilterCondition::save(DataFileParser *file)
{
    FilterCondition::save(file);

    file->writeBlockIdentifier("scriptCondition");
    file->writeVal(m_lang);
    file->writeString(m_script);
}

void ScriptFilterCondition::load(DataFileParser *file)
{
    FilterCondition::load(file);

    if(file->seekToNextBlock("scriptCondition", "filterCondition"))
    {
        m_lang = file->readVal<int>();
        setScript(file->readString());
    }
}
