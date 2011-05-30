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

#include "mainwindow.h"
#include "plot.h"
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
    QAction* actionQuit = new QAction(tr("&Quit"), this);
    QAction* actionAbout = new QAction(tr("About Lorris.."), this);

    actionNewTab->setShortcut(QKeySequence("Ctrl+N"));

    connect(actionNewTab, SIGNAL(triggered()), this, SLOT(NewTab()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    // connect(actionAboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()));

    menuFile->addAction(actionNewTab);
    menuFile->addAction(actionQuit);
    menuHelp->addAction(actionAbout);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);

    setMenuBar(menuBar);

    //Tabs
    tabs = new QTabWidget;

    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", tabs);
    connect(newTabBtn, SIGNAL(clicked()), this, SLOT(NewTab()));

    tabs->setCornerWidget(newTabBtn);
    tabs->setMovable(true);
    tabs->setTabsClosable(true);

    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));

    setCentralWidget(tabs);
    resize(950, 700);

    windowCount = 0;
}

MainWindow::~MainWindow()
{
    delete tabs;
}

QString MainWindow::getVersionString()
{
    QString ver = "Lorris - build ";
    ver.append(QString::number(REVISION));
    return ver;
}

void MainWindow::NewTab()
{
    QString label = tr("New tab ");
    label.append(QString::number(++windowCount));
    int index = tabs->addTab(new Plot, label);
    if(tabs->count() > 1)
        tabs->setCurrentIndex(index);
}

void MainWindow::CloseTab(int index)
{
    delete tabs->widget(index);
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
        case 0:
            box->close();
            this->close();
            return true;
        case 1:
        default:
            box->close();
            return false;
    }
}

void MainWindow::About()
{

}
