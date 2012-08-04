/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qscriptsyntaxhighlighter_p.h>
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
#include "engines/pythonhighlighter.h"

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

    updateExampleList();

    ui->errorBtn->setChecked(sConfig.get(CFG_BOOL_SHOW_SCRIPT_ERROR));
    on_errorBtn_toggled(ui->errorBtn->isChecked());

    setFilename(filename);
    m_contentChanged = false;
    checkChange();

    Utils::loadWindowParams(this, sConfig.get(CFG_STRING_SCRIPT_WND_PARAMS));
}

ScriptEditor::~ScriptEditor()
{
    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_STR, ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));
    delete ui;
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
        case QDialogButtonBox::RejectRole: emit rejected(); return;
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
    ui->exampleBox->setCurrentIndex(0);
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
}

void ScriptEditor::setFilename(const QString& filename)
{
    m_filename = filename;
    if(!m_filename.isEmpty())
    {
        ui->nameLabel->setText(filename.split("/").last());
        setTabText(tr("%1 - Script").arg(ui->nameLabel->text()));
    }
    else
    {
        ui->nameLabel->setText(QString());
        setTabText(tr("Script"));
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

    m_editor->setEngine(idx);

    updateExampleList();
    setFilename(QString());
}

void ScriptEditor::on_errorBtn_toggled(bool checked)
{
    ui->errorEdit->setShown(checked);
    ui->resizeLine->setShown(checked);
    sConfig.set(CFG_BOOL_SHOW_SCRIPT_ERROR, checked);
}

void ScriptEditor::on_exampleBox_activated(int index)
{
    if(index == 0)
        return;

    if(m_changed)
    {
        QMessageBox box(QMessageBox::Question, tr("Load example"), tr("Script was changed, do you really want to load an example?"),
                       (QMessageBox::Yes | QMessageBox::No), this);

        if(box.exec() == QMessageBox::No)
        {
            ui->exampleBox->setCurrentIndex(0);
            return;
        }
    }

    QFile file(":/examples/" + ui->exampleBox->currentText());
    if(!file.open(QIODevice::ReadOnly))
        return;

    m_editor->setText(QString::fromUtf8(file.readAll()));
    ui->exampleBox->setCurrentIndex(index);
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

void ScriptEditor::updateExampleList()
{
    static const QStringList filters[ENGINE_MAX] =
    {
        (QStringList() << "*.js"),
        (QStringList() << "*.py")
    };

    while(ui->exampleBox->count() > 1)
        ui->exampleBox->removeItem(1);

    QDir dir(":/examples");
    ui->exampleBox->addItems(dir.entryList(filters[ui->langBox->currentIndex()], QDir::NoFilter, QDir::Name));
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
    w->setEngine(ui->langBox->currentIndex());

    m_editor = w;
    ui->mainLayout->insertWidget(1, m_editor->getWidget(),
                                 100 - ui->mainLayout->stretch(ui->mainLayout->indexOf(ui->errorEdit)));
    ui->editSettBtn->setVisible(m_editor->hasSettings());

    connect(m_editor, SIGNAL(textChangedByUser()), SLOT(textChanged()));
    connect(m_editor, SIGNAL(applyShortcutPressed()), ui->buttonBox->button(QDialogButtonBox::Apply), SLOT(animateClick()));
    connect(ui->editSettBtn, SIGNAL(clicked()), m_editor, SLOT(settingsBtn()));

    sConfig.set(CFG_QUINT32_SCRIPTEDITOR_TYPE, idx);
}

LineNumber::LineNumber(QWidget *parent) : QWidget(parent)
{
#ifdef Q_OS_MAC
    setFont(Utils::getMonospaceFont(12));
#else
    setFont(Utils::getMonospaceFont());
#endif
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setMinimumSize(5, 5);

    m_char_h = fontMetrics().height();
    m_line_num = 0;
    m_last_w = 0;
    m_scroll = 0;
}

void LineNumber::setLineNum(int lineNum)
{
    m_line_num = lineNum;
    update();
}

void LineNumber::setScroll(int line)
{
    m_scroll = line;
    update();
}

void LineNumber::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);

    int h = 5;
    for(int line = m_scroll; h+m_char_h < height() && line < m_line_num; ++line)
    {
        QString text;
        text.setNum(line+1);

        int text_w = fontMetrics().width(text);
        if(m_last_w < text_w)
        {
            m_last_w = text_w;
            setFixedWidth(text_w);
        }

        painter.drawText(0, h, m_last_w, m_char_h, Qt::AlignRight, text);
        h += m_char_h;
    }
}

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent)
{

}

EditorWidget *EditorWidget::getEditor(int type, QWidget *parent)
{
    switch(type)
    {
        case EDITOR_INTERNAL: return new EditorWidgetLorris(parent);
#ifdef USE_KATE
        case EDITOR_KATE: return new EditorWidgetKate(parent);
#endif
#ifdef USE_QSCI
        case EDITOR_QSCINTILLA: return new EditorWidgetQSci(parent);
#endif
    }
    return NULL;
}

void EditorWidget::fillEditorBox(QComboBox *box)
{
    box->addItem(tr("Internal (basic)"), QVariant(EDITOR_INTERNAL));

#ifdef USE_KATE
    box->addItem(tr("Kate (advanced)"), QVariant(EDITOR_KATE));
#endif

#ifdef USE_QSCI
    box->addItem(tr("QScintilla (advanced)"), QVariant(EDITOR_QSCINTILLA));
#endif
}

EditorWidgetLorris::EditorWidgetLorris(QWidget *parent) : EditorWidget(parent)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    this->setStyleSheet("margin: 0px; padding: 0px");

    m_lineNumber = new LineNumber(this);
    m_edit = new QPlainTextEdit(this);

    l->addWidget(m_lineNumber);
    l->addWidget(m_edit, 1);

    m_highlighter = NULL;

    m_edit->setTabStopWidth(m_edit->fontMetrics().width(' ') * 4);

#ifdef Q_OS_MAC
    m_edit->setFont(Utils::getMonospaceFont(12));
#else
    m_edit->setFont(Utils::getMonospaceFont());
#endif

    QScrollBar *bar = m_edit->verticalScrollBar();
    connect(bar,    SIGNAL(rangeChanged(int,int)),           SLOT(rangeChanged(int,int)));
    connect(bar,    SIGNAL(valueChanged(int)), m_lineNumber, SLOT(setScroll(int)));
    connect(m_edit->document(), SIGNAL(contentsChange(int,int,int)), SLOT(contentsChange(int,int,int)));
}

void EditorWidgetLorris::rangeChanged(int, int)
{
    m_lineNumber->setScroll(m_edit->verticalScrollBar()->value());
}

void EditorWidgetLorris::contentsChange(int /*position*/, int charsRemoved, int charsAdded)
{
    if(charsRemoved != charsAdded)
        emit textChangedByUser();
    m_lineNumber->setLineNum(m_edit->document()->lineCount());
}

void EditorWidgetLorris::setEngine(int idx)
{
    delete m_highlighter;
    switch(idx)
    {
        case ENGINE_QTSCRIPT:
            m_highlighter = new QScriptSyntaxHighlighter(m_edit->document());
            break;
        case ENGINE_PYTHON:
            m_highlighter = new PythonHighlighter(m_edit->document());
            break;
        default:
            m_highlighter = NULL;
            break;
    }
}

#ifdef USE_KATE

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/configinterface.h>
#include <kconfig.h>

EditorWidgetKate::EditorWidgetKate(QWidget *parent) : EditorWidget(parent)
{
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    m_doc = editor->createDocument(this);
    m_view = m_doc->createView(this);

    KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface*>(m_doc);
    loadSettings(iface, CFG_VARIANT_KATE_SETTINGS_DOC);

    iface = qobject_cast<KTextEditor::ConfigInterface*>(m_view);
    iface->setConfigValue("line-numbers", true);
    loadSettings(iface, CFG_VARIANT_KATE_SETTINGS_VIEW);

    m_view->installEventFilter(this);

    // FIXME: Two config files. Great.
    KConfig config("lorrisrc");
    m_doc->editor()->readConfig(&config);


    connect(m_doc, SIGNAL(textChanged(KTextEditor::Document*)), SLOT(modified(KTextEditor::Document*)));
}

EditorWidgetKate::~EditorWidgetKate()
{
    save();
}

bool EditorWidgetKate::hasSettings()
{
    return m_doc->editor()->configDialogSupported();
}

QWidget *EditorWidgetKate::getWidget()
{
    return m_view;
}

void EditorWidgetKate::setText(const QString &text)
{
    m_doc->setText(text);
}

QString EditorWidgetKate::getText() const
{
    return m_doc->text();
}

void EditorWidgetKate::setEngine(int idx)
{
    switch(idx)
    {
        case ENGINE_QTSCRIPT:
            m_doc->setMode("JavaScript");
            break;
        case ENGINE_PYTHON:
            m_doc->setMode("Python");
            break;
        default:
            m_doc->setMode("");
            break;
    }
}

void EditorWidgetKate::settingsBtn()
{
    m_doc->editor()->configDialog((QWidget*)parent());

    KConfig config("lorrisrc");
    m_doc->editor()->writeConfig(&config);

    save();
}

void EditorWidgetKate::save()
{
    KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface*>(m_doc);
    saveSettings(iface, CFG_VARIANT_KATE_SETTINGS_DOC);

    iface = qobject_cast<KTextEditor::ConfigInterface*>(m_view);
    saveSettings(iface, CFG_VARIANT_KATE_SETTINGS_VIEW);
}

void EditorWidgetKate::saveSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg)
{
    if(!iface)
        return;

    QStringList keys = iface->configKeys();
    QHash<QString, QVariant> values;
    for(int i = 0; i < keys.size(); ++i)
    {
        // FIXME: QColor breaks whole config entry on linux. Qt bug?
        // "QVariant::load: unable to load type 67."
        // "QVariant::save: unable to save type 67."
        if(iface->configValue(keys[i]).type() == QVariant::Color)
            continue;

        values[keys[i]] = iface->configValue(keys[i]);
    }

    sConfig.set(cfg, values);
}

void EditorWidgetKate::loadSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg)
{
    if(!iface)
        return;

    QHash<QString, QVariant> values = sConfig.get(cfg).toHash();
    for(QHash<QString, QVariant>::iterator itr = values.begin(); itr != values.end(); ++itr)
        iface->setConfigValue(itr.key(), *itr);
}

void EditorWidgetKate::modified(KTextEditor::Document *)
{
    emit textChangedByUser();
}

bool EditorWidgetKate::eventFilter(QObject *, QEvent *ev)
{
    switch(ev->type())
    {
        case QEvent::ShortcutOverride:
        {
            QKeyEvent *kev = (QKeyEvent*)ev;
            if(kev->key() == Qt::Key_F5)
            {
                kev->accept();
                emit applyShortcutPressed();
                return true;
            }
            break;
        }
        default: break;
    }
    return false;
}

#endif // USE_KATE

#ifdef USE_QSCI

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerpython.h>

EditorWidgetQSci::EditorWidgetQSci(QWidget *parent) : EditorWidget(parent)
{
    m_editor = new QsciScintilla(this);
    m_editor->setMarginLineNumbers(QsciScintilla::NumberMargin, true);
    m_editor->setMarginWidth(QsciScintilla::NumberMargin, "12322");
    m_editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    m_editor->setUtf8(true);

    connect(m_editor, SIGNAL(modificationChanged(bool)), SLOT(modified(bool)));
}

EditorWidgetQSci::~EditorWidgetQSci()
{
    QsciLexer *lex = m_editor->lexer();
    m_editor->setLexer(NULL);
    delete lex;

    delete m_editor;
}

QString EditorWidgetQSci::getText() const
{
    return m_editor->text();
}

void EditorWidgetQSci::setText(const QString &text)
{
    m_editor->setText(text);
    m_editor->setModified(false);
}

void EditorWidgetQSci::setEngine(int idx)
{
    QsciLexer *lex = m_editor->lexer();
    m_editor->setLexer(NULL);
    delete lex;

    switch(idx)
    {
        case ENGINE_QTSCRIPT:
            m_editor->setLexer(new QsciLexerJavaScript);
            m_editor->lexer()->setFont(Utils::getMonospaceFont(10), QsciLexerJavaScript::Comment);
            m_editor->lexer()->setFont(Utils::getMonospaceFont(10), QsciLexerJavaScript::CommentLine);
            break;
        case ENGINE_PYTHON:
            m_editor->setLexer(new QsciLexerPython);
            m_editor->lexer()->setFont(Utils::getMonospaceFont(10), QsciLexerPython::Comment);
            m_editor->lexer()->setFont(Utils::getMonospaceFont(10), QsciLexerPython::CommentBlock);
            break;
        default:
            m_editor->setLexer(NULL);
            return;
    }
}

QWidget *EditorWidgetQSci::getWidget()
{
    return m_editor;
}

void EditorWidgetQSci::modified(bool mod)
{
    if(mod)
        emit textChangedByUser();
}

void EditorWidgetQSci::setModified(bool modded)
{
    m_editor->setModified(modded);
}

#endif // USE_QSCI
