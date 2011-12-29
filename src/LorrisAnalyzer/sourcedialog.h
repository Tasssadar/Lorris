#ifndef SOURCEDIALOG_H
#define SOURCEDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <vector>

namespace Ui {
  class SourceDialog;
}

class ScrollDataLayout;
class QSpacerItem;
class QLabel;

enum DataFormat
{
    FORMAT_HEX    = 0,
    FORMAT_BYTE   = 1,
    FORMAT_STRING = 2
};

enum DataType
{
    DATA_BODY      = 0x01,
    DATA_HEADER    = 0x02,
    DATA_DEVICE_ID = 0x04,
    DATA_OPCODE    = 0x08,
    DATA_LEN       = 0x10
};

class SourceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SourceDialog(QWidget *parent = 0);
    ~SourceDialog();

signals:

public slots:
    void readData(QByteArray data);
    void headerLenToggled(bool checked);
    void headerLenChanged(int values);

private:
    ScrollDataLayout *scroll_layout;
    Ui::SourceDialog *ui;

};

class ScrollDataLayout : public QHBoxLayout
{
    Q_OBJECT
public:
    explicit ScrollDataLayout(QWidget *parent = 0);
    ~ScrollDataLayout();

    void ClearLabels();
    void AddLabel(QString value, qint8 type);
    void RemoveLabel();

    void SetData(QByteArray data);
    void SetLabelType(QLabel *label, quint8 type);
    quint8 GetTypeForPos(quint32 pos);

public slots:
    void lenChanged(int len);
    void fmtChanged(int len);

private:
    std::vector<QLabel*> m_labels;
    QSpacerItem *m_spacer;
    quint8 m_format;
};

#endif // SOURCEDIALOG_H
