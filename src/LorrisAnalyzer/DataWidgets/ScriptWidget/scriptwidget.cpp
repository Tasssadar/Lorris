/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QLabel>

#include "scriptwidget.h"
#include "scripteditor.h"
#include "scriptenv.h"
#include "terminal.h"

ScriptWidget::ScriptWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Script"));
    setIcon(":/dataWidgetIcons/script.png");

    m_widgetType = WIDGET_SCRIPT;
    m_editor = NULL;


    adjustSize();
    setMinimumSize(width(), width());

    m_terminal = new Terminal(this);
    layout->setContentsMargins(5, 0, 5, 5);
    layout->addWidget(m_terminal, 4);

    m_env = NULL;
}

ScriptWidget::~ScriptWidget()
{
    delete m_editor;
}

void ScriptWidget::setUp(AnalyzerDataStorage *storage)
{
    DataWidget::setUp(storage);

    QAction *src_act = contextMenu->addAction(tr("Set source..."));
    connect(src_act,    SIGNAL(triggered()),                 this,       SLOT(setSourceTriggered()));

    m_env = new ScriptEnv((AnalyzerDataArea*)parent(), getId(),  this);

    connect(m_terminal, SIGNAL(keyPressedASCII(QByteArray)), m_env,      SLOT(keyPressed(QByteArray)));
    connect(m_env,      SIGNAL(clearTerm()),                 m_terminal, SLOT(clear()));
    connect(m_env,      SIGNAL(appendTerm(QByteArray)),      m_terminal, SLOT(appendText(QByteArray)));
    connect(m_env,      SIGNAL(SendData(QByteArray)),        this,       SIGNAL(SendData(QByteArray)));
}

void ScriptWidget::newData(analyzer_data *data, quint32 index)
{
    // FIXME: is it correct?
    //if(!m_updating)
    //    return;

    QString res = m_env->dataChanged(data, index);
    if(!res.isEmpty())
        m_terminal->appendText(res.toAscii());
}

void ScriptWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::saveWidgetInfo(file);

    // source
    file->writeBlockIdentifier("scriptWSource");
    {
        QByteArray source = m_env->getSource().toUtf8();
        quint32 len = source.length();

        file->write((char*)&len, sizeof(quint32));
        file->write(source.data(), len);
    }

    // terminal data
    file->writeBlockIdentifier("scriptWTerm");
    {
        QByteArray data = m_terminal->getData();
        quint32 len = data.length();

        file->write((char*)&len, sizeof(quint32));
        file->write(data.data());
    }
}

void ScriptWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::loadWidgetInfo(file);

    // source
    if(file->seekToNextBlock("scriptWSource", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QString source = QString::fromUtf8(file->read(size), size);
        try
        {
            m_env->setSource(source);
        } catch(const QString&) { }
    }

    // terminal data
    if(file->seekToNextBlock("scriptWTerm", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QByteArray data(file->read(size));
        m_terminal->appendText(data);
    }
}

void ScriptWidget::setSourceTriggered()
{
    delete m_editor;

    m_editor = new ScriptEditor(m_env->getSource(), getTitle());
    m_editor->show();

    connect(m_editor, SIGNAL(applySource(bool)), SLOT(sourceSet(bool)));
}

void ScriptWidget::sourceSet(bool close)
{
    try
    {
        m_env->setSource(m_editor->getSource());

        if(close)
        {
            m_editor->deleteLater();
            m_editor = NULL;
        }
    }
    catch(const QString& text)
    {
        Utils::ThrowException(text, m_editor);
    }
}

void ScriptWidget::moveEvent(QMoveEvent *)
{
    if(m_env)
        m_env->setPos(pos().x(), pos().y());
}

void ScriptWidget::resizeEvent(QResizeEvent *)
{
    if(m_env)
        m_env->setSize(size());
}

ScriptWidgetAddBtn::ScriptWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Script"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/script.png"));

    m_widgetType = WIDGET_SCRIPT;
}
