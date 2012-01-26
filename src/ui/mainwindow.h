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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <vector>
#include <QLocale>

extern QLocale::Language langs[];

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void NewTab();
    void CloseTab(int index = -1);
    void QuitButton();
    void About();
    void closeEvent(QCloseEvent *event);
    void tabChanged(int index);

    void langChanged(int idx);

private:
    bool Quit();
    QString getVersionString();

    std::vector<QMenu*> m_tab_menu;
    QMenuBar* menuBar;
    QMenu* menuFile;
    QMenu* menuHelp;

    std::vector<QAction*> m_lang_menu;
};

#endif // MAINWINDOW_H
