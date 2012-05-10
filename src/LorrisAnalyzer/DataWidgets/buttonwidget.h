/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include "datawidget.h"

class ButtonWidget : public DataWidget
{
    Q_OBJECT
public:
    ButtonWidget(QWidget *parent);
    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setButtonName(const QString& name);
    void setButtonName();

private slots:
    void buttonClicked();

private:
    QPushButton *m_button;
};

class ButtonWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ButtonWidgetAddBtn(QWidget *parent = 0);

};
#endif // TERMINALWIDGET_H
