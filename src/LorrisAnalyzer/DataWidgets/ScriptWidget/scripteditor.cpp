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
#include "ui_scripteditor.h"
#include "../../../common.h"

ScriptEditor::ScriptEditor(const QString& source, const QString& widgetName) :
    QDialog(),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    m_line_num = new LineNumber(this);
    ui->editLayout->insertWidget(0, m_line_num);

    new QScriptSyntaxHighlighter(ui->sourceEdit->document());

    ui->sourceEdit->setFont(Utils::getMonospaceFont());
    setWindowTitle(windowTitle() + widgetName);

    QMenuBar *menu = new QMenuBar(this);
    ui->verticalLayout->insertWidget(0, menu);

    QAction *loadAct = menu->addAction(tr("Load file..."));
    loadAct->setShortcut(QKeySequence("Ctrl+O"));

    QScrollBar *bar = ui->sourceEdit->verticalScrollBar();
    connect(bar,            SIGNAL(rangeChanged(int,int)),     SLOT(rangeChanged(int,int)));
    connect(bar,            SIGNAL(valueChanged(int)),         SLOT(sliderMoved(int)));
    connect(ui->buttonBox,  SIGNAL(clicked(QAbstractButton*)), SLOT(buttonPressed(QAbstractButton*)));
    connect(ui->sourceEdit, SIGNAL(textChanged()),             SLOT(textChanged()));
    connect(loadAct,        SIGNAL(triggered()),               SLOT(loadFile()));

    ui->sourceEdit->setPlainText(source);
    ui->sourceEdit->setTabStopWidth(ui->sourceEdit->fontMetrics().width(' ') * 4);
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

QString ScriptEditor::getSource()
{
    return ui->sourceEdit->toPlainText();
}

void ScriptEditor::buttonPressed(QAbstractButton *btn)
{
    switch(ui->buttonBox->buttonRole(btn))
    {
        case QDialogButtonBox::ApplyRole:  emit applySource(false); break;
        case QDialogButtonBox::AcceptRole: emit applySource(true);  break;
        default: break;
    }
}

void ScriptEditor::textChanged()
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

void ScriptEditor::loadFile()
{
    static const QString filters = tr("JavaScript file (*.js);;Any file (*.*)");
    QString filename = QFileDialog::getOpenFileName(this, tr("Load file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_JS), filters);

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

LineNumber::LineNumber(QWidget *parent) : QWidget(parent)
{
    setFont(Utils::getMonospaceFont());
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
