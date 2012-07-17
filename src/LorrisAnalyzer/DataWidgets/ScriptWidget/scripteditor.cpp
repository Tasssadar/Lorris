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

ScriptEditor::ScriptEditor(const QString& source, const QString& filename, int type, const QString &widgetName) :
    ChildTab(NULL), ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    m_line_num = new LineNumber(this);
    ui->editLayout->insertWidget(0, m_line_num);

    ui->resizeLine->setOrientation(false);
    ui->resizeLine->setResizeLayout(ui->mainLayout);
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit), sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->mainLayout->setStretch(ui->mainLayout->indexOf(ui->errorEdit)-2, 100 - sConfig.get(CFG_QUINT32_SCRIPTEDITOR_STR));
    ui->resizeLine->updateStretch();

    m_highlighter = NULL;
    m_errors = 0;
    m_ignoreNextFocus = false;
    m_ignoreFocus = false;

#ifdef Q_OS_MAC
    ui->sourceEdit->setFont(Utils::getMonospaceFont(12));
#else
    ui->sourceEdit->setFont(Utils::getMonospaceFont());
#endif
    setWindowTitle(windowTitle() + widgetName);

    QScrollBar *bar = ui->sourceEdit->verticalScrollBar();
    connect(bar,            SIGNAL(rangeChanged(int,int)),     SLOT(rangeChanged(int,int)));
    connect(bar,            SIGNAL(valueChanged(int)),         SLOT(sliderMoved(int)));
    connect(ui->sourceEdit->document(), SIGNAL(contentsChange(int,int,int)),
                                        SLOT(contentsChange(int,int,int)));

    ui->sourceEdit->setPlainText(source);
    ui->sourceEdit->setTabStopWidth(ui->sourceEdit->fontMetrics().width(' ') * 4);

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
    return ui->sourceEdit->toPlainText();
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

void ScriptEditor::on_sourceEdit_textChanged()
{
    m_line_num->setLineNum(ui->sourceEdit->document()->lineCount());
}

void ScriptEditor::contentsChange(int /*position*/, int charsRemoved, int charsAdded)
{
    if(charsRemoved != charsAdded)
    {
        m_changed = true;
        m_contentChanged = true;
        ui->exampleBox->setCurrentIndex(0);
    }
}

void ScriptEditor::sliderMoved(int val)
{
    m_line_num->setScroll(val);
}

void ScriptEditor::rangeChanged(int, int)
{
    m_line_num->setScroll(ui->sourceEdit->verticalScrollBar()->value());
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
        return Utils::ThrowException(tr("Failed to open \"%1!\"").arg(filename));

    ui->sourceEdit->clear();
    ui->sourceEdit->setPlainText(QString::fromUtf8(file.readAll()));
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

        ui->sourceEdit->setPlainText(defaultCode[idx]);
        m_changed = false;
    }

    delete m_highlighter;
    switch(idx)
    {
        case ENGINE_QTSCRIPT:
            m_highlighter = new QScriptSyntaxHighlighter(ui->sourceEdit->document());
            break;
        case ENGINE_PYTHON:
            m_highlighter = new PythonHighlighter(ui->sourceEdit->document());
            break;
        default:
            m_highlighter = NULL;
            break;
    }
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

    ui->sourceEdit->setPlainText(file.readAll());
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
        Utils::ThrowException(tr("Can't open file %1 for writing!").arg(file));
        return false;
    }

    f.write(ui->sourceEdit->toPlainText().toUtf8());

    setStatus(tr("File %1 was saved").arg(f.fileName().split("/").last()));
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

    QByteArray disk = MD5(f.readAll());
    QByteArray here = MD5(ui->sourceEdit->toPlainText().toUtf8());
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
                    Utils::ThrowException(tr("Can't open file %1 for reading!").arg(m_filename));
                ui->sourceEdit->setPlainText(QString::fromUtf8(f.readAll()));
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
