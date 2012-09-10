/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "../datawidget.h"

class RenderWidget;

class GLWidget : public DataWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    
private:
    RenderWidget *m_widget;
};

class GLWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    GLWidgetAddBtn(QWidget *parent = 0);
};

#endif // GLWIDGET_H
