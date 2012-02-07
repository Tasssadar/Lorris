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

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>

class WorkTab;
class Connection;
class WorkTabInfo;
class ConnectionBuilder;

namespace Ui {
    class TabDialog;
}

class TabDialog : public QDialog
{
    Q_OBJECT
public:
    TabDialog(QWidget *parent = 0);
    ~TabDialog();

private slots:
    void PluginSelected(int index);
    void CreateTab();
    void FillConOptions(int index);
    void setCreateButtonState(bool connecting);
    void connectFailed(const QString& text);
    void connectionSucces(Connection *con, const QString &tabName, WorkTabInfo *info, qint16 conType);

private:
    ConnectionBuilder *m_con_builder;
    Ui::TabDialog *ui;
    int m_cur_con;
};

#endif // TABDIALOG_H
