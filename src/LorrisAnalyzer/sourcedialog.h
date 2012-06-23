/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
class PortConnection;

class SourceDialog : public QDialog
{
    Q_OBJECT

public:
    SourceDialog(analyzer_packet *pkt, PortConnection *con, const QString &importFile = QString());
    ~SourceDialog();

    static analyzer_packet *getStructure(analyzer_packet *pkt, PortConnection *con, const QString &importFile = QString());

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
