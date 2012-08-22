/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVBoxLayout>
#include "homedialog.h"
#include "HomeTab.h"

HomeDialog::HomeDialog(quint32 windowId, QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);

    HomeTab *widget = new HomeTab(this);
    widget->setWindowId(windowId);
    l->addWidget(widget);

    connect(widget, SIGNAL(tabOpened()), SLOT(accept()));

    resize(800, 450);
}
