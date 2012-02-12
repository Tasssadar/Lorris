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

#include "mainwindow.h"
#include "HomeTab.h"
#include "WorkTab/WorkTab.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "tabdialog.h"
#include "revision.h"
#include "config.h"

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

    QAction* actionNewTab = new QAction(tr("&New tab.."), this);
    QAction* actionCloseTab = new QAction(tr("&Close tab"), this);
    QAction* actionQuit = new QAction(tr("&Quit"), this);
    QAction* actionAbout = new QAction(tr("About Lorris..."), this);

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

    quint32 curLang = sConfig.get(CFG_QUINT32_LANGUAGE);
    if(curLang >= m_lang_menu.size())
        curLang = 0;
    m_lang_menu[curLang]->setChecked(true);

    actionNewTab->setShortcut(QKeySequence("Ctrl+T"));
    actionQuit->setShortcut(QKeySequence("Alt+F4"));
    actionCloseTab->setShortcut(QKeySequence("Ctrl+W"));

    connect(actionNewTab,   SIGNAL(triggered()), this, SLOT(NewTab()));
    connect(actionQuit,     SIGNAL(triggered()), this, SLOT(QuitButton()));
    connect(actionAbout,    SIGNAL(triggered()), this, SLOT(About()));
    connect(actionCloseTab, SIGNAL(triggered()), this, SLOT(CloseTab()));

    menuFile->addAction(actionNewTab);
    menuFile->addAction(actionCloseTab);
    menuFile->addAction(actionQuit);
    menuHelp->addAction(actionAbout);
    menuHelp->addMenu(menuLang);
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);
    menuBar->addSeparator();

    setMenuBar(menuBar);

    //Tabs
    QTabWidget *tabWidget = sWorkTabMgr.CreateWidget(this);

    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", tabWidget);
    connect(newTabBtn, SIGNAL(clicked()), this, SLOT(NewTab()));

    tabWidget->setCornerWidget(newTabBtn);
    tabWidget->setMovable(true);
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)),    this, SLOT(tabChanged(int)));

    // Sort tab infos after they were added by static variables
    sWorkTabMgr.SortTabInfos();

    sWorkTabMgr.OpenHomeTab();
    setCentralWidget(tabWidget);
}

MainWindow::~MainWindow()
{

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

void MainWindow::tabChanged(int index)
{
    WorkTab *tab = sWorkTabMgr.getWorkTab(index);
    if(!tab)
        return;

    menuBar->clear();
    menuBar->addMenu(menuFile);
    menuBar->addMenu(menuHelp);

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
