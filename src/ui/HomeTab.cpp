/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPushButton>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QCommandLinkButton>

#include "HomeTab.h"
#include "mainwindow.h"
#include "../WorkTab/WorkTabMgr.h"
#include "chooseconnectiondlg.h"

#include "../ui/ui_hometab.h"

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
#ifdef Q_OS_MAC
        QFont fnt = btn->font();
        fnt.setPointSize(11);
        btn->setFont(fnt);
#endif
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
    {
        emit tabOpened();
        sWorkTabMgr.AddWorkTab(info);
    }
}

void HomeTab::on_openConnManagerLink_linkActivated(const QString &link)
{
    ChooseConnectionDlg dialog(this);
    dialog.exec();
}
