/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "ScriptWidget/scriptwidget.h"

class TerminalWidget : public ScriptWidget
{
    Q_OBJECT
public:
    TerminalWidget(QWidget *parent = 0);
    ~TerminalWidget();

    void setUp(Storage *storage);
protected slots:
    void newData(analyzer_data *data, quint32 index);
};

#endif // TERMINALWIDGET_H
