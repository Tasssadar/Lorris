#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class WorkTabMgr;
class WorkTab;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void ExtNewTab() { NewTab(); }
    void AddTab(WorkTab *tab, QString label);

private slots:
    void NewTab();
    void CloseTab(int index = -1);
    void QuitButton();
    void About();
    void closeEvent(QCloseEvent *event);

private:
    bool Quit();
    void OpenHomeTab();
    QString getVersionString();

    QTabWidget* tabs;
    int windowCount;
};

#endif // MAINWINDOW_H
