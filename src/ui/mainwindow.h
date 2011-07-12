#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>

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

private:
    bool Quit();
    QString getVersionString();
};

#endif // MAINWINDOW_H
