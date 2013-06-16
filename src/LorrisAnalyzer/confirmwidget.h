/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CONFIRMWIDGET_H
#define CONFIRMWIDGET_H

#include "../ui/floatingwidget.h"

class QListWidgetItem;
class QListWidget;
class ConfirmWidget : public FloatingWidget
{
    Q_OBJECT
public:
    explicit ConfirmWidget(QListWidgetItem *it);
    ~ConfirmWidget();

protected:
    void keyPressEvent(QKeyEvent *ev);

private slots:
    void itemActivated(QListWidgetItem *it);

private:
    QListWidget *m_list;
};

#endif // CONFIRMWIDGET_H
