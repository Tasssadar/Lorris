/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FLOATINGINPUTDIALOG_H
#define FLOATINGINPUTDIALOG_H

#include <QLineEdit>
#include <QPointer>

#include "floatingwidget.h"

class QValidator;

class FloatingInputDialog : public FloatingWidget
{
    Q_OBJECT
public:
    static QString getText(const QString& title, const QString & text = QString(),
                           bool * ok = 0, QLineEdit::EchoMode mode = QLineEdit::Normal);
    static QByteArray getBytes(const QString& title, const QString & text = QString(),
                           bool * ok = 0, QLineEdit::EchoMode mode = QLineEdit::Normal);
    static int getInt(const QString& title, int value = 0, int min = -2147483647, int max = 2147483647,
                      int step = 1, bool *ok = 0);
    static double getDouble(const QString& title, double value = 0, double min = -2147483647,
                            double max = 2147483647, int decimals = 1, bool *ok = 0);
    static bool getIntRange(const QString& title, int& minVal, int& maxVal,
                            int min = -2147483647, int max = 2147483647);
    static bool getDoubleRange(const QString& title, double& minVal, double& maxVal,
                              int decimals = 1, double min = -2147483647, double max = 2147483647);

protected:
    explicit FloatingInputDialog(const QString& title, const QString &icon, QWidget *parent = 0);

    void initRangeLayout(QPointer<QLineEdit> &minEdit, QPointer<QLineEdit> &maxEdit);
};

#endif // FLOATINGINPUTDIALOG_H
