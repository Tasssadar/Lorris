/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QListWidgetItem>
#include <QFileDialog>
#include <QtConcurrentRun>

#include "graphexport.h"
#include "graphwidget.h"
#include "graphcurve.h"
#include "graphdata.h"

GraphExport::GraphExport(std::vector<GraphCurveInfo*> *curves, QWidget *parent) :
    QDialog(parent), ui(new Ui::GraphExport)
{
    ui->setupUi(this);
    ui->progress->hide();

    m_curves = curves;

    ui->colList->item(0)->setData(Qt::UserRole, -1);
    for(quint32 i = 0; i < curves->size(); ++i)
    {
        QString text = curves->at(i)->curve->title().text();
        ui->curveBox->addItem(text, QVariant(i));

        QListWidgetItem *item = new QListWidgetItem(ui->colList);
        item->setText(text);
        item->setData(Qt::UserRole, i);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(Qt::Checked);
    }

    connect(ui->colNamesBox, SIGNAL(clicked()),                     SLOT(updatePreview()));
    connect(ui->endBox,      SIGNAL(currentIndexChanged(int)),      SLOT(updatePreview()));
    connect(ui->sepEdit,     SIGNAL(textChanged(QString)),          SLOT(updatePreview()));
    connect(ui->colList,     SIGNAL(itemChanged(QListWidgetItem*)), SLOT(updatePreview()), Qt::QueuedConnection);

    updatePreview();
}

GraphExport::~GraphExport()
{
    delete ui;
}

void GraphExport::updatePreview()
{
    if(ui->binRadio->isChecked())
    {
        // TODO: how to implement?
        ui->prevText->setPlainText(tr("No preview available for binary format."));
    }
    else
    {
        ui->prevText->setPlainText(generateCSV(500));
    }
}

QString GraphExport::generateCSV(quint32 limit)
{
    quint32 maxSize = 0;
    for(quint32 i = 0; i < m_curves->size(); ++i)
        maxSize = std::max(maxSize, m_curves->at(i)->curve->getSize());
    maxSize = std::min(maxSize, limit);

    std::vector<GraphCurve*> order;
    for(int i = 0; i < ui->colList->count(); ++i)
    {
        QListWidgetItem *item = ui->colList->item(i);
        if(item->checkState() != Qt::Checked)
            continue;

        int idx = item->data(Qt::UserRole).toInt();
        if(idx == -1)
            order.push_back(NULL);
        else
            order.push_back(m_curves->at(idx)->curve);
    }

    QString csv;
    QString lineEnd;
    switch(ui->endBox->currentIndex())
    {
        case 0: lineEnd = "\r\n"; break;
        case 1: lineEnd = "\n"; break;
        case 2: lineEnd = "\n\r"; break;
        case 3: lineEnd = "\r"; break;
    }

    if(ui->colNamesBox->isChecked())
    {
        for(quint32 i = 0; i < order.size(); ++i)
        {
            if(!order[i])
                csv += "\"index\"";
            else
                csv += "\"" + order[i]->title().text() + "\"";
            csv += ui->sepEdit->text();
        }
        csv += lineEnd;
    }

    for(quint32 line = 0; line < maxSize; ++line)
    {
        emit updateProgress(line*100/maxSize);

        for(quint32 i = 0; i < order.size(); ++i)
        {
            if(!order[i])
                csv += QString::number(line);
            else
            {
                if(line >= order[i]->getSize())
                    csv += "-1";
                else
                    csv += QString::number(order[i]->sample(line).y());
            }
            csv += ui->sepEdit->text();
        }
        csv += lineEnd;
    }
    return csv;
}

QByteArray GraphExport::generateBin()
{
    GraphCurve *c = m_curves->at(ui->curveBox->itemData(ui->curveBox->currentIndex()).toInt())->curve;
    Q_ASSERT(c);

    QByteArray bin;
    QBuffer buff(&bin);
    buff.open(QIODevice::WriteOnly);

    bool big = !ui->endianBox->currentIndex();
    int idxW = (1 << ui->idxWidthBox->currentIndex());

    for(quint32 i = 0; i < c->getSize(); ++i)
    {
        emit updateProgress(i*100/c->getSize());

        if(ui->indexBox->isChecked())
        {
            quint64 idx = i;
            Utils::swapEndian<quint64>((char*)&idx);
            if(!big)
                Utils::swapEndian(((char*)&idx)+(sizeof(idx)-idxW), idxW);
            buff.write(((char*)&idx)+(sizeof(idx)-idxW), idxW);
        }

        qreal s = c->sample(i).y();
        switch(c->getDataType())
        {
            case NUM_FLOAT:
            {
                float f = s;
                buff.write((char*)&f, sizeof(f));
                break;
            }
            case NUM_DOUBLE:
                buff.write((char*)&s, sizeof(s));
                break;
            default:
            {
                QVariant var(s);
                switch(c->getDataType())
                {
                    case NUM_UINT8:
                    case NUM_INT8:
                    {
                        quint8 y = var.toInt();
                        if(big) Utils::swapEndian((char*)&y, sizeof(y));
                        buff.write((char*)&y, sizeof(y));
                        break;
                    }
                    case NUM_UINT16:
                    case NUM_INT16:
                    {
                        quint16 y = var.toInt();
                        if(big) Utils::swapEndian((char*)&y, sizeof(y));
                        buff.write((char*)&y, sizeof(y));
                        break;
                    }
                    case NUM_UINT32:
                    case NUM_INT32:
                    {
                        quint32 y = var.toInt();
                        if(big) Utils::swapEndian((char*)&y, sizeof(y));
                        buff.write((char*)&y, sizeof(y));
                        break;
                    }
                    case NUM_UINT64:
                    case NUM_INT64:
                    {
                        quint64 y = var.toLongLong();
                        if(big) Utils::swapEndian((char*)&y, sizeof(y));
                        buff.write((char*)&y, sizeof(y));
                        break;
                    }
                }
                break;
            }
        }
    }
    buff.close();
    return bin;
}

void GraphExport::accept()
{
    ui->buttonBox->setEnabled(false);

    if(exportData())
        QDialog::accept();
    else
        Utils::ThrowException(tr("Failed to open output file!"), this);

    ui->buttonBox->setEnabled(true);
}

bool GraphExport::exportData()
{
    QFile file(ui->fileLine->text());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    ui->progress->show();
    ui->progress->setValue(0);
    connect(this, SIGNAL(updateProgress(int)), ui->progress, SLOT(setValue(int)), Qt::QueuedConnection);

    if(ui->binRadio->isChecked())
        m_future = QtConcurrent::run(this, &GraphExport::generateBin);
    else
        m_future = QtConcurrent::run(this, &GraphExport::generateCSVByte);
    m_watcher.setFuture(m_future);

    QEventLoop ev(this);
    connect(&m_watcher, SIGNAL(finished()), &ev, SLOT(quit()));
    ev.exec();

    file.write(m_watcher.result());
    file.close();

    ui->progress->setValue(100);
    return true;
}

void GraphExport::on_binRadio_toggled(bool checked)
{
    ui->stack->setCurrentIndex(!checked);
    updatePreview();
}

void GraphExport::on_browseBtn_clicked()
{
    static const QString filter[] = { "Any file (*.*)", "CSV file (*.csv);;Any file (*.*)" };
    QString name = QFileDialog::getSaveFileName(this, tr("Graph data export"),
                                                sConfig.get(CFG_STRING_GRAPH_EXPORT), filter[ui->csvRadio->isChecked()]);

    if(name.isEmpty())
        return;

    ui->fileLine->setText(name);
    sConfig.set(CFG_STRING_GRAPH_EXPORT, name);
}

void GraphExport::on_curveBox_currentIndexChanged(int index)
{
    if(index >= (int)m_curves->size())
        return;

    GraphCurve *c = m_curves->at(index)->curve;
    switch(c->getDataType())
    {
        case NUM_FLOAT:
            ui->typeBox->setCurrentIndex(1);
            break;
        case NUM_DOUBLE:
            ui->typeBox->setCurrentIndex(2);
            break;
        default:
        {
            ui->typeBox->setCurrentIndex(0);
            switch(c->getDataType())
            {
                case NUM_UINT8:
                case NUM_INT8:
                    ui->widthBox->setCurrentIndex(0);
                    break;
                case NUM_UINT16:
                case NUM_INT16:
                    ui->widthBox->setCurrentIndex(1);
                    break;
                case NUM_UINT32:
                case NUM_INT32:
                    ui->widthBox->setCurrentIndex(2);
                    break;
                case NUM_UINT64:
                case NUM_INT64:
                    ui->widthBox->setCurrentIndex(3);
                    break;
            }
            break;
        }
    }
}

void GraphExport::on_typeBox_currentIndexChanged(int index)
{
    ui->widthBox->setEnabled(index == 0);
}
