#include <QWebView>
#include <QPushButton>
#include <QString>
#include <QMessageBox>
#include <QCloseEvent>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <QLibrary>
#include <dlfcn.h>

#include "mainwindow.h"
#include "HomeTab.h"
#include "WorkTabMgr.h"
#include "revision.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(getVersionString());

    // menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* menuFile = new QMenu(tr("&File"));
    QMenu* menuHelp = new QMenu(tr("&Help"));

    QAction* actionNewTab = new QAction(tr("&New tab.."), this);
    QAction* actionCloseTab = new QAction(tr("&Close tab"), this);
    QAction* actionQuit = new QAction(tr("&Quit"), this);
    QAction* actionAbout = new QAction(tr("About Lorris.."), this);

    actionNewTab->setShortcut(QKeySequence("Ctrl+T"));
    actionQuit->setShortcut(QKeySequence("Alt+F4"));
    actionCloseTab->setShortcut(QKeySequence("Ctrl+W"));

    connect(actionNewTab, SIGNAL(triggered()), this, SLOT(NewTab()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(QuitButton()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(actionCloseTab, SIGNAL(triggered()), this, SLOT(CloseTab()));

    menuFile->addAction(actionNewTab);
    menuFile->addAction(actionCloseTab);
    menuFile->addAction(actionQuit);
    menuHelp->addAction(actionAbout);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);

    setMenuBar(menuBar);

    //Tabs
    tabs = new QTabWidget(this);
    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", tabs);
    connect(newTabBtn, SIGNAL(clicked()), this, SLOT(NewTab()));

    tabs->setCornerWidget(newTabBtn);
    tabs->setMovable(true);
    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));
    resize(950, 700);
    OpenHomeTab();
    setCentralWidget(tabs);
    windowCount = 0;

    tabMgr = new WorkTabMgr();
}

MainWindow::~MainWindow()
{
    delete tabs;
    delete tabMgr;
}

QString MainWindow::getVersionString()
{
    QString ver = "Lorris [" + QString::number(REVISION) + "]";
    return ver;
}

void MainWindow::OpenHomeTab()
{
    tabs->addTab(new HomeTab(this), "Home");
}

void MainWindow::NewTab()
{
   /* QString label = tr("New tab ");
    label.append(QString::number(++windowCount));
    int index = tabs->addTab(new Plot, label);
    if(tabs->count() > 1)
    {
        tabs->setTabsClosable(true);
        tabs->setCurrentIndex(index);
    } */
    //TODO
}

void MainWindow::CloseTab(int index)
{
    if(index == -1)
        index = tabs->currentIndex();
    delete tabs->widget(index);
    if(tabs->count() < 1)
    {
        tabs->setTabsClosable(false);
        OpenHomeTab();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(Quit())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::Quit()
{
    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle("Exit Lorris?");
    box->setText("Do you really want to leave Lorris?");
    box->addButton("Yes", QMessageBox::YesRole);
    box->addButton("No", QMessageBox::NoRole);
    int ret = box->exec();
    switch(ret)
    {
        case 0: return true;
        case 1:
        default: return false;
    }
}

void MainWindow::QuitButton()
{
    this->close();
}

void MainWindow::About()
{
    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle("About Lorris");
    box->setText("Lorris revision " + QString::number(REVISION));
    box->show();
}
