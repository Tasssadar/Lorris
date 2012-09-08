/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPushButton>
#include <QStyle>

#include "progressdialog.h"
#include "../../misc/utils.h"

ProgressDialog::ProgressDialog(WId id, const QString &text, QWidget *parent) :
    QProgressDialog(parent, Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("Progress"));
    setLabelText(text);
    setMaximum(100);
    setFixedSize(500, height());

    m_cancel_btn = new QPushButton(tr("Cancel"));
    m_cancel_btn->setIcon(m_cancel_btn->style()->standardIcon(QStyle::SP_DialogCancelButton));
    setCancelButton(m_cancel_btn);

    disconnect(this, 0, 0, 0);
    connect(m_cancel_btn, SIGNAL(clicked()), SLOT(cancel()));
    connect(m_cancel_btn, SIGNAL(clicked()), SIGNAL(canceled()));

    m_win7.init(id);
}

void ProgressDialog::cancel()
{
    m_win7.setProgressState(EcWin7::NoProgress);

    m_cancel_btn->setEnabled(false);
    setLabelText(tr("Waiting for pending operation to finish..."));
}

void ProgressDialog::hideEvent(QHideEvent *event)
{
    QProgressDialog::hideEvent(event);
    m_win7.setProgressState(EcWin7::NoProgress);
}

void ProgressDialog::setValue(int progress)
{
    QProgressDialog::setValue(progress);
    m_win7.setProgressState(EcWin7::Normal);
    m_win7.setProgressValue(progress, 100);
}
