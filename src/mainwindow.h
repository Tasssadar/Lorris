#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void NewTab();
    void CloseTab(int index);
    bool Quit();
    void About();
    void closeEvent(QCloseEvent *event);

private:
    QString getVersionString();

    QTabWidget* tabs;
    int windowCount;
};

#endif // MAINWINDOW_H
