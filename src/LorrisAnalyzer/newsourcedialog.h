#ifndef NEWSOURCEDIALOG_H
#define NEWSOURCEDIALOG_H

#include <QDialog>
#include <QHBoxLayout>

#include "lorrisanalyzer.h"

class QVBoxLayout;
class QTableWidget;

class NewSourceDialog : public QDialog
{
    Q_OBJECT

Q_SIGNALS:
    void structureData(analyzer_packet pkt, QByteArray curData);

public:
    NewSourceDialog(QWidget *parent);
    ~NewSourceDialog();

    void newData(QByteArray data);

private slots:
    void countBoxChanged(int i);
    void pauseButton();
    void nextButton();
    void tableClicked();

private:
    void UpdateTable();

    QVBoxLayout *layout;

    QByteArray tableData;
    QTableWidget *table;
    bool paused;

};

#endif // NEWSOURCEDIALOG_H
