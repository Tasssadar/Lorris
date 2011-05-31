#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class MainWindow;

class HomeTab : public QWidget {
    Q_OBJECT
public:
    HomeTab(MainWindow *parent);
    ~HomeTab();

private slots:
    void NewTab();

private:
    MainWindow *m_mainWindow;
};

#endif // HOMETAB_H
