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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>

namespace Ui {
    class ScriptEditor;
}

class QAbstractButton;
class LineNumber;

class ScriptEditor : public QDialog
{
    Q_OBJECT

Q_SIGNALS:
    void applySource(bool close);
    
public:
    explicit ScriptEditor(const QString& source, const QString &widgetName = 0);
    ~ScriptEditor();

    QString getSource();
    
private slots:
    void buttonPressed(QAbstractButton *btn);
    void textChanged();
    void sliderMoved(int val = -1);
    void rangeChanged(int, int);

private:
    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
};

class LineNumber : public QWidget
{
public:
    LineNumber(QWidget *parent = 0);

    void setLineNum(int lineNum);
    void setScroll(int line);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_char_h;
    int m_line_num;
    int m_scroll;
    quint8 m_last_w;
};

#endif // SCRIPTEDITOR_H
