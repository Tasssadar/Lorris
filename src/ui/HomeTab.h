/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>
#include <QHash>

class WorkTabInfo;

namespace Ui {
    class HomeTab;
}

class HomeTab : public QWidget {
    Q_OBJECT

Q_SIGNALS:
    void tabOpened();

public:
    HomeTab(QWidget *parent);
    ~HomeTab();

private slots:
    void buttonClicked();

private:
    Ui::HomeTab *ui;

    QHash<QObject *, WorkTabInfo *> m_buttonInfoMap;
};

#endif // HOMETAB_H
