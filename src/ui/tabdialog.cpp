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

#include <QMessageBox>

#include "common.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "WorkTab/WorkTab.h"
#include "tabdialog.h"

#include "ui_tabdialog.h"

TabDialog::TabDialog(QWidget *parent) : QDialog(parent, Qt::WindowFlags(0)), ui(new Ui::TabDialog)
{
    ui->setupUi(this);

    m_cur_con = -1;
    m_con_builder = NULL;

    WorkTabMgr::InfoList *tabs = sWorkTabMgr.GetWorkTabInfos();
    for(WorkTabMgr::InfoList::iterator i = tabs->begin(); i != tabs->end(); ++i)
        ui->modulesList->addItem((*i)->GetName());

    connect(ui->modulesList,  SIGNAL(currentRowChanged(int)),   SLOT(PluginSelected(int)));
    connect(ui->conBox,       SIGNAL(currentIndexChanged(int)), SLOT(FillConOptions(int)));
    connect(ui->createButton, SIGNAL(clicked()),                SLOT(CreateTab()));

    quint32 lastSelected = sConfig.get(CFG_QUINT32_TAB_TYPE);
    if(lastSelected >= (quint32)sWorkTabMgr.GetWorkTabInfos()->size())
        lastSelected = 0;
    ui->modulesList->setCurrentRow(lastSelected);
}

TabDialog::~TabDialog()
{
    delete m_con_builder;
    delete ui;
}

void TabDialog::PluginSelected(int index)
{
    WorkTabMgr::InfoList *tabs = sWorkTabMgr.GetWorkTabInfos();
    quint8 conn = tabs->at(index)->GetConType();

    ui->descLabel->setText(tabs->at(index)->GetDescription());
    ui->conBox->clear();

    if(conn & CON_MSK(CONNECTION_SERIAL_PORT))
    {
        ui->conBox->addItem(tr("Serial port"), CONNECTION_SERIAL_PORT);
        if(sConMgr.isAnyShupito())
            ui->conBox->addItem(tr("Shupito tunnel"), CONNECTION_SHUPITO);
    }

    if(conn & CON_MSK(CONNECTION_TCP_SOCKET))
        ui->conBox->addItem(tr("TCP socket"), CONNECTION_TCP_SOCKET);

    if(conn & CON_MSK(CONNECTION_FILE))
        ui->conBox->addItem(tr("None (Load data from File)"), CONNECTION_FILE);

    quint32 lastConn = sConfig.get(CFG_QUINT32_CONNECTION_TYPE);
    if(lastConn != MAX_CON_TYPE)
    {
        for(quint8 i = 0; i < ui->conBox->count(); ++i)
        {
            if((quint32)ui->conBox->itemData(i).toInt() == lastConn)
            {
                ui->conBox->setCurrentIndex(i);
                break;
            }
        }
    }
}

void TabDialog::FillConOptions(int index)
{
    int conType = ui->conBox->itemData(index).toInt();
    if(conType == m_cur_con)
        return;

    m_cur_con = conType;

    delete m_con_builder;
    m_con_builder = sConMgr.getConBuilder(conType, ui->modulesList->currentIndex().row(), this);

    if(!m_con_builder)
        return;

    connect(m_con_builder, SIGNAL(setCreateBtnStatus(bool)),  SLOT(setCreateButtonState(bool)));
    connect(m_con_builder, SIGNAL(connectionFailed(QString)), SLOT(connectFailed(QString)));
    connect(m_con_builder, SIGNAL(connectionSucces(Connection*,QString,WorkTabInfo*)),
                           SLOT  (connectionSucces(Connection*,QString,WorkTabInfo*)));

    QWidget *conLabel = ui->conLayout->itemAtPosition(0, 0)->widget();
    QWidget *conBox   = ui->conLayout->itemAtPosition(0, 1)->widget();

    for(quint8 i = 0; i < ui->conLayout->rowCount(); ++i)
    {
        QLayoutItem *item = ui->conLayout->itemAtPosition(i, 0);
        for(quint8 col = 0; item != NULL;)
        {
            ui->conLayout->removeItem(item);
            if(i != 0)
                delete item->widget();
            delete item;
            item = ui->conLayout->itemAtPosition(i, ++col);
        }
    }

    m_con_builder->addOptToTabDialog(ui->conLayout);

    ui->conLayout->addWidget(conLabel, 0, 0);
    ui->conLayout->addWidget(conBox, 0, 1, 1, ui->conLayout->columnCount()-1);
}

void TabDialog::CreateTab()
{
    WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(ui->modulesList->currentIndex().row());
    m_con_builder->CreateConnection(info);
}

void TabDialog::setCreateButtonState(bool connecting)
{
    ui->createButton->setText(connecting ? tr("Connecting...") : tr("Create tab"));
    ui->createButton->setEnabled(!connecting);
}

void TabDialog::connectFailed(const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(tr("Error!"));
    box.setText(text);
    box.setIcon(QMessageBox::Critical);
    box.exec();
}

void TabDialog::connectionSucces(Connection* con, const QString& tabName, WorkTabInfo *info)
{
    WorkTab *tab = info->GetNewTab();
    sWorkTabMgr.AddWorkTab(tab, tabName);
    tab->setConnection(con);
    sConMgr.AddCon(con->getType(), con);

    sConfig.set(CFG_QUINT32_TAB_TYPE, ui->modulesList->currentIndex().row());
    sConfig.set(CFG_QUINT32_CONNECTION_TYPE, con->getType());

    close();
    tab->onTabShow();
}
