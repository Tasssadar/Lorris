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
    QWidget *newWidget(const QString& name, const QString& idString, int stretch = 0);
    void setHorizontal(bool horizontal);
    void clear();
    void removeWidget(QWidget *widget);
    void removeWidget(const QString& idString);
    QWidget *get(const QString& idString);

private:
    QBoxLayout *m_layout;
    QHash<QString, QWidget*> m_namedWidgets;
};

#endif // INPUTWIDGET_H
