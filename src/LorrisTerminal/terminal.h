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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QPlainTextEdit>

class QByteArray;

class Terminal : public QPlainTextEdit
{
    Q_OBJECT

Q_SIGNALS:
    void keyPressedASCII(QByteArray key);

public:
    Terminal(QWidget *parent);
    ~Terminal();

    void appendText(QString text, bool toEdit = true);
    void setTextTerm(QString text, bool toEdit = true);
    QString getText() { return content; }
    void updateEditText();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QString content;
    QScrollBar *sb;
};

#endif // TERMINAL_H
