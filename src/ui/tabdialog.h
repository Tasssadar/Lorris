#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>

class TabDialog : public QDialog
{
    Q_OBJECT
public:
    TabDialog(QWidget *parent = 0);
    ~TabDialog();

private slots:
    void PluginSelected(int index);
    void ConnectionSelected(int index);
    void CreateTab();

private:

    QHBoxLayout *layout;
    QComboBox *pluginsBox;
    QComboBox *conBox;
    QLineEdit *conLine;
};

#endif // TABDIALOG_H
