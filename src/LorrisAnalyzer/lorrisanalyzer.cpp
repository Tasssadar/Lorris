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
#include <QSlider>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "lorrisanalyzer.h"
#include "sourcedialog.h"
#include "ui_lorrisanalyzer.h"
#include "packet.h"
#include "analyzerdatastorage.h"
#include "devicetabwidget.h"
#include "analyzerdataarea.h"
#include "connection/connectionmgr.h"
#include "DataWidgets/datawidget.h"
#include "DataWidgets/numberwidget.h"
#include "DataWidgets/barwidget.h"
#include "sourceselectdialog.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab(),ui(new Ui::LorrisAnalyzer)
{
    ui->setupUi(this);

    m_storage = new AnalyzerDataStorage();

    QPushButton *connectButton = findChild<QPushButton*>("connectButton");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *addSourceButton = findChild<QPushButton*>("addSourceButton");
    connect(addSourceButton, SIGNAL(clicked()), this, SLOT(onTabShow()));

    QPushButton *saveDataButton = findChild<QPushButton*>("saveDataButton");
    connect(saveDataButton, SIGNAL(clicked()), this, SLOT(saveDataButton()));

    QPushButton *loadDataButton = findChild<QPushButton*>("loadDataButton");
    connect(loadDataButton, SIGNAL(clicked()), this, SLOT(loadDataButton()));

    timeSlider = findChild<QSlider*>("timeSlider");
    connect(timeSlider, SIGNAL(valueChanged(int)), this, SLOT(timeSliderMoved(int)));

    timeBox = findChild<QSpinBox*>("timeBox");
    connect(timeBox, SIGNAL(valueChanged(int)), this, SLOT(timeBoxChanged(int)));

    m_dev_tabs = new DeviceTabWidget(this);

    QVBoxLayout *leftVLayout = findChild<QVBoxLayout*>("leftVLayout");
    leftVLayout->insertWidget(1, m_dev_tabs);

    connect(this, SIGNAL(newData(analyzer_data*)), m_dev_tabs, SLOT(handleData(analyzer_data*)));
    connect(m_dev_tabs, SIGNAL(updateData()), this, SLOT(updateData()));

    m_data_area = new AnalyzerDataArea(this);
    QHBoxLayout *bottomHLayout = findChild<QHBoxLayout*>("bottomHLayout");
    bottomHLayout->insertWidget(0, m_data_area, 5);
    bottomHLayout->setStretch(1, 1);
    connect(m_data_area, SIGNAL(updateData()), this, SLOT(updateData()));

    QScrollArea *widgetsScrollArea = findChild<QScrollArea*>("widgetsScrollArea");
    QWidget *tmp = new QWidget(this);
    QVBoxLayout *widgetBtnL = new QVBoxLayout(tmp);
    widgetBtnL->addWidget(new NumberWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new BarWidgetAddBtn(tmp));

    widgetBtnL->addWidget(new QWidget(tmp), 4);
    widgetsScrollArea->setWidget(tmp);

    setMouseTracking(true);

    m_packet = NULL;
    m_state = 0;
    highlightInfoNotNull = false;
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    delete ui;
    delete m_storage;
    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    delete m_dev_tabs;
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

void LorrisAnalyzer::readData(const QByteArray& data)
{
    if((m_state & STATE_DIALOG) != 0 || !m_packet)
        return;

    analyzer_data *a_data = m_storage->addData(data);
    if(!a_data)
        return;

    bool update = timeSlider->value() == timeSlider->maximum();
    timeSlider->setMaximum(m_storage->getSize());
    timeBox->setMaximum(m_storage->getSize());
    timeBox->setSuffix(tr(" of ") + QString::number(m_storage->getSize()));
    if(update)
    {
        timeSlider->setValue(m_storage->getSize());
        timeBox->setValue(m_storage->getSize());
    }
}

void LorrisAnalyzer::onTabShow()
{
    SourceSelectDialog *s = new SourceSelectDialog(this);

    if(m_con->getType() == CONNECTION_FILE)
        s->DisableNew();

    qint8 res = s->get();
    switch(res)
    {
        case -1: break;
        case 0:
        {
            QString file = s->getFileName();
            quint8 mask = s->getDataMask();
            load(&file, mask);
            break;
        }
        case 1:
        {
            SourceDialog *d = new SourceDialog(this);
            m_state |= STATE_DIALOG;
            connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SLOT(readData(QByteArray)));

            analyzer_packet *packet = d->getStructure();
            delete d;
            m_state &= ~(STATE_DIALOG);
            if(!packet)
                return;

            if(m_packet)
            {
                delete m_packet->header;
                delete m_packet;
            }
            m_storage->Clear();
            m_dev_tabs->removeAll();
            m_dev_tabs->setHeader(packet->header);
            m_dev_tabs->addDevice();

            m_storage->setPacket(packet);
            m_packet = packet;
            break;
        }
    }
    delete s;
}

void LorrisAnalyzer::timeSliderMoved(int value)
{
    if(value != 0)
        updateData();

    if(timeSlider->isSliderDown())
        timeBox->setValue(value);
}

void LorrisAnalyzer::timeBoxChanged(int value)
{
    timeSlider->setValue(value);
}

void LorrisAnalyzer::updateData()
{
    int val = timeSlider->value();

    if(val != 0)
        emit newData(m_storage->get(val-1));
}

void LorrisAnalyzer::loadDataButton()
{
    load(NULL, (STORAGE_STRUCTURE | STORAGE_DATA | STORAGE_WIDGETS));
}

void LorrisAnalyzer::load(QString *name, quint8 mask)
{
    analyzer_packet *packet = m_storage->loadFromFile(name, mask, m_data_area, m_dev_tabs);
    if(!packet)
        return;
    m_packet = packet;

    if(!m_dev_tabs->count())
    {
        m_dev_tabs->removeAll();
        m_dev_tabs->setHeader(packet->header);
        m_dev_tabs->addDevice();
    }

    timeSlider->setMaximum(m_storage->getSize());
    timeSlider->setValue(m_storage->getSize());
    timeBox->setMaximum(m_storage->getSize());
    timeBox->setSuffix(tr(" of ") + QString::number(m_storage->getSize()));
    timeBox->setValue(m_storage->getSize());
}

void LorrisAnalyzer::saveDataButton()
{
    m_storage->SaveToFile(m_data_area, m_dev_tabs);
}

void LorrisAnalyzer::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->modifiers() & Qt::ShiftModifier))
    {
        WorkTab::mouseMoveEvent(event);
        return;
    }

    if(DataWidget *w = m_data_area->isMouseInWidget())
    {
        if(highlightInfoNotNull && highlightInfo != w->getInfo())
            m_dev_tabs->setHighlightPos(highlightInfo, false);

        bool found = m_dev_tabs->setHighlightPos(w->getInfo(), true);

        if(!found)
            return;

        highlightInfo = w->getInfo();
        highlightInfoNotNull = true;
    }
    else if(highlightInfoNotNull)
    {
        m_dev_tabs->setHighlightPos(highlightInfo, false);
        highlightInfoNotNull = false;
    }
}

void LorrisAnalyzer::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
    {
        QMouseEvent *ev = new QMouseEvent(QEvent::None, QCursor::pos(), Qt::NoButton, Qt::NoButton, Qt::ShiftModifier);
        mouseMoveEvent(ev);
        delete ev;
    }
}

void LorrisAnalyzer::keyReleaseEvent(QKeyEvent *event)
{
    if(highlightInfoNotNull && event->key() == Qt::Key_Shift)
    {
        m_dev_tabs->setHighlightPos(highlightInfo, false);
        highlightInfoNotNull = false;
    }
}
