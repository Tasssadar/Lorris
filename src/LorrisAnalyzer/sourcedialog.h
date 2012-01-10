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
