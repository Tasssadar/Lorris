#ifndef SOURCEDIALOG_H
#define SOURCEDIALOG_H

#include <QDialog>

class QVBoxLayout;

namespace Ui {
  class SourceDialog;
}

class SourceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SourceDialog(QWidget *parent = 0);
    ~SourceDialog();

signals:

public slots:

private:
    QVBoxLayout *layout;

    Ui::SourceDialog *ui;

};

#endif // SOURCEDIALOG_H
