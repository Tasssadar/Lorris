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
#include <QHBoxLayout>
#include <QLibrary>
#include <QObjectList>

#include "mainwindow.h"
#include "HomeTab.h"
#include "WorkTab.h"
#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "ui/tabdialog.h"
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
    tabs = new QTabWidget(this);
    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", tabs);
    connect(newTabBtn, SIGNAL(clicked()), this, SLOT(NewTab()));

    tabs->setCornerWidget(newTabBtn);
    tabs->setMovable(true);
    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));

    OpenHomeTab();
    setCentralWidget(tabs);
    windowCount = 0;

    //Initialize singletons
    //new WorkTabMgr;
    //sConMgr.
}

MainWindow::~MainWindow()
{
    const QList<QObject*> list = children();
    for(QList<QObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
        delete *it;
    //delete tabs;
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
    TabDialog *dialog = new TabDialog(this);
    dialog->exec();
    delete dialog;
}

void MainWindow::AddTab(WorkTab *tab, QString label)
{
    int index = tabs->addTab(tab->GetTab(this), label);
    m_workTabs.insert(std::make_pair<uint8_t, WorkTab*>(index, tab));
    if(tabs->count() > 1)
    {
        tabs->setTabsClosable(true);
        tabs->setCurrentIndex(index);
    }
}

void MainWindow::CloseTab(int index)
{
    if(index == -1)
        index = tabs->currentIndex();

    if(m_workTabs.find(index) != m_workTabs.end())
    {
        std::map<uint8_t, WorkTab*>::iterator itr = m_workTabs.find(index);
        delete itr->second;
        m_workTabs.erase(itr);
    }
    else
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
    box->setWindowTitle("About Lorris");
    box->setText("Lorris revision " + QString::number(REVISION));
    box->setIcon(QMessageBox::Information);
    box->exec();
    delete box;
}
