/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>
#include <QLineEdit>

#include "scriptwidget.h"
#include "scripteditor.h"
#include "engines/qtscriptengine.h"
#include "../../../ui/terminal.h"

REGISTER_DATAWIDGET(WIDGET_SCRIPT, Script, NULL)

ScriptWidget::ScriptWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Script"));
    setIcon(":/dataWidgetIcons/script.png");

    m_widgetType = WIDGET_SCRIPT;
    m_editor = NULL;

    m_terminal = new Terminal(this);
    layout->addWidget(m_terminal, 4);

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setToolTip(tr("Interactive input.\nAvailable in script as \"inputLine\" (class QLineEdit) object."));
    m_inputEdit->hide();
    layout->addWidget(m_inputEdit);

    resize(120, 100);

    m_engine = NULL;
    m_engine_type = ENGINE_QTSCRIPT;
    m_error_label = new QLabel(this);
    m_error_label->setPixmap(QIcon(":/actions/red-cross").pixmap(16, 16));
    m_error_label->hide();

    ((QHBoxLayout*)layout->itemAt(0)->layout())->insertWidget(2, m_error_label);
}

ScriptWidget::~ScriptWidget()
{
    if(m_editor)
        emit removeChildTab(m_editor);

    if(!m_examplePrevs.empty())
    {
        std::vector<ExamplePreviewTab*> prevs = m_examplePrevs.takeAll();
        for(quint32 i = 0; i < prevs.size(); ++i)
            emit removeChildTab(prevs[i]);
    }
}

void ScriptWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QAction *src_act = contextMenu->addAction(tr("Set source..."));
    m_inputAct = contextMenu->addAction(tr("Show input line"));
    m_inputAct->setCheckable(true);

    connect(m_inputAct,           SIGNAL(triggered(bool)), SLOT(inputShowAct(bool)));
    connect(src_act,              SIGNAL(triggered()), SLOT(setSourceTriggered()));
    connect(&m_error_blink_timer, SIGNAL(timeout()),   SLOT(blinkError()));
    connect(this, SIGNAL(closeEdit()), SLOT(closeEditor()), Qt::QueuedConnection);
    connect(this, SIGNAL(setSourceDelayed(QString)), SLOT(setSourceDirect(QString)), Qt::QueuedConnection);

    inputShowAct(sConfig.get(CFG_BOOL_SCRIPT_SHOW_INPUT));

    m_engine_type = sConfig.get(CFG_QUINT32_ANALYZER_SCRIPT_ENG);
    createEngine();
}

void ScriptWidget::createEngine()
{
    delete m_engine;
    m_engine = ScriptEngine::getEngine(m_engine_type, (WidgetArea*)parent(), getId(), this);

    if(!m_engine && m_engine_type != ENGINE_QTSCRIPT)
    {
        Utils::showErrorBox(tr("Script engine %1 is not available, using QtScript!").arg(m_engine_type));
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
    connect(m_engine,      SIGNAL(error(QString)),              this,       SLOT(blinkError()));
    connect(this,          SIGNAL(rawData(QByteArray)),         m_engine,   SLOT(rawData(QByteArray)));
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

    // scripts filename
    file->writeBlockIdentifier("scriptWFilename");
    file->writeString(m_filename);

    // script editor
    file->writeBlockIdentifier("scriptWEditor");
    file->writeVal(!m_editor.isNull());
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

    // Filename
    if(file->seekToNextBlock("scriptWFilename", BLOCK_WIDGET))
        m_filename = file->readString();

    emit setSourceDelayed(source);

    // Editor settings
    if(file->seekToNextBlock("scriptWEditor", BLOCK_WIDGET))
        if(file->readVal<bool>())
            setSourceTriggered();
}

void ScriptWidget::setSourceTriggered()
{
    if(m_editor)
    {
        m_editor->activateTab();
        return;
    }

    m_editor = new ScriptEditor(m_engine->getSource(), m_filename, m_engine_type);
    emit addChildTab(m_editor, m_editor->windowTitle());
    m_editor->activateTab();

    connect(m_editor, SIGNAL(applySource(bool)), SLOT(sourceSet(bool)));
    connect(m_editor, SIGNAL(rejected()),        SLOT(closeEditor()), Qt::QueuedConnection);
    connect(m_engine, SIGNAL(error(QString)), m_editor, SLOT(addError(QString)));
    connect(m_editor, SIGNAL(openPreview(QString)), SLOT(addExampleTab(QString)));
}

void ScriptWidget::sourceSet(bool close)
{
    try
    {
        int type = m_editor->getEngine();

        if(type != m_engine_type)
        {
            sConfig.set(CFG_QUINT32_ANALYZER_SCRIPT_ENG, type);
            m_engine_type = type;
            createEngine();
            connect(m_engine, SIGNAL(error(QString)), m_editor, SLOT(addError(QString)));
        }
        m_editor->clearErrors();

        m_error_blink_timer.stop();
        m_error_label->hide();

        m_engine->setSource(m_editor->getSource());
        m_filename = m_editor->getFilename();

        if(close)
            emit closeEdit();
        emit updateForMe();
    }
    catch(const QString& text)
    {
        Utils::showErrorBox(text);
    }
}

void ScriptWidget::setSourceDirect(const QString &source)
{
    if(m_editor)
        m_editor->setSource(source);

    try
    {
        if(m_engine)
            m_engine->setSource(source);
    }catch(const QString&) { }
}

void ScriptWidget::closeEditor()
{
    emit removeChildTab(m_editor);
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

void ScriptWidget::blinkError()
{
    m_error_label->setVisible(!m_error_label->isVisible());
    m_error_blink_timer.start(500);
}

void ScriptWidget::titleDoubleClick()
{
    setSourceTriggered();
}

void ScriptWidget::addExampleTab(const QString &name)
{
    ExamplePreviewTab *tab = new ExamplePreviewTab(name);
    m_examplePrevs.add(tab);

    addChildTab(tab, name + tr(" - example"));
    tab->activateTab();
}

void ScriptWidget::inputShowAct(bool show)
{
    m_inputAct->setChecked(show);
    m_inputEdit->setVisible(show);
    sConfig.set(CFG_BOOL_SCRIPT_SHOW_INPUT, show);
}

ScriptWidgetAddBtn::ScriptWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Script"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/script.png"));

    m_widgetType = WIDGET_SCRIPT;
}
