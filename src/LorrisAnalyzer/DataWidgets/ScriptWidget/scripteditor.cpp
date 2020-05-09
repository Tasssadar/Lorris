/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <QScrollBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QCryptographicHash>
#include <QToolBar>
#include <QComboBox>

#include "scripteditor.h"
#include "../../../common.h"
#include "engines/scriptengine.h"
#include "../../../ui/editorwidget.h"
#include "../../../WorkTab/WorkTabMgr.h"
#include "../../widgetarea.h"

#define MD5(x) QCryptographicHash::hash(x, QCryptographicHash::Md5)

static const QString filters[ENGINE_MAX] =
{
    ScriptEditor::tr("JavaScript file (*.js);;Any file (*.*)"),
    ScriptEditor::tr("Python file (*.py);;Any file (*.*)"),
};

ScriptEditor::ScriptEditor(WidgetArea *area, const QString& source, const QString& filename, int type) :
    ChildTab(NULL), ui(new Ui::ScriptEditor), m_language(ENGINE_QTSCRIPT), m_area(area), m_editor(NULL)
{
    ui->setupUi(this);

    m_exampleBtn = new QPushButton(this);
    m_exampleBtn->setToolTip(tr("Examples"));
    m_exampleBtn->setCheckable(true);
    m_exampleBtn->setIconSize(QSize(24, 24));
    m_exampleBtn->setIcon(QIcon(":/actions/info"));
    m_exampleBtn->setFlat(true);
    m_exampleBtn->setStyleSheet("padding: 3px;");

    m_settingsBtn = new QPushButton(this);
    m_settingsBtn->setToolTip(tr("Settings"));
    m_settingsBtn->setCheckable(true);
    m_settingsBtn->setIconSize(QSize(24, 24));
    m_settingsBtn->setIcon(QIcon(":/actions/system"));
    m_settingsBtn->setFlat(true);
    m_settingsBtn->setStyleSheet("padding: 3px;");

    m_eventsBtn = new QPushButton(this);
    m_eventsBtn->setToolTip(tr("Add widget event handler..."));
    m_eventsBtn->setCheckable(true);
    m_eventsBtn->setIconSize(QSize(24, 24));
    m_eventsBtn->setIcon(QIcon(":/actions/merge"));
    m_eventsBtn->setFlat(true);
    m_eventsBtn->setStyleSheet("padding: 3px;");

    QLabel *docLabel = new QLabel(this);
    docLabel->setTextFormat(Qt::RichText);
    docLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    docLabel->setOpenExternalLinks(true);
    docLabel->setText(tr("<a href=\"http://technika.tasemnice.eu/docs/Lorris/index.html\">Documentation</a>"));
    docLabel->setToolTip("http://technika.tasemnice.eu/docs/Lorris/index.html");

    QToolBar *bar = new QToolBar(this);
    bar->setIconSize(QSize(24, 24));

    QAction *load = bar->addAction(QIcon(":/actions/open"), tr("Load"), this, SLOT(loadAct()));
    QAction *save = bar->addAction(QIcon(":/actions/save"), tr("Save"), this, SLOT(saveAct()));
    QAction *saveAs = bar->addAction(QIcon(":/actions/save-as"), tr("Save as..."), this, SLOT(saveAs()));
    bar->addSeparator();
    QAction *undo = bar->addAction(QIcon(":/actions/undo"), tr("Undo"), this, SIGNAL(undo()));
    QAction *redo = bar->addAction(QIcon(":/actions/redo"), tr("Redo"), this, SIGNAL(redo()));
    bar->addSeparator();
    QAction *apply = bar->addAction(QIcon(":/icons/start"), tr("Apply"), this, SLOT(applyAct()));
    bar->addSeparator();
    bar->addWidget(m_eventsBtn);
    bar->addWidget(m_exampleBtn);
    bar->addWidget(m_settingsBtn);
    bar->addSeparator();
    bar->addWidget(docLabel);

    load->setShortcut(QKeySequence("Ctrl+O"));
    load->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    load->setToolTip(tr("Load (Ctrl+O)"));
    save->setShortcut(QKeySequence("Ctrl+S"));
    save->setToolTip(tr("Save (Ctrl+S)"));
    save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    saveAs->setShortcut(QKeySequence("Ctrl+Shift+S"));
    saveAs->setToolTip(tr("Save as... (Ctrl+Shift+S)"));
    saveAs->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    apply->setShortcut(QKeySequence("F5"));
    apply->setToolTip(tr("Apply (F5)"));
    apply->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(this, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoAvailable(bool)), redo, SLOT(setEnabled(bool)));

    ui->mainLayout->insertWidget(0, bar);

    quint32 editor_cfg = sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE);


#if defined(USE_QSCI)
    setEditor(EDITOR_QSCINTILLA);
#else
    setEditor(EDITOR_INTERNAL);
#endif

    if(editor_cfg != UINT_MAX)
        setEditor(editor_cfg);

    undo->setToolTip(tr("Undo (%1)").arg(m_editor->getUndoShortcut()));
    redo->setToolTip(tr("Redo (%1)").arg(m_editor->getRedoShortcut()));

    ui->resizeLine->setOrientation(false);
    ui->resizeLine->setResizeLayout(ui->mainLayout);
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit), sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit)-2, 100 - sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->resizeLine->updateStretch();

    m_errors = 0;
    m_ignoreFocus = false;

    m_editor->setText(source);
    m_changed = !source.isEmpty();

    setLanguage(type);

    connect(&m_status_timer, SIGNAL(timeout()), SLOT(clearStatus()));
    connect(qApp,   SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));

    ui->errorBtn->setChecked(sConfig.get(CFG_BOOL_SHOW_SCRIPT_ERROR));
    on_errorBtn_toggled(ui->errorBtn->isChecked());

    setFilename(filename);
    m_contentChanged = false;
    m_fileChanged = false;
    checkChange();

    connect(m_eventsBtn,   SIGNAL(clicked()),  SLOT(eventsBtn()));
    connect(m_exampleBtn,  SIGNAL(clicked()), SLOT(exampleBtn()));
    connect(m_settingsBtn, SIGNAL(clicked()), SLOT(settingsBtn()));
}

ScriptEditor::~ScriptEditor()
{
    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_STR, ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));
    delete ui;
}

void ScriptEditor::setSource(const QString &source)
{
    if(source.isEmpty())
        return;

    m_editor->setText(source);
    m_editor->setModified(false);
    m_contentChanged = false;
    m_fileChanged = false;
    m_changed = true;
}

QString ScriptEditor::getSource()
{
    return m_editor->getText();
}

int ScriptEditor::getEngine()
{
    return m_language;
}

int ScriptEditor::getEditorType() const
{
    return m_editor->getType();
}

void ScriptEditor::textChanged()
{
    if(!m_filename.isEmpty() && !ui->nameLabel->text().startsWith('*'))
    {
        ui->nameLabel->setText(ui->nameLabel->text().prepend('*'));
        QString title = tr("%1 - Script").arg(ui->nameLabel->text());
        setTabText(title);
        setWindowTitle(title);
    }
    m_changed = true;
    m_contentChanged = true;
    m_fileChanged = true;
}

void ScriptEditor::loadAct()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS),
                                                    filters[m_language]);
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return Utils::showErrorBox(tr("Failed to open \"%1!\"").arg(filename));

    m_editor->setText(QString::fromUtf8(file.readAll()));
    file.close();

    sConfig.set(CFG_STRING_ANALYZER_JS, filename);
    setFilename(filename);
    m_fileChanged = false;
}

void ScriptEditor::setFilename(const QString& filename)
{
    m_filename = filename;
    if(!m_filename.isEmpty())
    {
        ui->nameLabel->setText(filename.split("/").last());
        setTabText(tr("%1 - Script").arg(ui->nameLabel->text()));
        setWindowTitle(tr("%1 - Script").arg(ui->nameLabel->text()));
    }
    else
    {
        ui->nameLabel->setText(QString());
        setTabText(tr("Script"));
        setWindowTitle(tr("Script"));
    }
    m_editor->setModified(false);
}

void ScriptEditor::setLanguage(int lang)
{
    if(!m_changed)
    {
        static const QString def[ENGINE_MAX] = { "default.js", "default.py" };
        loadExample(def[lang]);
    }

    m_editor->setHighlighter((EditorHighlight)lang);
    m_language = lang;
    setFilename(QString());
}

void ScriptEditor::on_errorBtn_toggled(bool checked)
{
    ui->errorEdit->setVisible(checked);
    ui->resizeLine->setVisible(checked);
    sConfig.set(CFG_BOOL_SHOW_SCRIPT_ERROR, checked);
}

void ScriptEditor::loadExample(const QString& name)
{
    if(m_changed)
    {
        QMessageBox box(QMessageBox::Question, tr("Load example"), tr("Script was changed, do you really want to load an example?"),
                       (QMessageBox::Yes | QMessageBox::No), this);

        if(box.exec() == QMessageBox::No)
            return;
    }

    QFile file(":/examples/" + name);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    m_editor->setText(QString::fromUtf8(file.readAll()));
    m_changed = false;
}

void ScriptEditor::addError(const QString& error)
{
    ui->errorEdit->insertPlainText(error);
    ++m_errors;
    ui->errorBtn->setText(tr("Show errors (%1)").arg(m_errors));
}

void ScriptEditor::clearErrors()
{
    ui->errorEdit->clear();
    m_errors = 0;
    ui->errorBtn->setText(tr("Show errors (%1)").arg(m_errors));
}

void ScriptEditor::saveAct()
{
    if(m_filename.isEmpty())
        saveAs();
    else
        save(m_filename);
}

bool ScriptEditor::save(const QString& file)
{
    QFile f(file);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        Utils::showErrorBox(tr("Can't open file %1 for writing!").arg(file));
        return false;
    }

    f.write(m_editor->getText().toUtf8());

    setStatus(tr("File %1 was saved").arg(f.fileName().split("/").last()));
    setFilename(file);
    m_fileChanged = false;
    return true;
}

void ScriptEditor::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS),
                                                    filters[m_language]);

    if(filename.isEmpty())
        return;

    sConfig.set(CFG_STRING_ANALYZER_JS, filename);

    if(save(filename))
        setFilename(filename);
}

void ScriptEditor::load(QFile& f)
{
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Utils::showErrorBox(tr("Can't open file %1 for reading!").arg(m_filename));
        return;
    }
    m_editor->setText(QString::fromUtf8(f.readAll()));
    textChanged();
    m_fileChanged = false;
}

void ScriptEditor::setStatus(const QString &status)
{
    ui->statusLabel->setText(status);

    m_status_timer.start(3000);
}

void ScriptEditor::checkChange()
{
    if(m_fileChanged || m_filename.isEmpty())
        return;

    QFile f(m_filename);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray disk = MD5(f.readAll());
    QByteArray here = MD5(m_editor->getText().toUtf8());
    f.close();

    if(disk == here)
        return;

    QHash<QString, QByteArray>::iterator itr = m_ignoredFiles.find(m_filename);
    if(itr == m_ignoredFiles.end() || itr.value() != disk)
    {
        QMessageBox::StandardButtons btns = QMessageBox::Yes | QMessageBox::No | QMessageBox::Close;
        if(sWorkTabMgr.isBatchStarted())
        {
            if(sWorkTabMgr.getBatchVar("scripteditor_reloadall").toBool())
            {
                qDebug("Load file %s", m_filename.toStdString().c_str());
                load(f);
                return;
            }
            else if(sWorkTabMgr.getBatchVar("scripteditor_keepall").toBool())
            {
                m_ignoredFiles[m_filename] = disk;
                return;
            }
            btns |= QMessageBox::YesToAll | QMessageBox::NoToAll;
        }

        m_ignoreFocus = true;
        QMessageBox box(QMessageBox::Question, tr("File on disk was changed"),
                        tr("File on disk was changed. Do you want to reload it from disk?"), btns, this);
        box.setInformativeText(m_filename);
        box.setToolTip(m_filename);

        switch(box.exec())
        {
            case QMessageBox::Close:
                setFilename(QString());
                break;
            case QMessageBox::YesToAll:
                sWorkTabMgr.setBatchVar("scripteditor_reloadall", true);
                // fallthrough
            case QMessageBox::Yes:
                qDebug("Load file %s", m_filename.toStdString().c_str());
                load(f);
                break;
            case QMessageBox::NoToAll:
                sWorkTabMgr.setBatchVar("scripteditor_keepall", true);
                // fallthrough
            case QMessageBox::No:
                m_ignoredFiles[m_filename] = disk;
                break;
        }
        m_ignoreFocus = false;
    }
}

void ScriptEditor::focusChanged(QWidget *prev, QWidget *now)
{
    if(!prev && now && !m_ignoreFocus)
        checkChange();
}

void ScriptEditor::setEditor(int editorType)
{
    if(m_editor && m_editor->getType() == editorType)
        return;

    EditorWidget *w = EditorWidget::getEditor(editorType, this);
    if(!w)
        return;

    emit undoAvailable(false);
    emit redoAvailable(false);
    connect(w, SIGNAL(undoAvailable(bool)),    SIGNAL(undoAvailable(bool)));
    connect(w, SIGNAL(redoAvailable(bool)),    SIGNAL(redoAvailable(bool)));

    if(m_editor)
    {
        w->setText(m_editor->getText());
        delete m_editor;
    }
    w->setHighlighter((EditorHighlight)m_language);

    m_editor = w;
    int idx = ui->mainLayout->indexOf(ui->resizeLine);
    ui->mainLayout->insertWidget(idx, m_editor->getWidget(),
                                 100 - ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));

    connect(m_editor, SIGNAL(textChangedByUser()),    SLOT(textChanged()));
    connect(m_editor, SIGNAL(applyShortcutPressed()), SLOT(applyAct()));
    connect(this,     SIGNAL(undo()),       m_editor, SLOT(undo()));
    connect(this,     SIGNAL(redo()),       m_editor, SLOT(redo()));

    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_TYPE, editorType);
}

void ScriptEditor::exampleBtn()
{
    if(m_examples)
    {
        delete m_examples.data();
        m_examples.clear();
        return;
    }

    m_examples = new ExamplesPreview(m_language, this);
    m_examples->move(m_exampleBtn->mapToGlobal(QPoint(0, 0)) + QPoint(0, m_exampleBtn->height()));
    m_examples->show();
    m_examples->setFocus();

    connect(m_examples.data(), SIGNAL(destroyed()), SLOT(examplePreviewDestroyed()));
    connect(m_examples.data(), SIGNAL(openInEditor(QString)), SLOT(loadExample(QString)));
    connect(m_examples.data(), SIGNAL(openPreview(QString)), SIGNAL(openPreview(QString)));
}

void ScriptEditor::examplePreviewDestroyed()
{
    m_exampleBtn->setChecked(false);
}

bool ScriptEditor::onTabClose()
{
    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_STR, ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));

    if(m_fileChanged && !m_filename.isEmpty())
    {
        QMessageBox::StandardButtons btns = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
        if(sWorkTabMgr.isBatchStarted())
        {
            if(sWorkTabMgr.getBatchVar("scripteditor_saveall").toBool())
            {
                save(m_filename);
                return true;
            }
            else if(sWorkTabMgr.getBatchVar("scripteditor_discardall").toBool())
                return true;

            btns |= QMessageBox::YesAll | QMessageBox::NoAll;
        }

        QMessageBox box(QMessageBox::Question, tr("Script was changed"),
                        tr("File was changed, but not saved:"),
                        btns, this);
        box.setInformativeText(m_filename);
        switch(box.exec())
        {
            case QMessageBox::YesAll:
                sWorkTabMgr.setBatchVar("scripteditor_saveall", true);
                // fallthrough
            case QMessageBox::Yes:
                save(m_filename);
                return true;
            case QMessageBox::NoAll:
                sWorkTabMgr.setBatchVar("scripteditor_discardall", true);
                // fallthrough
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
        }
    }
    else if(m_contentChanged)
    {
        QMessageBox::StandardButtons btns = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
        if(sWorkTabMgr.isBatchStarted())
        {
            if(sWorkTabMgr.getBatchVar("scripteditor_applyall").toBool())
            {
                applyAct();
                return true;
            }
            else if(sWorkTabMgr.getBatchVar("scripteditor_ignoreall").toBool())
                return true;

            btns |= QMessageBox::YesAll | QMessageBox::NoAll;
        }

        QMessageBox box(QMessageBox::Question, tr("Script was changed"),
                        tr("Script was changed, but not applied. Apply?"),
                        btns, this);
        switch(box.exec())
        {
            case QMessageBox::YesAll:
                sWorkTabMgr.setBatchVar("scripteditor_applyall", true);
                // fallthrough
            case QMessageBox::Yes:
                applyAct();
                return true;
            case QMessageBox::NoAll:
                sWorkTabMgr.setBatchVar("scripteditor_ignoreall", true);
                // fallthrough
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
        }
    }
    return true;
}

void ScriptEditor::settingsBtn()
{
    if(m_settings)
    {
        delete m_settings.data();
        m_settings.clear();
        return;
    }

    m_settings = new SettingsPopup(this);
    m_settings->move(m_settingsBtn->mapToGlobal(QPoint(0, 0)) + QPoint(0, m_settingsBtn->height()));
    m_settings->show();
    m_settings->setFocus();

    connect(m_settings.data(), SIGNAL(destroyed()), SLOT(settingsDestroyed()));
}

void ScriptEditor::settingsDestroyed()
{
    m_settingsBtn->setChecked(false);
}

void ScriptEditor::applyAct()
{
    emit applySource();
    setStatus(tr("Source was applied."));
    m_contentChanged = false;
}

void ScriptEditor::eventsBtn()
{
    if(m_events)
    {
        delete m_events.data();
        m_events.clear();
        return;
    }

    m_events = new EventsPopup(this, m_area);
    m_events->move(m_eventsBtn->mapToGlobal(QPoint(0, 0)) + QPoint(0, m_eventsBtn->height()));
    m_events->show();
    m_events->setFocus();

    connect(m_events.data(), SIGNAL(destroyed()), SLOT(eventsDestroyed()));
    connect(m_events.data(), SIGNAL(addHandler(QString,QString)), SLOT(addEventHandler(QString,QString)));
}

void ScriptEditor::eventsDestroyed()
{
    m_eventsBtn->setChecked(false);
}

void ScriptEditor::addEventHandler(const QString& widgetTitle, const QString& event)
{
    QString source = getSource();
    switch(m_language) {
    case ENGINE_QTSCRIPT:
        source += QString("\n\nfunction %1_%2 {\n\n}\n").arg(widgetTitle).arg(event);
        break;
    case ENGINE_PYTHON:
        source += QString("\n\ndef %1_%2:\n    pass").arg(widgetTitle).arg(event);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    setSource(source);
    m_editor->scrollToBottom();
}

ExamplesPreview::ExamplesPreview(int engine, QWidget *parent) : QScrollArea(parent)
{
    setWindowFlags(windowFlags() | Qt::ToolTip);
    setAutoFillBackground(true);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setFixedSize(300, 300);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *widget = new QWidget(this);
    widget->setFixedWidth(280);
    QVBoxLayout *l = new QVBoxLayout(widget);
    l->setSpacing(0);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#c0daff"));
    setPalette(p);

    static const QStringList filters[ENGINE_MAX] =
    {
        (QStringList() << "*.js"),
        (QStringList() << "*.py")
    };

    QDir dir(":/examples");
    QStringList files = dir.entryList(filters[engine], QDir::NoFilter, QDir::Name);
    QFile f;
    for(int i = 0; i < files.size(); ++i)
    {
        f.setFileName(":/examples/" + files[i]);
        if(!f.open(QIODevice::ReadOnly))
            continue;

        ExamplePreviewItem *prev = new ExamplePreviewItem(files[i], f.readLine(), this);
        connect(prev, SIGNAL(openInEditor(QString)), SIGNAL(openInEditor(QString)));
        connect(prev, SIGNAL(openPreview(QString)),  SIGNAL(openPreview(QString)));

        l->addWidget(prev);
        l->addWidget(getSeparator());

        f.close();
    }
    setWidget(widget);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*, QWidget*)));
}

QFrame *ExamplesPreview::getSeparator()
{
    QFrame *res = new QFrame(this);
    res->setFrameStyle(QFrame::HLine | QFrame::Plain);
    return res;
}

void ExamplesPreview::focusChanged(QWidget *, QWidget *to)
{
    if(to != this)
        deleteLater();
}

ExamplePreviewItem::ExamplePreviewItem(const QString& name, QString line, QWidget *parent) : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QFont f;
    QVBoxLayout *l = new QVBoxLayout(this);

    QLabel *nameLabel = new QLabel(name.left(name.lastIndexOf('.')), this);
    f.setBold(true);
    f.setPointSize(12);
    nameLabel->setFont(f);

    line.chop(1);
    QLabel *lineLabel = new QLabel(line, this);
    f.setPointSize(9);
    f.setBold(false);
    lineLabel->setFont(f);
    lineLabel->setStyleSheet("color: green;");
    lineLabel->setWordWrap(true);

    QHBoxLayout *linkLayout = new QHBoxLayout;
    QLabel *linkToEditor = new QLabel(tr("<a href=\"%1\">Load to editor</a>").arg(name), this);
    QLabel *linkPreview = new QLabel(tr("<a href=\"%1\">Show preview</a>").arg(name), this);

    linkLayout->addStretch();
    linkLayout->addWidget(linkToEditor);
    linkLayout->addWidget(linkPreview);

    l->addWidget(nameLabel);
    l->addWidget(lineLabel);
    l->addLayout(linkLayout);

    connect(linkToEditor, SIGNAL(linkActivated(QString)), SIGNAL(openInEditor(QString)));
    connect(linkPreview,  SIGNAL(linkActivated(QString)), SIGNAL(openPreview(QString)));
    connect(linkToEditor, SIGNAL(linkActivated(QString)), parent, SLOT(deleteLater()));
    connect(linkPreview,  SIGNAL(linkActivated(QString)), parent, SLOT(deleteLater()));
}

ExamplePreviewTab::ExamplePreviewTab(const QString &name)
{
    QHBoxLayout *l = new QHBoxLayout(this);

    m_editor = EditorWidget::getEditor(sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE), this);
    if(!m_editor)
        m_editor = EditorWidget::getEditor(EDITOR_INTERNAL, this);

    m_editor->setReadOnly(true);
    l->addWidget(m_editor->getWidget());

    loadExample(name);
}

void ExamplePreviewTab::loadExample(const QString &name)
{
    setTabText(name + tr(" - example"));

    QFile f(":/examples/" + name);
    if(!f.open(QIODevice::ReadOnly))
    {
        m_editor->setText(tr("Could not load example!"));
        return;
    }

    if(name.endsWith(".py"))
        m_editor->setHighlighter(HIGHLIGHT_PYTHON);
    else
        m_editor->setHighlighter(HIGHLIGHT_JSCRIPT);

    m_editor->setText(QString::fromUtf8(f.readAll()));
}

ScriptEditorPopup::ScriptEditorPopup(ScriptEditor *editor) : QFrame(editor)
{
    setWindowFlags(windowFlags() | Qt::Popup);
    setAutoFillBackground(true);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#c0daff"));
    setPalette(p);

    setFocusPolicy(Qt::StrongFocus);

    setFrameStyle(QFrame::Box | QFrame::Plain);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*, QWidget*)));
}


void ScriptEditorPopup::focusChanged(QWidget *, QWidget *to)
{
    if(!to || (to != this && !isAncestorOf(to)))
        deleteLater();
}


bool ScriptEditorPopup::isAncestorOf(const QWidget *child) const
{
    while (child)
    {
        if (child == this)
            return true;
        child = child->parentWidget();
    }
    return false;
}

SettingsPopup::SettingsPopup(ScriptEditor *editor) : ScriptEditorPopup(editor)
{
    QGridLayout *l = new QGridLayout(this);

    QComboBox *langBox = new QComboBox(this);
    langBox->addItems(ScriptEngine::getEngineList());
    langBox->setCurrentIndex(editor->getEngine());

    m_editorBox = new QComboBox(this);
    EditorWidget::fillEditorBox(m_editorBox);

    for(int i = 0; i < m_editorBox->count(); ++i)
    {
        if(m_editorBox->itemData(i).toInt() == editor->getEditorType())
        {
            m_editorBox->setCurrentIndex(i);
            break;
        }
    }

    m_editorSett = new QPushButton(tr("Settings"), this);

    l->addWidget(new QLabel(tr("Language:"), this), 0, 0, Qt::AlignRight);
    l->addWidget(langBox, 0, 1);
    l->addWidget(new QLabel(tr("Editor:"), this), 1, 0, Qt::AlignRight);
    l->addWidget(m_editorBox, 1, 1);
    l->addWidget(m_editorSett, 1, 2);

    m_editorSett->setVisible(editor->getEditor()->hasSettings());

    adjustSize();

    connect(m_editorSett, SIGNAL(clicked()), editor->getEditor(), SLOT(settingsBtn()));
    connect(m_editorBox,  SIGNAL(currentIndexChanged(int)),       SLOT(editorChanged(int)));
    connect(langBox,      SIGNAL(currentIndexChanged(int)),editor,SLOT(setLanguage(int)));
}

void SettingsPopup::editorChanged(int idx)
{
    if(idx == -1)
        return;

    int type = m_editorBox->itemData(idx).toInt();

    disconnect(m_editorSett, 0, editor()->getEditor(), 0);
    editor()->setEditor(type);
    connect(m_editorSett, SIGNAL(clicked()), editor()->getEditor(), SLOT(settingsBtn()));
    m_editorSett->setVisible(editor()->getEditor()->hasSettings());

    adjustSize();
}

EventsPopup::EventsPopup(ScriptEditor *editor, WidgetArea *area) : ScriptEditorPopup(editor)
{
    m_area = area;

    m_widgetsBox = new QComboBox(this);
    m_eventsBox = new QComboBox(this);
    QPushButton *confirmBtn = new QPushButton(tr("Add handler"), this);

    auto widgets = m_area->getWidgets();

    QList<quint32> ids = widgets.keys();
    std::sort(ids.begin(), ids.end(), [&widgets](quint32 a, quint32 b) {
        return widgets[a]->getTitle() < widgets[b]->getTitle();
    });

    for(auto id : ids) {
        auto &w = widgets[id];
        QStringList events = w->getScriptEvents();

        if(!events.empty()) {
            m_widgetsBox->addItem(w->getTitle());
            m_widgetEvents.push_back(events);
        }
    }

    if(m_widgetsBox->count() == 0)
        confirmBtn->setEnabled(false);

    QGridLayout *l = new QGridLayout(this);
    l->addWidget(new QLabel(tr("Widget:"), this), 0, 0);
    l->addWidget(m_widgetsBox, 0, 1);
    l->addWidget(new QLabel(tr("Event:"), this), 0, 2);
    l->addWidget(m_eventsBox, 0, 3);
    l->addWidget(confirmBtn, 1, 2, 1, 2);

    widgetBoxIndexChanged(0);

    connect(confirmBtn, SIGNAL(clicked()), SLOT(addHandlerClicked()));
    connect(m_widgetsBox, SIGNAL(currentIndexChanged(int)), SLOT(widgetBoxIndexChanged(int)));
}

void EventsPopup::widgetBoxIndexChanged(int idx) {
    m_eventsBox->clear();
    if((size_t)idx < m_widgetEvents.size())
        m_eventsBox->addItems(m_widgetEvents[idx]);
    adjustSize();
}

void EventsPopup::addHandlerClicked()
{
    emit addHandler(m_widgetsBox->currentText(), m_eventsBox->currentText());
    deleteLater();
}
