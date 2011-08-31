#include <QDockWidget>
#include <QMdiArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QListView>
#include <QStringListModel>

#include "parser.h"
#include "lorrisanalyzer.h"
#include "newsourcedialog.h"
#include "datawidget.h"
#include "widgets/textwidget.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab()
{
    layout = new QVBoxLayout(this);

    QHBoxLayout *butt_1_layout = new QHBoxLayout();
    layout_area = new QHBoxLayout();

    QHBoxLayout *toolbox_layout = new QHBoxLayout();
    QHBoxLayout *header_ver_layout = new QHBoxLayout();

    QPushButton *connectButt = new QPushButton(tr("Disconnect"), this);
    connectButt->setObjectName("connectButton");
    connect(connectButt, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *newsource_button = new QPushButton(tr("New data source"), this);
    newsource_button->setObjectName("newSourceButton");
    connect(newsource_button, SIGNAL(clicked()), this, SLOT(newSourceButton()));
    QSpacerItem *spacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_area = new QMdiArea(this);

    QPushButton *tool1 = new QPushButton("TextView", this);
    connect(tool1, SIGNAL(clicked()), this, SLOT(textLabelButton()));
    toolbox_layout->addWidget(tool1, Qt::AlignTop);

    DataWidget *data = new DataWidget(this);

    butt_1_layout->addWidget(connectButt);
    butt_1_layout->addWidget(newsource_button);
    butt_1_layout->addSpacerItem(spacer);
    header_ver_layout->addLayout(butt_1_layout);
    header_ver_layout->addWidget(data, 1, Qt::AlignLeft);
    layout->addLayout(header_ver_layout);
    layout_area->addWidget(m_area, 1);
    layout_area->addLayout(toolbox_layout);
    layout->addLayout(layout_area);

    dialog = NULL;
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
    if(dialog)
        dialog->newData(data);
}

void LorrisAnalyzer::newSourceButton()
{
    dialog = new NewSourceDialog(this);
    connect(dialog, SIGNAL(structureData(analyzer_packet,QByteArray)), this, SLOT(dataStructure(analyzer_packet,QByteArray)));
    dialog->exec();
}

void LorrisAnalyzer::textLabelButton()
{
    TextWidget *wid = new TextWidget(NULL);

    m_area->addSubWindow(wid);
    wid->show();
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

