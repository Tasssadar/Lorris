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

#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include "../datawidget.h"

class QLabel;
class ScriptEditor;
class ScriptEnv;
class Terminal;

class ScriptWidget : public DataWidget
{
    Q_OBJECT
public:
    ScriptWidget(QWidget *parent = 0);
    ~ScriptWidget();

    void setUp(AnalyzerDataStorage *);
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

protected:
     void newData(analyzer_data *data, quint32 index);

private slots:
     void setSourceTriggered();
     void sourceSet(bool close);

private:
     ScriptEditor *m_editor;
     ScriptEnv *m_env;
     Terminal *m_terminal;
};

class ScriptWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ScriptWidgetAddBtn(QWidget *parent = 0);

};
#endif // SCRIPTWIDGET_H
