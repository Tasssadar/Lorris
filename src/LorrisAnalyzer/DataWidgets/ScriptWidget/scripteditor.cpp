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

#define MD5(x) QCryptographicHash::hash(x, QCryptographicHash::Md5)

static const QString filters[ENGINE_MAX] =
{
    ScriptEditor::tr("JavaScript file (*.js);;Any file (*.*)"),
    ScriptEditor::tr("Python file (*.py);;Any file (*.*)"),
};

ScriptEditor::ScriptEditor(const QString& source, const QString& filename, int type) :
    ChildTab(NULL), ui(new Ui::ScriptEditor), m_editor(NULL)
{
    ui->setupUi(this);

    m_exampleBtn = new QPushButton(tr("Examples"), this);
    m_settingsBtn = new QPushButton(tr("Settings"), this);
    m_exampleBtn->setCheckable(true);
    m_settingsBtn->setCheckable(true);

    QLabel *docLabel = new QLabel(this);
    docLabel->setTextFormat(Qt::RichText);
    docLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    docLabel->setOpenExternalLinks(true);
    docLabel->setText(tr("<a href=\"http://tasssadar.github.com/Lorris/doc/\">Documentation</a>"));

    QToolBar *bar = new QToolBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(16, 16));

    QAction *load = bar->addAction(QIcon(":/actions/open"), tr("Load"), this, SLOT(loadAct()));
    QAction *save = bar->addAction(QIcon(":/actions/save"), tr("Save"), this, SLOT(saveAct()));
    QAction *saveAs = bar->addAction(QIcon(":/actions/save-as"), tr("Save as..."), this, SLOT(saveAs()));
    bar->addSeparator();
    QAction *apply = bar->addAction(QIcon(":/icons/start"), tr("Apply"), this, SLOT(applyAct()));
    bar->addSeparator();
    bar->addWidget(m_exampleBtn);
    bar->addWidget(m_settingsBtn);
    bar->addSeparator();
    bar->addWidget(docLabel);

    load->setShortcut(QKeySequence("Ctrl+O"));
    load->setToolTip(tr("Load (Ctrl+O)"));
    save->setShortcut(QKeySequence("Ctrl+S"));
    save->setToolTip(tr("Save (Ctrl+S)"));
    saveAs->setShortcut(QKeySequence("Ctrl+Shift+S"));
    saveAs->setToolTip(tr("Save as... (Ctrl+Shift+S)"));
    apply->setShortcut(QKeySequence("F5"));
    apply->setToolTip(tr("Apply (F5)"));

    ui->mainLayout->insertWidget(0, bar);

    quint32 editor_cfg = sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE);

#ifdef USE_KATE
    setEditor(EDITOR_KATE);
#elif defined(USE_QSCI)
    setEditor(EDITOR_QSCINTILLA);
#else
    setEditor(EDITOR_INTERNAL);
#endif

    if(editor_cfg != UINT_MAX)
        setEditor(editor_cfg);

    ui->resizeLine->setOrientation(false);
    ui->resizeLine->setResizeLayout(ui->mainLayout);
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit), sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit)-2, 100 - sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->resizeLine->updateStretch();

    m_errors = 0;
    m_ignoreNextFocus = false;
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
        setTabText(tr("%1 - Script").arg(ui->nameLabel->text()));
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
    ui->errorEdit->setShown(checked);
    ui->resizeLine->setShown(checked);
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
    if(!file.open(QIODevice::ReadOnly))
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

    QByteArray disk = MD5(f.readAll().replace('\r', ""));
    QByteArray here = MD5(m_editor->getText().toUtf8().replace('\r', ""));
    f.close();

    if(disk != here)
    {
        m_ignoreFocus = true;
        QMessageBox box(QMessageBox::Question, tr("File on disk was changed"),
                        tr("File on disk was changed. What do you want to do?"), QMessageBox::NoButton, this);
        box.setInformativeText(m_filename);
        box.setToolTip(m_filename);

        box.addButton(tr("Reload from disk"), QMessageBox::AcceptRole);
        box.addButton(tr("Ignore"), QMessageBox::RejectRole);
        box.addButton(QMessageBox::Close);

        switch(box.exec())
        {
            case QMessageBox::Close:
                setFilename(QString());
                break;
            case QMessageBox::AcceptRole:
                if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
                    Utils::showErrorBox(tr("Can't open file %1 for reading!").arg(m_filename));
                m_editor->setText(QString::fromUtf8(f.readAll()));
                break;
            case QMessageBox::RejectRole:
                m_ignoreNextFocus = true;
                break;
        }
        m_ignoreFocus = false;
    }
}

void ScriptEditor::focusChanged(QWidget *prev, QWidget *now)
{
    if(!prev && now && !m_ignoreFocus)
    {
        if(m_ignoreNextFocus)
            m_ignoreNextFocus = false;
        else
            checkChange();
    }
}

void ScriptEditor::setEditor(int editorType)
{
    if(m_editor && m_editor->getType() == editorType)
        return;

    EditorWidget *w = EditorWidget::getEditor(editorType, this);
    if(!w)
        return;

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

    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_TYPE, editorType);
}

void ScriptEditor::exampleBtn()
{
    if(m_examples)
    {
        delete m_examples.data();
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
        QMessageBox box(QMessageBox::Question, tr("Script was changed"),
                        tr("File was changed, but not saved:"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        box.setInformativeText(m_filename);
        switch(box.exec())
        {
            case QMessageBox::Save:
                save(m_filename);
                return true;
            case QMessageBox::Discard:
                return true;
            case QMessageBox::Cancel:
                return false;
        }
    }
    else if(m_contentChanged)
    {
        QMessageBox box(QMessageBox::Question, tr("Script was changed"),
                        tr("Script was changed, but not applied"),
                        QMessageBox::Apply | QMessageBox::Discard | QMessageBox::Cancel, this);
        switch(box.exec())
        {
            case QMessageBox::Apply:
                applyAct();
                return true;
            case QMessageBox::Discard:
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

SettingsPopup::SettingsPopup(ScriptEditor *editor) : QFrame(editor)
{
    setWindowFlags(windowFlags() | Qt::Popup);
    setAutoFillBackground(true);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#c0daff"));
    setPalette(p);

    setFocusPolicy(Qt::StrongFocus);

    setFrameStyle(QFrame::Box | QFrame::Plain);

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

    m_editorSett->setShown(editor->getEditor()->hasSettings());

    adjustSize();

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*, QWidget*)));
    connect(m_editorSett, SIGNAL(clicked()), editor->getEditor(), SLOT(settingsBtn()));
    connect(m_editorBox,  SIGNAL(currentIndexChanged(int)),       SLOT(editorChanged(int)));
    connect(langBox,      SIGNAL(currentIndexChanged(int)),editor,SLOT(setLanguage(int)));
}

void SettingsPopup::focusChanged(QWidget *, QWidget *to)
{
    if(!to || (to != this && !isAncestorOf(to)))
        deleteLater();
}

void SettingsPopup::editorChanged(int idx)
{
    if(idx == -1)
        return;

    int type = m_editorBox->itemData(idx).toInt();

    disconnect(m_editorSett, 0, editor()->getEditor(), 0);
    editor()->setEditor(type);
    connect(m_editorSett, SIGNAL(clicked()), editor()->getEditor(), SLOT(settingsBtn()));
    m_editorSett->setShown(editor()->getEditor()->hasSettings());

    adjustSize();
}

bool SettingsPopup::isAncestorOf(const QWidget *child) const
{
    while (child)
    {
        if (child == this)
            return true;
        child = child->parentWidget();
    }
    return false;
}
