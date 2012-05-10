/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QString& text, QWidget *parent = 0);

public slots:
    void cancel();

private:
    QPushButton *m_cancel_btn;
};

#endif // PROGRESSDIALOG_H
