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

#include "mainwindow.h"
#include "HomeTab.h"
#include "WorkTab/WorkTab.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "tabdialog.h"
#include "revision.h"
#include "config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(getVersionString());
    setMinimumSize(600, 500);
    setWindowState(Qt::WindowMaximized);

    // init config now!
    Config::GetSingleton();

    // menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* menuFile = new QMenu(tr("&File"), this);
    QMenu* menuHelp = new QMenu(tr("&Help"), this);

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
    Config::Destroy();
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
    delete box;
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
    box->setWindowTitle(tr("About Lorris"));
    box->setText(tr("Lorris revision ") + QString::number(REVISION));
    box->setIcon(QMessageBox::Information);
    box->exec();
    delete box;
}
