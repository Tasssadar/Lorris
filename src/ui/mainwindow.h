/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <vector>
#include <QLocale>
#include <QHash>

extern QLocale::Language langs[];

class WorkTabInfo;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

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
};

#endif // MAINWINDOW_H
