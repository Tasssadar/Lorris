#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class MainWindow;

class HomeTab : public QWidget {
    Q_OBJECT
public:
    HomeTab(QWidget *parent);
    ~HomeTab();

private slots:
    void NewTab();

};

#endif // HOMETAB_H
