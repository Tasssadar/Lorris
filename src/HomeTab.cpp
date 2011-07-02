#include <QPushButton>
#include <QTabWidget>
#include <QHBoxLayout>

#include "HomeTab.h"
#include "mainwindow.h"

HomeTab::HomeTab(MainWindow *parent)
{
    m_mainWindow = parent;

    QPushButton *button = new QPushButton("New tab", this);
    button->setFixedSize(200, 50);

    connect(button, SIGNAL(clicked()), this, SLOT(NewTab()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(button);
    this->setLayout(layout);

}

HomeTab::~HomeTab()
{

}

void HomeTab::NewTab()
{
    m_mainWindow->ExtNewTab();
}
