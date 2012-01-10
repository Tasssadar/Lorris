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

#ifndef SOURCEDIALOG_H
#define SOURCEDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <vector>
#include "packet.h"

namespace Ui {
  class SourceDialog;
}

class ScrollDataLayout;
class LabelLayout;
class QSpacerItem;
class QLabel;
class QAbstractButton;

class SourceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SourceDialog(QWidget *parent = 0);
    ~SourceDialog();

    analyzer_packet *getStructure();

public slots:
    void readData(const QByteArray& data);
    void headerLenToggled(bool checked);
    void headerLenChanged(int values);
    void staticLenChanged(int values);
    void staticCheckToggled(bool checked);
    void cmdCheckToggled(bool checked);
    void idCheckToggled(bool checked);
    void lenFmtChanged(int index);
    void butonnBoxClicked(QAbstractButton *b);

private:
    void AddOrRmHeaderType(bool add, quint8 type);


    ScrollDataLayout *scroll_layout;
    LabelLayout *scroll_header;
    Ui::SourceDialog *ui;
    analyzer_header m_header;
    bool setted;
    bool setFirst;
};


#endif // SOURCEDIALOG_H
