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

#include "../dep/ecwin7/ecwin7.h"

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
    bool winEvent(MSG *message, long *result);

private:
    void saveWindowParams();
    void loadWindowParams();
    QString getVersionString();

    EcWin7 m_win7;
};

#endif // MAINWINDOW_H
