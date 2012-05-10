#ifndef OVERVCCDIALOG_H
#define OVERVCCDIALOG_H

#include <QDialog>

namespace Ui {
class OverVccDialog;
}

class OverVccDialog : public QDialog
{
    Q_OBJECT
    
public:
    OverVccDialog(bool autoclose, QWidget *parent = 0);
    ~OverVccDialog();

    bool autoclose() const { return m_autoclose; }

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    
private:
    Ui::OverVccDialog *ui;
    bool m_autoclose;
};

#endif // OVERVCCDIALOG_H
