/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SOURCESELECTDIALOG_H
#define SOURCESELECTDIALOG_H

#include <QDialog>

#include "ui_sourceselectdialog.h"

class SourceSelectDialog : public QDialog, private Ui::SourceSelectDialog
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
    void on_newRadio_toggled(bool toggle);
    void on_loadRadio_toggled(bool toggle);
    void on_binRadio_toggled(bool toggle);

    void on_contButton_clicked();
    void on_loadBrowse_clicked();
    void on_importBrowse_clicked();
    
private:
    Ui::SourceSelectDialog *ui;
};

#endif // SOURCESELECTDIALOG_H
