/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include <QLineEdit>
#include <QHash>

#include "datawidget.h"

class QUiLoader;

class InputWidget : public DataWidget
{
    Q_OBJECT
public:
    typedef QWidget *(*QWidgetFc)(QWidget*);

    InputWidget(QWidget *parent = 0);
    ~InputWidget();

public slots:
    QWidget *newWidget(const QString& name, int stretch = 0);
    void setHorizontal(bool horizontal);
    void clear();
    void removeWidget(QWidget *widget);

private:
    QBoxLayout *m_layout;
};

class InputWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    InputWidgetAddBtn(QWidget *parent = 0);

};

#endif // INPUTWIDGET_H
