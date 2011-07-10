#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>

class SerialDeviceEnumerator;
class WorkTab;
class Connection;
class WorkTabInfo;

class TabDialog : public QDialog
{
    Q_OBJECT
public:
    TabDialog(QWidget *parent = 0);
    ~TabDialog();

private slots:
    void PluginSelected(int index);
    void CreateTab();
    void FillConOptions(int index);
    void serialConResult(Connection *con, bool result);

private:
    WorkTab *ConnectSP(WorkTabInfo *info);

    QVBoxLayout *layout;
    QHBoxLayout *firstLine;
    QHBoxLayout *secondLine;
    QComboBox *pluginsBox;
    QComboBox *conBox;
    SerialDeviceEnumerator *m_sde;
    WorkTabInfo *tmpTabInfo;
};

#endif // TABDIALOG_H
