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

#include "../WorkTab/tab.h"

class WorkTabInfo;

namespace Ui {
    class HomeTab;
}

class HomeTab : public Tab {
    Q_OBJECT

Q_SIGNALS:
    void tabOpened();

public:
    HomeTab(QWidget *parent);
    ~HomeTab();

private slots:
    void buttonClicked();

    void on_openConnManagerLink_linkActivated(const QString &link);

private:
    Ui::HomeTab *ui;

    QHash<QObject *, WorkTabInfo *> m_buttonInfoMap;
};

#endif // HOMETAB_H
