/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
#include <QSignalMapper>
#include <QLocale>
#include <QTranslator>
#include <QCloseEvent>

#include "mainwindow.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../WorkTab/WorkTabInfo.h"
#include "../revision.h"
#include "../config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(getVersionString());
    setMinimumSize(600, 500);
    setWindowState(Qt::WindowMaximized);
    setWindowIcon(QIcon(":/icons/icon.png"));

    m_win7.init(winId());
    Utils::setWin7(&m_win7);

    setStatusBar(new QStatusBar(this));
    Utils::setStatusBar(statusBar());

    TabView *tabWidget = sWorkTabMgr.CreateWidget(this);
    connect(tabWidget, SIGNAL(statusBarMsg(QString,int)), statusBar(), SLOT(showMessage(QString,int)));
    connect(tabWidget, SIGNAL(close()),                                SLOT(close()));

    sWorkTabMgr.OpenHomeTab();
    setCentralWidget(tabWidget);
}

MainWindow::~MainWindow()
{
    Utils::setWin7(NULL);
    Utils::setStatusBar(NULL);
}

void MainWindow::show(const QStringList& openFiles)
{
    QMainWindow::show();

    for(QStringList::const_iterator itr = openFiles.begin(); itr != openFiles.end(); ++itr)
        sWorkTabMgr.openTabWithFile(*itr);
}

bool MainWindow::winEvent(MSG *message, long *result)
{
    return m_win7.winEvent(message, result);
}

QString MainWindow::getVersionString()
{
    QString ver = "Lorris v" + QString::number(REVISION);
    return ver;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!sWorkTabMgr.onTabsClose())
        event->ignore();
    else
        event->accept();
}
