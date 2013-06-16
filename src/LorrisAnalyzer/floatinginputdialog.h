/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FLOATINGINPUTDIALOG_H
#define FLOATINGINPUTDIALOG_H

#include <QLineEdit>
#include "floatingwidget.h"

class FloatingInputDialog : public FloatingWidget
{
    Q_OBJECT
public:
    
    static QString getText(QWidget *parent, const QString& title, const QString& label, QLineEdit::EchoMode mode = QLineEdit::Normal, const QString& text = QString(), bool *ok = 0, Qt::WindowFlags flags = 0);

protected:
    explicit FloatingInputDialog(QWidget *parent = 0);
};

#endif // FLOATINGINPUTDIALOG_H
