/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class WorkTabInfo;
class TabView;
class HomeTab;
class DataFileParser;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(quint32 id, QWidget *parent = 0);
    ~MainWindow();

    quint32 getId() const { return m_id; }
    TabView *getTabView() const { return m_tabView; }

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

public slots:
    void show(const QStringList &openFiles);
    void closeHomeTab();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void openHomeTab();
    void changeWindowTitle(const QString &title);

private:
    void saveWindowParams();
    void loadWindowParams();

    quint32 m_id;
    TabView *m_tabView;
    HomeTab *m_hometab;
};

#endif // MAINWINDOW_H
