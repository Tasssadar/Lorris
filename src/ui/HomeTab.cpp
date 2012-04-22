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

#include <QPushButton>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QCommandLinkButton>

#include "HomeTab.h"
#include "mainwindow.h"
#include "WorkTab/WorkTabMgr.h"

#include "ui_hometab.h"

HomeTab::HomeTab(QWidget *parent) : QWidget(parent), ui(new Ui::HomeTab)
{
    ui->setupUi(this);

    QLayoutItem * vertStretch = ui->tabButtonsWidget->layout()->takeAt(0);

    WorkTabMgr::InfoList const & infoList = sWorkTabMgr.GetWorkTabInfos();
    for (int i = 0; i < infoList.size(); ++i)
    {
        WorkTabInfo * info = infoList[i];

        QCommandLinkButton * btn = new QCommandLinkButton(ui->tabButtonsWidget);
        m_buttonInfoMap[btn] = info;

        btn->setText(info->GetName());
        btn->setDescription(info->GetDescription());
        btn->setMaximumHeight(100);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        ui->tabButtonsWidget->layout()->addWidget(btn);

        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }

    ui->tabButtonsWidget->layout()->addItem(vertStretch);
}

HomeTab::~HomeTab()
{
    delete ui;
}

void HomeTab::buttonClicked()
{
    WorkTabInfo * info = m_buttonInfoMap.value(this->sender());
    if (info)
        sWorkTabMgr.AddWorkTab(info);
}
