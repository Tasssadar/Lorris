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

class SourceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SourceDialog(QWidget *parent = 0);
    ~SourceDialog();

public slots:
    void readData(QByteArray data);
    void headerLenToggled(bool checked);
    void headerLenChanged(int values);
    void staticLenChanged(int values);
    void staticCheckToggled(bool checked);
    void cmdCheckToggled(bool checked);
    void idCheckToggled(bool checked);

private:
    void AddOrRmHeaderType(bool add, quint8 type);


    ScrollDataLayout *scroll_layout;
    LabelLayout *scroll_header;
    Ui::SourceDialog *ui;
    analyzer_header m_header;

};


#endif // SOURCEDIALOG_H
