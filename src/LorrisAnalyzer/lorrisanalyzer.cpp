#include <QDockWidget>
#include <QMdiArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QMessageBox>

#include "lorrisanalyzer.h"
#include "newsourcedialog.h"


LorrisAnalyzer::LorrisAnalyzer() : WorkTab()
{
    layout = new QVBoxLayout(this);

    QHBoxLayout *butt_1_layout = new QHBoxLayout();

    QPushButton *connectButt = new QPushButton(tr("Disconnect"), this);
    connectButt->setObjectName("connectButton");
    connect(connectButt, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *newsource_button = new QPushButton(tr("New data source"), this);
    newsource_button->setObjectName("newSourceButton");
    connect(newsource_button, SIGNAL(clicked()), this, SLOT(newSourceButton()));
    QSpacerItem *spacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);


    m_area = new QMdiArea(this);
    m_area->addSubWindow(new QLabel("fdfdas", this));
    m_area->addSubWindow(new QLabel("fdfdasěšššě", this));

    butt_1_layout->addWidget(connectButt);
    butt_1_layout->addWidget(newsource_button);
    butt_1_layout->addSpacerItem(spacer);
    layout->addLayout(butt_1_layout);
    layout->addWidget(m_area);

    dialog = NULL;
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
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

void LorrisAnalyzer::dataStructure(analyzer_packet pkt, QByteArray curData)
{

}

