#ifndef RELOADDIALOG_H
#define RELOADDIALOG_H

#include <QDialog>

namespace Ui {
class ReloadDialog;
}

class ReloadDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ReloadDialog(const QString& filename, QWidget *parent = 0);
    ~ReloadDialog();
    
    quint8 getReloadState();

private:
    Ui::ReloadDialog *ui;
};

#endif // RELOADDIALOG_H
