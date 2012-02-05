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
    void serialConResult(Connection *con, bool result);
    void tcpConResult(Connection *con, bool result);

private:
    WorkTab *ConnectSP(WorkTabInfo *info);
    WorkTab *ConnectShupito(WorkTabInfo *info);
    WorkTab *ConnectTcp(WorkTabInfo *info);

    QVBoxLayout *layout;
    QHBoxLayout *columns;
    QVBoxLayout *secondCol;
    QHBoxLayout *conOptions;
    QListWidget *pluginsBox;
    QComboBox *conBox;
    WorkTabInfo *tmpTabInfo;
};

#endif // TABDIALOG_H
