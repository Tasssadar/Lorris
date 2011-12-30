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
#include "sourcedialog.h"
#include "ui_lorrisanalyzer.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab(),ui(new Ui::LorrisAnalyzer)
{
    ui->setupUi(this);

    QPushButton *connectButton = findChild<QPushButton*>("connectButton");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *addSourceButton = findChild<QPushButton*>("addSourceButton");
    connect(addSourceButton, SIGNAL(clicked()), this, SLOT(onTabShow()));
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    delete ui;
}

void LorrisAnalyzer::connectButton()
{
    QPushButton *connectButt = findChild<QPushButton *>("connectButton");
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

        connectButt->setText(tr("Connect"));
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
    if(m_state & STATE_DIALOG)
        return;
}

void LorrisAnalyzer::onTabShow()
{
    SourceDialog *d = new SourceDialog(this);
    connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SLOT(readData(QByteArray)));
    d->exec();
    delete d;
}


