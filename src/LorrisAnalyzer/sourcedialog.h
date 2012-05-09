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
class PacketParser;
class QListWidgetItem;

class SourceDialog : public QDialog
{
    Q_OBJECT

Q_SIGNALS:
    void readData(const QByteArray& data);

public:
    explicit SourceDialog(analyzer_packet *pkt, QWidget *parent = 0);
    ~SourceDialog();

    analyzer_packet *getStructure();

public slots:
    void headerLenToggled(bool checked);
    void headerLenChanged(int values);
    void staticLenChanged(int values);
    void staticCheckToggled(bool checked);
    void cmdCheckToggled(bool checked);
    void idCheckToggled(bool checked);
    void lenFmtChanged(int index);
    void butonnBoxClicked(QAbstractButton *b);
    void offsetChanged(int val);
    void staticDataChanged(QListWidgetItem*);
    void endianChanged(int idx);
    void packetLenChanged(int val);
    void packetReceived(analyzer_data *data, quint32);

private:
    void AddOrRmHeaderType(bool add, quint8 type);
    void updateHeaderLabels();

    ScrollDataLayout *scroll_layout;
    LabelLayout *scroll_header;
    Ui::SourceDialog *ui;
    analyzer_packet m_packet;
    PacketParser *m_parser;
    bool setted;
};


#endif // SOURCEDIALOG_H
