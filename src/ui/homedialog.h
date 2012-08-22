/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef HOMEDIALOG_H
#define HOMEDIALOG_H

#include <QDialog>

class HomeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HomeDialog(quint32 windowId, QWidget *parent = 0);
};

#endif // HOMEDIALOG_H
