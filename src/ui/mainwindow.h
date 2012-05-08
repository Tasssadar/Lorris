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
#include <QHash>
#include <QPointer>

extern QLocale::Language langs[];

class WorkTabInfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow *getInstance()
    {
        return m_instance.data();
    }

public slots:
    void show(const QStringList &openFiles);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void NewSpecificTab();
    void newTab();
    void About();
    void OpenConnectionManager();

    void langChanged(int idx);

private:
    QString getVersionString();

    std::vector<QMenu*> m_tab_menu;
    QMenuBar* menuBar;
    QMenu* menuFile;
    QMenu* menuHelp;

    std::vector<QAction*> m_lang_menu;
    QHash<QObject *, WorkTabInfo *> m_actionTabInfoMap;

    static QPointer<MainWindow> m_instance;
};

#endif // MAINWINDOW_H
