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

    ui->resizeLine->setOrientation(false);
    ui->resizeLine->setResizeLayout(ui->mainLayout);
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit), sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit)-2, 100 - sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->resizeLine->updateStretch();

    quint32 editor_cfg = sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE);
    EditorWidget::fillEditorBox(ui->editorBox);

#if defined(USE_KATE) || defined(USE_QSCI)
    ui->editorBox->setCurrentIndex(1);
#else
    ui->editorBox->setCurrentIndex(0);
#endif

    if(editor_cfg != UINT_MAX)
        ui->editorBox->setCurrentIndex(editor_cfg);

    if(ui->editorBox->count() == 1)
    {
        ui->editorLabel->hide();
        ui->editorBox->hide();
    }

    ui->buttonBox->button(QDialogButtonBox::Apply)->setShortcut(QKeySequence("F5"));

    m_errors = 0;
    m_ignoreNextFocus = false;
    m_ignoreFocus = false;

    m_editor->setText(source);
    m_changed = !source.isNull();

    ui->langBox->addItems(ScriptEngine::getEngineList());
    ui->langBox->setCurrentIndex(type);

    QAction *saveAs = new QAction(tr("Save as..."), this);
    ui->saveBtn->addAction(saveAs);

    connect(&m_status_timer, SIGNAL(timeout()), SLOT(clearStatus()));
    connect(saveAs, SIGNAL(triggered()), SLOT(saveAs()));
    connect(qApp,   SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));

    ui->errorBtn->setChecked(sConfig.get(CFG_BOOL_SHOW_SCRIPT_ERROR));
    on_errorBtn_toggled(ui->errorBtn->isChecked());

    setFilename(filename);
    m_contentChanged = false;
    m_fileChanged = false;
    checkChange();

    Utils::loadWindowParams(this, sConfig.get(CFG_STRING_SCRIPT_WND_PARAMS));
}

ScriptEditor::~ScriptEditor()
{
    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_STR, ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));
    delete ui;
}

void ScriptEditor::setSource(const QString &source)
{
    m_editor->setText(source);
    m_editor->setModified(false);
    m_contentChanged = false;
    m_fileChanged = false;
    m_changed = !source.isNull();
}

QString ScriptEditor::getSource()
{
    return m_editor->getText();
}

int ScriptEditor::getEngine()
{
    return ui->langBox->currentIndex();
}

void ScriptEditor::on_buttonBox_clicked(QAbstractButton *btn)
{
    sConfig.set(CFG_STRING_SCRIPT_WND_PARAMS, Utils::saveWindowParams(this));
    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_STR, ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));

    switch(ui->buttonBox->buttonRole(btn))
    {
        case QDialogButtonBox::ApplyRole:  emit applySource(false); break;
        case QDialogButtonBox::AcceptRole: emit applySource(true);  break;
        case QDialogButtonBox::RejectRole:
            if(onTabClose())
                emit rejected();
            return;
        default: return;
    }
    m_contentChanged = false;

    if(!m_filename.isEmpty())
        save(m_filename);
}

void ScriptEditor::reject()
{
    if(!m_contentChanged)
        return emit rejected();

    QMessageBox box(QMessageBox::Question, tr("Script changed"), tr("Script was changed, but not applied."),
                    QMessageBox::Cancel | QMessageBox::Close | QMessageBox::Apply, this);
    box.setInformativeText(tr("Do you really want to close editor?"));
    switch(box.exec())
    {
        case QMessageBox::Close:
            return emit rejected();
        case QMessageBox::Cancel:
            return;
        case QMessageBox::Apply:
            applySource(true);
            m_contentChanged = false;
            return;
    }
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

void ScriptEditor::on_loadBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS),
                                                    filters[ui->langBox->currentIndex()]);
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

void ScriptEditor::on_langBox_currentIndexChanged(int idx)
{
    if(!m_changed)
    {
        static const QString defaultCode[ENGINE_MAX] = {
            tr("// You can use clearTerm() and appendTerm(string) to set term content\n"
            "// You can use sendData(Array of ints) to send data to device. It expects array of uint8s\n\n"
            "// This function gets called on data received\n"
            "// it should return string, which is automatically appended to terminal\n"
            "function onDataChanged(data, dev, cmd, index) {\n"
            "\treturn \"\";\n"
            "}\n\n"
            "// This function is called on key press in terminal.\n"
            "// Param is string\n"
            "function onKeyPress(key) {\n"
            "\n"
            "}\n"),

            tr("# You can use terminal.clear() and terminal.appendText(string) to set term content\n"
            "# You can use lorris.sendData(QByteArray) to send data to device.\n"
            "\n"
            "# This function gets called on data received\n"
            "# it should return string, which is automatically appended to terminal\n"
            "def onDataChanged(data, dev, cmd, index):\n"
            "\treturn \"\";\n"
            "\n"
            "# This function is called on key press in terminal.\n"
            "# Param is string\n"
            "def onKeyPress(key):\n"
            "\treturn;\n")
        };

        m_editor->setText(defaultCode[idx]);
        m_changed = false;
    }

    m_editor->setHighlighter((EditorHighlight)idx);

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

void ScriptEditor::on_saveBtn_clicked()
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
                                                    filters[ui->langBox->currentIndex()]);

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
    if(m_contentChanged || m_filename.isEmpty())
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

void ScriptEditor::on_editorBox_currentIndexChanged(int idx)
{
    if(idx == -1)
        return;

    int editorType = ui->editorBox->itemData(idx).toInt();
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
    w->setHighlighter((EditorHighlight)ui->langBox->currentIndex());

    m_editor = w;
    ui->mainLayout->insertWidget(1, m_editor->getWidget(),
                                 100 - ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));
    ui->editSettBtn->setVisible(m_editor->hasSettings());

    connect(m_editor, SIGNAL(textChangedByUser()), SLOT(textChanged()));
    connect(m_editor, SIGNAL(applyShortcutPressed()), ui->buttonBox->button(QDialogButtonBox::Apply), SLOT(animateClick()));
    connect(ui->editSettBtn, SIGNAL(clicked()), m_editor, SLOT(settingsBtn()));

    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_TYPE, idx);
}

void ScriptEditor::on_exampleBtn_clicked()
{
    if(m_examples)
    {
        delete m_examples.data();
        return;
    }

    m_examples = new ExamplesPreview(ui->langBox->currentIndex(), this);
    m_examples->move(ui->exampleBtn->mapToGlobal(QPoint(0, 0)) + QPoint(0, ui->exampleBtn->height()));
    m_examples->show();
    m_examples->setFocus();

    connect(m_examples.data(), SIGNAL(destroyed()), SLOT(examplePreviewDestroyed()));
    connect(m_examples.data(), SIGNAL(openInEditor(QString)), SLOT(loadExample(QString)));
    connect(m_examples.data(), SIGNAL(openPreview(QString)), SIGNAL(openPreview(QString)));
}

void ScriptEditor::examplePreviewDestroyed()
{
    ui->exampleBtn->setChecked(false);
}

bool ScriptEditor::onTabClose()
{
    if(!m_fileChanged || m_filename.isEmpty())
        return true;

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
    return true;
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
