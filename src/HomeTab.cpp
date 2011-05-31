#include <QPushButton>
#include <QTabWidget>

#include "HomeTab.h"
#include "mainwindow.h"

HomeTab::HomeTab(MainWindow *parent)
{
    m_mainWindow = parent;

    QPushButton *button = new QPushButton("New tab", this);
    button->setFixedSize(200, 50);
    button->move(parent->geometry().center());
    connect(button, SIGNAL(clicked()), this, SLOT(NewTab()));
    button->show();
}

HomeTab::~HomeTab()
{

}

void HomeTab::NewTab()
{
    m_mainWindow->ExtNewTab();
}
