/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPHEXPORT_H
#define GRAPHEXPORT_H

#include <QDialog>
#include <vector>
#include <QFuture>
#include <QFutureWatcher>

#include "ui_graphexport.h"

struct GraphCurveInfo;

class GraphExport : public QDialog, private Ui::GraphExport
{
    Q_OBJECT

Q_SIGNALS:
    void updateProgress(int val);
    
public:
    explicit GraphExport(std::vector<GraphCurveInfo*> *curves, QWidget *parent = 0);
    ~GraphExport();

public slots:
     void accept();
    
private slots:
    void on_binRadio_toggled(bool checked);
    void on_browseBtn_clicked();
    void on_curveBox_currentIndexChanged(int index);
    void on_typeBox_currentIndexChanged(int index);

    void updatePreview();

private:
    QString generateCSV(quint32 limit = UINT_MAX);
    QByteArray generateCSVByte()
    {
        return generateCSV().toUtf8();
    }

    QByteArray generateBin();
    bool exportData();

    Ui::GraphExport *ui;
    std::vector<GraphCurveInfo*> *m_curves;
    QFuture<QByteArray> m_future;
    QFutureWatcher<QByteArray> m_watcher;
};

#endif // GRAPHEXPORT_H
