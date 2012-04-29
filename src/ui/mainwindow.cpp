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
#include "HomeTab.h"
#include "homedialog.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../WorkTab/WorkTabInfo.h"
#include "../revision.h"
#include "../config.h"
#include "../ui/chooseconnectiondlg.h"

QLocale::Language langs[] = { QLocale::system().language(), QLocale::English, QLocale::Czech };

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(getVersionString());
    setMinimumSize(600, 500);
    setWindowState(Qt::WindowMaximized);
    setWindowIcon(QIcon(":/icons/icon.png"));

    // menu bar
    menuBar = new QMenuBar(this);
    menuFile = new QMenu(tr("&File"), this);
    menuHelp = new QMenu(tr("&Help"), this);

    QAction* actionQuit = new QAction(tr("&Quit"), this);
    QAction* actionAbout = new QAction(tr("About Lorris..."), this);
    QAction* actionConnectionManager = new QAction(tr("Connection &manager..."), this);

    QMenu* menuLang = new QMenu(tr("Language"), this);

    QSignalMapper *langSignals = new QSignalMapper(this);
    connect(langSignals, SIGNAL(mapped(int)), this, SLOT(langChanged(int)));


    for(quint8 i = 0; i < 3; ++i)
    {
        QString langName = QLocale::languageToString(langs[i]);
        if(i == 0)
            langName.prepend(tr("Same as OS - "));

        QAction* actLang = menuLang->addAction(langName);
        actLang->setCheckable(true);
        m_lang_menu.push_back(actLang);

        langSignals->setMapping(actLang, i);
        connect(actLang, SIGNAL(triggered()), langSignals, SLOT(map()));

        if(i == 0)
            menuLang->addSeparator();
    }

    setStatusBar(new QStatusBar(this));

    quint32 curLang = sConfig.get(CFG_QUINT32_LANGUAGE);
    if(curLang >= m_lang_menu.size())
        curLang = 0;
    m_lang_menu[curLang]->setChecked(true);

    actionQuit->setShortcut(QKeySequence("Alt+F4"));

    connect(actionQuit,     SIGNAL(triggered()), this, SLOT(close()));
    connect(actionAbout,    SIGNAL(triggered()), this, SLOT(About()));
    connect(actionConnectionManager, SIGNAL(triggered()), this, SLOT(OpenConnectionManager()));

    QMenu * menuFileNew = menuFile->addMenu(tr("&New"));

    {
        WorkTabMgr::InfoList const & infos = sWorkTabMgr.GetWorkTabInfos();
        for (int i = 0; i < infos.size(); ++i)
        {
            WorkTabInfo * info = infos[i];
            QAction * action = new QAction(info->GetName(), this);
            m_actionTabInfoMap[action] = info;
            connect(action, SIGNAL(triggered()), this, SLOT(NewSpecificTab()));
            menuFileNew->addAction(action);
        }
    }

    menuFile->addAction(actionConnectionManager);
    menuFile->addAction(actionQuit);
    menuHelp->addAction(actionAbout);
    menuHelp->addMenu(menuLang);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);

    // FIXME: replace by addSeparator() when supported
    QAction *sep = menuBar->addAction("|");
    sep->setEnabled(false);

    setMenuBar(menuBar);

    //Tabs
    TabView *tabWidget = sWorkTabMgr.CreateWidget(this);
    connect(tabWidget, SIGNAL(changeMenu(quint32)),                    SLOT(changeMenu(quint32)));
    connect(tabWidget, SIGNAL(statusBarMsg(QString,int)), statusBar(), SLOT(showMessage(QString,int)));
    connect(tabWidget, SIGNAL(newTab()),                               SLOT(newTab()));

    sWorkTabMgr.OpenHomeTab();
    setCentralWidget(tabWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::show(const QStringList& openFiles)
{
    QWidget::show();

    for(QStringList::const_iterator itr = openFiles.begin(); itr != openFiles.end(); ++itr)
        sWorkTabMgr.openTabWithFile(*itr);
}

QString MainWindow::getVersionString()
{
    QString ver = "Lorris v" + QString::number(REVISION);
    return ver;
}

void MainWindow::OpenConnectionManager()
{
    ChooseConnectionDlg dialog(0, this);
    dialog.exec();
}

void MainWindow::About()
{
    QString text = tr("Lorris version " VERSION);
    if(text.contains("-dev"))
        text += ", git revision " + QString::number(REVISION);

    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle(tr("About Lorris"));
    box->setText(text);
    box->setIcon(QMessageBox::Information);
    box->exec();
    delete box;
}

void MainWindow::changeMenu(quint32 id)
{
    WorkTab *tab = sWorkTabMgr.getWorkTab(id);
    if(!tab)
        return;

    menuBar->clear();
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);

    // FIXME: replace by addSeparator() when supported
    QAction *sep = menuBar->addAction("|");
    sep->setEnabled(false);

    m_tab_menu = tab->getMenu();
    for(quint8 i = 0; i < m_tab_menu.size(); ++i)
        menuBar->addMenu(m_tab_menu[i]);
}

void MainWindow::langChanged(int idx)
{
    sConfig.set(CFG_QUINT32_LANGUAGE, idx);
    for(quint8 i = 0; i < m_lang_menu.size(); ++i)
        m_lang_menu[i]->setChecked(i == idx);

    QMessageBox box(this);
    box.setWindowTitle(tr("Restart"));
    box.setText(tr("You need to restart Lorris for this change to take effect"));
    box.setIcon(QMessageBox::Information);
    box.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!sWorkTabMgr.onTabsClose())
        event->ignore();
    else
        event->accept();
}

void MainWindow::NewSpecificTab()
{
    WorkTabInfo * info = m_actionTabInfoMap.value(this->sender());
    if (info)
        sWorkTabMgr.AddWorkTab(info);
}

void MainWindow::newTab()
{
    HomeDialog dialog(this);
    dialog.exec();
}
