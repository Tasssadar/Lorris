/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>

#include "../dep/ecwin7/ecwin7.h"

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QString& text, QWidget *parent = 0);

public slots:
    void cancel();
    void setValue(int progress);

protected:
    void hideEvent(QHideEvent *event);
    bool winEvent(MSG *message, long *result);

private:
    QPushButton *m_cancel_btn;
    EcWin7 m_win7;
};

#endif // PROGRESSDIALOG_H
