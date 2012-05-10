/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QWebView>
#include <QPushButton>
#include <QSpacerItem>
#include <QComboBox>
#include <QCheckBox>

#include "newsourcedialog.h"
#include "WorkTab/WorkTab.h"

NewSourceDialog::NewSourceDialog(QWidget *parent) : QDialog(parent)
{
    setFixedSize(420, 450);
    layout = new QVBoxLayout(this);
    QHBoxLayout *first_line = new QHBoxLayout;
    QHBoxLayout *second_line = new QHBoxLayout;
    QHBoxLayout *third_line = new QHBoxLayout;

    QLabel *description = new QLabel(tr("Set packet lenght and start transmiting packets."), this);

    QLabel *countBoxLabel = new QLabel(tr("Packet lenght"), this);
    QSpinBox *countBox = new QSpinBox(this);
    countBox->setMinimum(0);
    countBox->setObjectName("countBox");
    connect(countBox, SIGNAL(valueChanged(int)), this, SLOT(countBoxChanged(int)));

    QLabel *startBoxLabel = new QLabel(tr("Start bytes: "), this);
    QSpinBox *startBox = new QSpinBox(this);
    startBox->setMinimum(0);
    startBox->setObjectName("startBox");

    QLabel *stopBoxLabel = new QLabel(tr("Stop bytes: "), this);
    QSpinBox *stopBox = new QSpinBox(this);
    stopBox->setMinimum(0);
    stopBox->setObjectName("stopBox");

    table = new QTableWidget(8, 8, this);
    for(quint8 i = 0; i < 8; ++i)
    {
        table->setColumnWidth(i, 45);
        table->setRowHeight(i, 35);
    }
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(table, SIGNAL(itemSelectionChanged()), this, SLOT(tableClicked()));

    QPushButton *freeze = new QPushButton(tr("Pause"), this);
    freeze->setObjectName("pauseButton");
    connect(freeze, SIGNAL(clicked()), this, SLOT(pauseButton()));

    QComboBox *parseBox = new QComboBox(this);
    parseBox->addItem(tr("Hex"));
    //parseBox->addItem("Byte");
    parseBox->addItem(tr("Text"));
    parseBox->setObjectName("parseBox");

    QSpacerItem *spacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QPushButton *next = new QPushButton(tr("Next"), this);
    next->setObjectName("nextButton");
    next->setEnabled(false);
    connect(next, SIGNAL(clicked()), this, SLOT(nextButton()));


    layout->addWidget(description);
    first_line->addWidget(countBoxLabel);
    first_line->addWidget(countBox);
    second_line->addWidget(startBoxLabel);
    second_line->addWidget(startBox);
    second_line->addWidget(stopBoxLabel);
    second_line->addWidget(stopBox);
    third_line->addWidget(freeze);
    third_line->addWidget(parseBox);
    third_line->addItem(spacer);
    third_line->addWidget(next);
    layout->addLayout(first_line);
    layout->addLayout(second_line);
    layout->addWidget(table);
    layout->addLayout(third_line);

    paused = false;
}

NewSourceDialog::~NewSourceDialog()
{

}

void NewSourceDialog::countBoxChanged(int i)
{
    QPushButton *butt = findChild<QPushButton *>("nextButton");
    if(i != 0 && !table->selectedItems().empty())
        butt->setEnabled(true);
    else
        butt->setEnabled(false);

}

void NewSourceDialog::newData(QByteArray data)
{
    tableData.append(data);
    if(tableData.size() > 8*8)
        tableData = tableData.right(8*8);

    UpdateTable();
}

void NewSourceDialog::UpdateTable()
{
    if(paused)
        return;

    QTableWidgetItem *newItem = NULL;
    quint8 col = 0;
    quint8 row = 0;
    table->setUpdatesEnabled(false);
    table->clear();

    int parse = findChild<QComboBox *>("parseBox")->currentIndex();

    QString result = "";
    for(qint8 i = 0; i < tableData.size(); ++i)
    {
        switch(parse)
        {
            case 0:
                result = QString(tableData.mid(i, 1).toHex()).toUpper();
                if(result.count() == 1)
                    result.push_front('0');
                result.push_front("0x");
                break;
            case 1:
                result = QString(tableData.mid(i, 1));
                break;
        }

        newItem = new QTableWidgetItem(result);
        table->setItem(row, col++, newItem);
        if(col == 8)
        {
            col = 0;
            ++row;
        }
    }
    table->setUpdatesEnabled(true);
}

void NewSourceDialog::tableClicked()
{
    QSpinBox *box = findChild<QSpinBox *>("countBox");
    QPushButton *butt = findChild<QPushButton *>("nextButton");
    if(box->value() != 0 && !table->selectedItems().empty())
        butt->setEnabled(true);
    else
        butt->setEnabled(false);
}

void NewSourceDialog::pauseButton()
{
    paused = !paused;
    QPushButton *button = findChild<QPushButton *>("pauseButton");
    if(paused)
        button->setText(tr("Unpause"));
    else
        button->setText(tr("Pause"));
    UpdateTable();
}

void NewSourceDialog::nextButton()
{
    QSpinBox *box = findChild<QSpinBox *>("countBox");

    analyzer_packet pkt;
    pkt.lenght = box->value();

    box = findChild<QSpinBox *>("startBox");
    quint8 startLen = box->value();
    box = findChild<QSpinBox *>("stopBox");
    quint8 stopLen = box->value();

    quint16 index = table->selectedItems()[0]->row()*8 + table->selectedItems()[0]->column();

    pkt.start.insert(0, tableData.mid(index, startLen));
    pkt.stop.insert(0, tableData.mid((index+pkt.lenght)-stopLen, stopLen));
    emit structureData(pkt, tableData);
    this->close();
}
