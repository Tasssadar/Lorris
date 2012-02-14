/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QPushButton>
#include <QStyle>

#include "progressdialog.h"

ProgressDialog::ProgressDialog(const QString &text, QWidget *parent) :
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
}

void ProgressDialog::cancel()
{
    m_cancel_btn->setEnabled(false);
    setLabelText(tr("Waiting for pending operation to finish..."));
}
