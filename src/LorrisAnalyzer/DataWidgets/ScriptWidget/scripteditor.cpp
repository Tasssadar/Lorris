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

#include "scripteditor.h"
#include "../../../common.h"
#include "engines/scriptengine.h"

ScriptEditor::ScriptEditor(const QString& source, int type, const QString &widgetName) :
    QDialog(),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    m_line_num = new LineNumber(this);
    ui->editLayout->insertWidget(0, m_line_num);

    m_highlighter = NULL;

#ifdef Q_OS_MAC
    ui->sourceEdit->setFont(Utils::getMonospaceFont(12));
#else
    ui->sourceEdit->setFont(Utils::getMonospaceFont());
#endif
    setWindowTitle(windowTitle() + widgetName);

    QScrollBar *bar = ui->sourceEdit->verticalScrollBar();
    connect(bar,            SIGNAL(rangeChanged(int,int)),     SLOT(rangeChanged(int,int)));
    connect(bar,            SIGNAL(valueChanged(int)),         SLOT(sliderMoved(int)));

    ui->sourceEdit->setPlainText(source);
    ui->sourceEdit->setTabStopWidth(ui->sourceEdit->fontMetrics().width(' ') * 4);

    ui->langBox->addItems(ScriptEngine::getEngineList());
    ui->langBox->setCurrentIndex(type);
}

ScriptEditor::~ScriptEditor()
{
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
    switch(ui->buttonBox->buttonRole(btn))
    {
        case QDialogButtonBox::ApplyRole:  emit applySource(false); break;
        case QDialogButtonBox::AcceptRole: emit applySource(true);  break;
        default: break;
    }
}

void ScriptEditor::on_sourceEdit_textChanged()
{
    m_line_num->setLineNum(ui->sourceEdit->document()->lineCount());
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
    static const QString filters[ENGINE_MAX] =
    {
        tr("JavaScript file (*.js);;Any file (*.*)"),
        tr("Python file (*.py);;Any file (*.*)"),
    };

    QString filename = QFileDialog::getOpenFileName(this, tr("Load file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS),
                                                    filters[ui->langBox->currentIndex()]);
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return Utils::ThrowException(tr("Failed to open \"%1!\"").arg(filename));

    ui->sourceEdit->clear();
    ui->sourceEdit->setPlainText(QString(file.readAll()));
    file.close();

    sConfig.set(CFG_STRING_ANALYZER_JS, filename);
}

void ScriptEditor::on_langBox_currentIndexChanged(int idx)
{
    delete m_highlighter;
    switch(idx)
    {
        case ENGINE_QTSCRIPT:
        {
            m_highlighter = new QScriptSyntaxHighlighter(ui->sourceEdit->document());
            break;
        }
        case ENGINE_PYTHON:
        default:
            m_highlighter = NULL;
            break;
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
