/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>

#include "scriptwidget.h"
#include "scripteditor.h"
#include "engines/qtscriptengine.h"
#include "../../../ui/terminal.h"

ScriptWidget::ScriptWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Script"));
    setIcon(":/dataWidgetIcons/script.png");

    m_widgetType = WIDGET_SCRIPT;
    m_editor = NULL;

    m_terminal = new Terminal(this);
    layout->addWidget(m_terminal, 4);

    resize(120, 100);

    m_engine = NULL;
    m_engine_type = ENGINE_QTSCRIPT;
}

ScriptWidget::~ScriptWidget()
{
    delete m_editor;
}

void ScriptWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QAction *src_act = contextMenu->addAction(tr("Set source..."));
    connect(src_act,    SIGNAL(triggered()),                 this,       SLOT(setSourceTriggered()));

    createEngine();
}

void ScriptWidget::createEngine()
{
    m_engine = ScriptEngine::getEngine(m_engine_type, (WidgetArea*)parent(), getId(), m_terminal, this);

    if(!m_engine && m_engine_type != ENGINE_QTSCRIPT)
    {
        Utils::ThrowException(tr("Script engine %1 is not available, using QtScript!").arg(m_engine_type));
        m_engine_type = ENGINE_QTSCRIPT;
        return createEngine();
    }

    Q_ASSERT(m_engine);

    m_engine->setPos(pos().x(), pos().y());
    m_engine->setSize(size());

    connect(m_terminal, SIGNAL(keyPressed(QString)),         m_engine,      SLOT(keyPressed(QString)));
    connect(m_engine,      SIGNAL(clearTerm()),                 m_terminal, SLOT(clear()));
    connect(m_engine,      SIGNAL(appendTerm(QString)),         m_terminal, SLOT(appendText(QString)));
    connect(m_engine,      SIGNAL(appendTermRaw(QByteArray)),   m_terminal, SLOT(appendText(QByteArray)));
    connect(m_engine,      SIGNAL(SendData(QByteArray)),        this,       SIGNAL(SendData(QByteArray)));
}

void ScriptWidget::newData(analyzer_data *data, quint32 index)
{
    // FIXME: is it correct?
    //if(!m_updating)
    //    return;

    QString res = m_engine->dataChanged(data, index);
    if(!res.isEmpty())
        m_terminal->appendText(res);
}

void ScriptWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    // engine type
    file->writeBlockIdentifier("scriptWType");
    file->write((char*)&m_engine_type, sizeof(m_engine_type));

    // source
    file->writeBlockIdentifier("scriptWSource");
    file->writeString(m_engine->getSource());

    // terminal data
    file->writeBlockIdentifier("scriptWTerm");
    {
        QByteArray data = m_terminal->getData();
        quint32 len = data.length();

        file->write((char*)&len, sizeof(quint32));
        file->write(data.data(), len);
    }

    // terminal settings
    file->writeBlockIdentifier("scriptWTermSett");
    file->writeString(m_terminal->getSettingsData());

    // storage data
    m_engine->onSave();
    m_engine->getStorage()->saveToFile(file);
}

void ScriptWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    // engine type
    if(file->seekToNextBlock("scriptWType", BLOCK_WIDGET))
        file->read((char*)&m_engine_type, sizeof(m_engine_type));
    else
        m_engine_type = ENGINE_QTSCRIPT;

    QString source = "";
    // source
    if(file->seekToNextBlock("scriptWSource", BLOCK_WIDGET))
        source = file->readString();

    // terminal data
    if(file->seekToNextBlock("scriptWTerm", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QByteArray data(file->read(size));
        m_terminal->appendText(data);
    }

    // terminal settings
    if(file->seekToNextBlock("scriptWTermSett", BLOCK_WIDGET))
    {
        QString settings = file->readString();
        m_terminal->loadSettings(settings);
    }

    createEngine();

    // storage data
    m_engine->getStorage()->loadFromFile(file);

    try
    {
        m_engine->setSource(source);
    } catch(const QString&) { }
}

void ScriptWidget::setSourceTriggered()
{
    delete m_editor;

    m_editor = new ScriptEditor(m_engine->getSource(), m_engine_type, getTitle());
    m_editor->show();

    connect(m_editor, SIGNAL(applySource(bool)), SLOT(sourceSet(bool)));
    connect(m_engine, SIGNAL(error(QString)), m_editor, SLOT(addError(QString)));
}

void ScriptWidget::sourceSet(bool close)
{
    try
    {
        int type = m_editor->getEngine();

        if(type != m_engine_type)
        {
            m_engine_type = type;
            delete m_engine;
            createEngine();
            connect(m_engine, SIGNAL(error(QString)), m_editor, SLOT(addError(QString)));
        }
        m_editor->clearErrors();

        m_engine->setSource(m_editor->getSource());

        if(close)
        {
            m_editor->deleteLater();
            m_editor = NULL;
        }
        emit updateForMe();
    }
    catch(const QString& text)
    {
        Utils::ThrowException(text, m_editor);
    }
}

void ScriptWidget::moveEvent(QMoveEvent *)
{
    if(m_engine)
        m_engine->setPos(pos().x(), pos().y());
}

void ScriptWidget::resizeEvent(QResizeEvent *)
{
    if(m_engine)
        m_engine->setSize(size());
}

void ScriptWidget::onWidgetAdd(DataWidget *w)
{
    if(m_engine)
        m_engine->onWidgetAdd(w);
}

void ScriptWidget::onWidgetRemove(DataWidget *w)
{
    if(m_engine)
        m_engine->onWidgetRemove(w);
}

void ScriptWidget::onScriptEvent(const QString& eventId)
{
    if(m_engine)
        m_engine->callEventHandler(eventId);
}

ScriptWidgetAddBtn::ScriptWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Script"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/script.png"));

    m_widgetType = WIDGET_SCRIPT;
}
