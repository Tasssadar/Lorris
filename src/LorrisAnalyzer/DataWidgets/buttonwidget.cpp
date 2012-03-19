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

#include <QInputDialog>

#include "buttonwidget.h"

ButtonWidget::ButtonWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Button"));
    setIcon(":/dataWidgetIcons/button.png");

    m_widgetType = WIDGET_BUTTON;

    m_button = new QPushButton(tr("Button"), this);
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->setContentsMargins(5, 0, 5, 5);
    layout->addWidget(m_button, 4);

    adjustSize();
}

void ButtonWidget::setUp(AnalyzerDataStorage *storage)
{
    DataWidget::setUp(storage);

    QAction *btnText = contextMenu->addAction(tr("Set button text..."));

    connect(btnText,  SIGNAL(triggered()), SLOT(setButtonName()));
    connect(m_button, SIGNAL(clicked()),   SLOT(buttonClicked()));
}

void ButtonWidget::buttonClicked()
{
    emit scriptEvent(getTitle() + "_clicked");
}

void ButtonWidget::setButtonName()
{
    QString name = QInputDialog::getText(this, tr("Button text"), tr("Enter new button text"));
    if(name.isEmpty())
        return;
    setButtonName(name);
}

void ButtonWidget::setButtonName(const QString &name)
{
    m_button->setText(name);
}

void ButtonWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("buttonWText");
    {
        QByteArray text = m_button->text().toUtf8();
        quint32 len = text.length();

        file->write((char*)&len, sizeof(quint32));
        file->write(text.data(), len);
    }
}

void ButtonWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("buttonWText", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QString text = QString::fromUtf8(file->read(size), size);
        m_button->setText(text);
    }
}

ButtonWidgetAddBtn::ButtonWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Button"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/button.png"));

    m_widgetType = WIDGET_BUTTON;
}
