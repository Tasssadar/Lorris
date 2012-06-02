/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SOURCESELECTDIALOG_H
#define SOURCESELECTDIALOG_H

#include <QDialog>

namespace Ui {
class SourceSelectDialog;
}

class SourceSelectDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SourceSelectDialog(QWidget *parent = 0);
    ~SourceSelectDialog();

    void DisableNew();

    qint8 get();
    quint8 getDataMask();
    QString getFileName();

private slots:
    void contButton();
    void browse();
    void loadRadioToggled(bool toggle);
    
private:
    Ui::SourceSelectDialog *ui;
};

#endif // SOURCESELECTDIALOG_H
