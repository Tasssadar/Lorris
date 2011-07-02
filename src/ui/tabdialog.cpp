#include <QHBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>

#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "tabdialog.h"
#include "mainwindow.h"

TabDialog::TabDialog(QWidget *parent) : QDialog(parent, Qt::WindowFlags(0))
{
    setFixedSize(600, 400);
    this->setModal(false);
    layout = new QHBoxLayout(this);

    pluginsBox = new QComboBox(this);
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    for(std::vector<WorkTabInfo*>::iterator i = tabs->begin(); i != tabs->end(); ++i)
        pluginsBox->addItem((*i)->GetName());
    connect(pluginsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PluginSelected(int)));

    conLine = new QLineEdit(this);

    conBox = new QComboBox(this);
    connect(conBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ConnectionSelected(int)));
    PluginSelected(pluginsBox->currentIndex());

    QPushButton *create = new QPushButton("Create", this);
    connect(create, SIGNAL(clicked()), this, SLOT(CreateTab()));

    layout->addWidget(pluginsBox);
    layout->addWidget(conBox);
    layout->addWidget(conLine);
    layout->addWidget(create);
    this->setLayout(layout);
}

TabDialog::~TabDialog()
{
    delete pluginsBox;
    delete conBox;
    delete layout;
}

void TabDialog::PluginSelected(int index)
{
    conBox->clear();
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    uint8_t conn = tabs->at(index)->GetConType();

    if(conn & CONNECTION_SOCKET)      conBox->addItem("Socket");
    if(conn & CONNECTION_SERIAL_PORT) conBox->addItem("Serial port");
    if(conn & CONNECTION_FILE)        conBox->addItem("File");
}

void TabDialog::ConnectionSelected(int index)
{
    if(conBox->itemText(index) == "Socket")
        conLine->setText("TODO: socket path");
    else if(conBox->itemText(index) == "Serial port")
        conLine->setText("/dev/rfcomm0");
    else if(conBox->itemText(index) == "File")
        conLine->setText(QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Any file (*.*)")));\
}

void TabDialog::CreateTab()
{
    WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(pluginsBox->currentIndex());
    ((MainWindow*)parent())->AddTab(info->GetNewTab(), info->GetName());
    this->close();
}


