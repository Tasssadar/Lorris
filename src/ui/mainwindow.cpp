#include <QPushButton>
#include <QString>
#include <QMessageBox>
#include <QCloseEvent>
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
#include <QHBoxLayout>
#include <QObjectList>

#include "mainwindow.h"
#include "HomeTab.h"
#include "WorkTab/WorkTab.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "tabdialog.h"
#include "revision.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(getVersionString());
    setMinimumSize(700, 500);
    setWindowState(Qt::WindowMaximized);

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
    sWorkTabMgr.CreateWidget(this);
    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", sWorkTabMgr.getWi());
    connect(newTabBtn, SIGNAL(clicked()), this, SLOT(NewTab()));

    sWorkTabMgr.getWi()->setCornerWidget(newTabBtn);
    sWorkTabMgr.getWi()->setMovable(true);
    connect(sWorkTabMgr.getWi(), SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));

    sWorkTabMgr.OpenHomeTab();
    setCentralWidget(sWorkTabMgr.getWi());
}

MainWindow::~MainWindow()
{
    //delete singletons
    WorkTabMgr::Destroy();
    ConnectionMgr::Destroy();

    const QList<QObject*> list = children();
    for(QList<QObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
        delete *it; 
}

QString MainWindow::getVersionString()
{
    QString ver = "Lorris v" + QString::number(REVISION);
    return ver;
}

void MainWindow::NewTab()
{
    sWorkTabMgr.NewTabDialog();
}

void MainWindow::CloseTab(int index)
{
    if(index == -1)
        index = sWorkTabMgr.getWi()->currentIndex();
    sWorkTabMgr.removeTab(index);
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
    box->setWindowTitle(tr("Exit Lorris?"));
    box->setText(tr("Do you really want to leave Lorris?"));
    box->addButton(tr("Yes"), QMessageBox::YesRole);
    box->addButton(tr("No"), QMessageBox::NoRole);
    box->setIcon(QMessageBox::Question);
    int ret = box->exec();
    switch(ret)
    {
        case 0: return true;
        case 1:
        default: return false;
    }
    delete box;
}

void MainWindow::QuitButton()
{
    this->close();
}

void MainWindow::About()
{
    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle(tr("About Lorris"));
    box->setText(tr("Lorris revision ") + QString::number(REVISION));
    box->setIcon(QMessageBox::Information);
    box->exec();
    delete box;
}
