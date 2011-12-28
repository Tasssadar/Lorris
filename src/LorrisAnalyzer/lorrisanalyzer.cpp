#include <QDockWidget>
#include <QMdiArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QListView>
#include <QStringListModel>
#include <QSpinBox>

#include "parser.h"
#include "lorrisanalyzer.h"
#include "datawidget.h"
#include "widgets/textwidget.h"
#include "sourcedialog.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab()
{
    layout = new QVBoxLayout(this);

    QHBoxLayout *butt_1_layout = new QHBoxLayout();
    QHBoxLayout *butt_2_layout = new QHBoxLayout();
    layout_area = new QHBoxLayout();

    QHBoxLayout *toolbox_layout = new QHBoxLayout();
    QVBoxLayout *header_ver_layout = new QVBoxLayout();
    QHBoxLayout *header_hor_layout = new QHBoxLayout();

    QPushButton *connectButt = new QPushButton(tr("Disconnect"), this);
    connectButt->setObjectName("connectButton");
    connect(connectButt, SIGNAL(clicked()), this, SLOT(connectButton()));

    QSpacerItem *spacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel *label_len = new QLabel(tr("Packet lenght: "), this);
    QSpinBox *packet_len = new QSpinBox(this);
    packet_len->setObjectName("packetLen");
    packet_len->setMinimum(0);
    connect(packet_len, SIGNAL(valueChanged(int)), this, SLOT(packetLenChanged(int)));

    m_area = new QMdiArea(this);

    QPushButton *tool1 = new QPushButton("TextView", this);
    connect(tool1, SIGNAL(clicked()), this, SLOT(textLabelButton()));
    toolbox_layout->addWidget(tool1, Qt::AlignTop);

    DataWidget *data = new DataWidget(this);

    butt_1_layout->addWidget(connectButt);
    butt_1_layout->addSpacerItem(spacer);
    butt_2_layout->addWidget(label_len);
    butt_2_layout->addWidget(packet_len);
    header_ver_layout->addLayout(butt_1_layout);
    header_ver_layout->addLayout(butt_2_layout);
    layout->addLayout(header_hor_layout);
    header_hor_layout->addLayout(header_ver_layout);
    header_hor_layout->addWidget(data, 1, Qt::AlignLeft);
    layout_area->addWidget(m_area, 1);
    layout_area->addLayout(toolbox_layout);
    layout->addLayout(layout_area);

    m_parser = new Parser();
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
    delete m_parser;
}

void LorrisAnalyzer::connectButton()
{
    QPushButton *connectButt = this->findChild<QPushButton *>("connectButton");
    if(m_state & STATE_DISCONNECTED)
    {
        connectButt->setText(tr("Connecting..."));
        connectButt->setEnabled(false);
        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
    else
    {
        m_con->Close();
        m_state |= STATE_DISCONNECTED;

        connectButt->setText("Connect");
    }
}

void LorrisAnalyzer::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    QPushButton *button = findChild<QPushButton *>("connectButton");
    button->setEnabled(true);
    if(!result)
    {
        button->setText(tr("Connect"));

        QMessageBox *box = new QMessageBox(this);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Can't open connection!"));
        box->exec();
        delete box;
    }
}

void LorrisAnalyzer::connectedStatus(bool connected)
{
    QPushButton *button = findChild<QPushButton *>("connectButton");
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        button->setText(tr("Disconnect"));
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        button->setText(tr("Connect"));
    }
}


void LorrisAnalyzer::readData(QByteArray data)
{
    DataWidget *dataW = findChild<DataWidget*>("DataWidget");
    dataW->newData(data);
}

void LorrisAnalyzer::textLabelButton()
{
    TextWidget *wid = new TextWidget(this);

    m_area->addSubWindow(wid);
    wid->show();

    DataWidget *dataW = findChild<DataWidget*>("DataWidget");
    connect(wid, SIGNAL(connectLabel(AnalyzerWidget*,int)), dataW, SLOT(connectLabel(AnalyzerWidget*,int)));
}

void LorrisAnalyzer::dataStructure(analyzer_packet pkt, QByteArray curData)
{
    QListView *list = new QListView(this);
    layout_area->insertWidget(0, list);
    m_parser->setStructure(pkt);


    QStringListModel *model = new QStringListModel();
    QStringList stringlist;
    for(quint8 i = 0; i < curData.size(); ++i)
        stringlist << (QString::number(curData[i]));
    model->setStringList(stringlist);
    list->setModel(model);
}

void LorrisAnalyzer::packetLenChanged(int val)
{
    DataWidget *data = this->findChild<DataWidget*>("DataWidget");
    data->setSize(val);
}

void LorrisAnalyzer::onTabShow()
{
    SourceDialog *d = new SourceDialog(this);
    d->exec();
    delete d;
}


