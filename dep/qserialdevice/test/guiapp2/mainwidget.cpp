#include <QtCore/QStringList>
#include <QtCore/QTimer>
//#include <QtCore/QDebug>

#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "infowidget.h"
#include "optionswidget.h"
#include "tracewidget.h"

#include <serialdeviceenumerator.h>
#include <abstractserial.h>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    infoWidget(0), optionsWidget(0), traceWidget(0),
    enumerator(0),
    serial(0),
    timer(0)
{
    ui->setupUi(this);

    this->initEnumerator();
    this->initSerial();
    this->initButtonConnections();
    this->initBoxConnections();
    this->initMainWidgetCloseState();
    //
    ui->plainTextEdit->document()->setMaximumBlockCount(100);
}

MainWidget::~MainWidget()
{
    this->deinitEnumerator();
    this->deinitSerial();
    this->deinitWidgets();

    delete ui;
}

void MainWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/* Private slots section */

void MainWidget::procEnumerate(const QStringList &l)
{
    // Fill ports box.
    ui->portBox->clear();
    ui->portBox->addItems(l);
}

void MainWidget::procSerialMessages(const QString &msg, QDateTime dt)
{
    QString s = dt.time().toString() + " > " + msg;
    ui->plainTextEdit->appendPlainText(s);
}

void MainWidget::procSerialDataReceive()
{
    if (this->initTraceWidget() && this->serial && this->serial->isOpen()) {
        this->initTimer();

        if (this->rxBuffer.isEmpty())
            this->timer->start();

        this->rxBuffer.append(this->serial->readAll());

        //QByteArray data = this->serial->readAll();
        //qDebug() << "Rx: " << data;
       // this->traceWidget->printTrace(data, true);
    }
}

void MainWidget::procSerialDataTransfer(const QByteArray &data)
{
    if (this->serial && this->serial->isOpen())
        this->serial->write(data);
}

void MainWidget::procCtsChanged(bool val)
{
    ui->ctsLabel->setEnabled(val);
}

void MainWidget::procDsrChanged(bool val)
{
    ui->dsrLabel->setEnabled(val);
}

void MainWidget::procRingChanged(bool val)
{
    ui->ringLabel->setEnabled(val);
}

void MainWidget::procApplyOptions(const QStringList &list)
{
    if (this->serial && this->serial->isOpen()) {
        QStringList notApplyList;
        bool result = true;
        if ((this->serial->baudRate() != list.at(0)) &&
            (!this->serial->setBaudRate(list.at(0)))) {
            notApplyList << list.at(0);
            result = false;
        }

        if ((this->serial->dataBits() != list.at(1)) &&
            (!this->serial->setDataBits(list.at(1)))) {
            notApplyList << list.at(1);
            result = false;
        }

        if ((this->serial->parity() != list.at(2)) &&
            (!this->serial->setParity(list.at(2)))) {
            notApplyList << list.at(2);
            result = false;
        }

        if ((this->serial->stopBits() != list.at(3)) &&
            (!this->serial->setStopBits(list.at(3)))) {
            notApplyList << list.at(3);
            result = false;
        }

        if ((this->serial->flowControl() != list.at(4)) &&
            (!this->serial->setFlowControl(list.at(4)))) {
            notApplyList << list.at(4);
            result = false;
        }

        if ((!result) && this->initInfoWidget())
            this->updateOptionsData();

        emit this->optionsApplied(result, notApplyList);
    }
}

void MainWidget::procControlButtonClick()
{
    if (this->serial) {
        bool result = this->serial->isOpen();
        if (result) {
            this->serial->close();
            result = false;
        }
        else {
            this->serial->setDeviceName(ui->portBox->currentText());
            result = this->serial->open(QIODevice::ReadWrite);
        }

        (result) ? this->initMainWidgetOpenState() : this->initMainWidgetCloseState();
    }
}

void MainWidget::procOptionsButtonClick()
{
    if (this->initOptionsWidget()) {
        this->updateOptionsData();
        this->optionsWidget->show();
    }
}

void MainWidget::procInfoButtonClick()
{
    if (this->initInfoWidget()) {
        this->updateInfoData(ui->portBox->currentText());
        this->infoWidget->show();
    }
}

void MainWidget::procIOButtonClick()
{
    if (this->initTraceWidget() && this->serial && this->serial->isOpen()) {
        this->traceWidget->setTitle(this->serial->deviceName());
        this->traceWidget->show();
    }
}

void MainWidget::procRtsButtonClick()
{
    bool result = this->serial && this->serial->isOpen();
    if (result) {
        // Get Rts state
        result = AbstractSerial::LineRTS & this->serial->lineStatus();
        this->serial->setRts(!result);
        this->detectSerialLineStates();
    }
}

void MainWidget::procDtrButtonClick()
{
    bool result = this->serial && this->serial->isOpen();
    if (result) {
        // Get Dtr state
        result = AbstractSerial::LineDTR & this->serial->lineStatus();
        this->serial->setDtr(!result);
        this->detectSerialLineStates();
    }
}

void MainWidget::procBoxChange(const QString &item)
{
    if (this->initInfoWidget())
        this->updateInfoData(item);
}

void MainWidget::procTimerOut(){
    this->timer->stop();
    this->traceWidget->printTrace(this->rxBuffer, true);
    this->rxBuffer.clear();
}

/* Private methods section */

void MainWidget::initMainWidgetCloseState()
{
    ui->portBox->setEnabled(true);
    ui->optionsButton->setEnabled(false);
    ui->ioButton->setEnabled(false);
    ui->rtsButton->setEnabled(false);
    ui->dtrButton->setEnabled(false);
    ui->controlButton->setText(QString(tr("Open")));

    this->detectSerialLineStates();

    if (this->optionsWidget && this->optionsWidget->isVisible())
        this->optionsWidget->hide();
    if (this->traceWidget && this->traceWidget->isVisible())
        this->traceWidget->hide();
}

void MainWidget::initMainWidgetOpenState()
{
    ui->portBox->setEnabled(false);
    ui->optionsButton->setEnabled(true);
    ui->ioButton->setEnabled(true);
    ui->rtsButton->setEnabled(true);
    ui->dtrButton->setEnabled(true);
    ui->controlButton->setText(QString(tr("Close")));

    this->detectSerialLineStates();
}

bool MainWidget::initInfoWidget()
{
    if (!this->infoWidget) {
        this->infoWidget = new InfoWidget();
        if (!this->infoWidget)
            return false;
    }
    return true;
}

bool MainWidget::initOptionsWidget()
{
    if (!this->optionsWidget && this->serial) {
        this->optionsWidget = new OptionsWidget(this->serial->listBaudRate(),
                                                this->serial->listDataBits(),
                                                this->serial->listParity(),
                                                this->serial->listStopBits(),
                                                this->serial->listFlowControl());
        if (!this->optionsWidget)
            return false;

        connect(this, SIGNAL(optionsApplied(bool,QStringList)),
                this->optionsWidget, SLOT(procAppliedOptions(bool,QStringList)));
        connect(this->optionsWidget, SIGNAL(applyOptions(QStringList)),
                this, SLOT(procApplyOptions(QStringList)));
    }
    return true;
}

bool MainWidget::initTraceWidget()
{
    if (!this->traceWidget) {
        this->traceWidget = new TraceWidget();
        if (!this->traceWidget)
            return false;

        connect(this->traceWidget, SIGNAL(sendSerialData(QByteArray)),
                this, SLOT(procSerialDataTransfer(QByteArray)));
    }
    return true;
}

void MainWidget::initEnumerator()
{
    if (!this->enumerator)
        this->enumerator = SerialDeviceEnumerator::instance();
    connect(this->enumerator, SIGNAL(hasChanged(QStringList)), this, SLOT(procEnumerate(QStringList)));
    this->procEnumerate(this->enumerator->devicesAvailable());
}

void MainWidget::deinitEnumerator()
{
}

void MainWidget::initSerial()
{
    if (this->serial)
        return;
    this->serial = new AbstractSerial(this);
    connect(this->serial, SIGNAL(signalStatus(QString,QDateTime)), this, SLOT(procSerialMessages(QString,QDateTime)));
    connect(this->serial, SIGNAL(ctsChanged(bool)), this, SLOT(procCtsChanged(bool)));
    connect(this->serial, SIGNAL(dsrChanged(bool)), this, SLOT(procDsrChanged(bool)));
    connect(this->serial, SIGNAL(ringChanged(bool)), this, SLOT(procRingChanged(bool)));
    connect(this->serial, SIGNAL(readyRead()), this, SLOT(procSerialDataReceive()));

    // Enable emmiting signal signalStatus
    this->serial->enableEmitStatus(true);
}

void MainWidget::deinitSerial()
{
    if (this->serial && this->serial->isOpen())
        this->serial->close();
}

void MainWidget::initTimer()
{
    if (this->timer)
        return;

    this->timer = new QTimer(this);
    this->timer->setInterval(50);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(procTimerOut()));
}

void MainWidget::deinitTimer()
{

}

void MainWidget::initButtonConnections()
{
    connect(ui->controlButton, SIGNAL(clicked()), this, SLOT(procControlButtonClick()));
    connect(ui->infoButton, SIGNAL(clicked()), this, SLOT(procInfoButtonClick()));
    connect(ui->optionsButton, SIGNAL(clicked()), this, SLOT(procOptionsButtonClick()));
    connect(ui->ioButton, SIGNAL(clicked()), this, SLOT(procIOButtonClick()));

    connect(ui->rtsButton, SIGNAL(clicked()), this, SLOT(procRtsButtonClick()));
    connect(ui->dtrButton, SIGNAL(clicked()), this, SLOT(procDtrButtonClick()));
}

void MainWidget::initBoxConnections()
{
    connect(ui->portBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(procBoxChange(QString)));
}

void MainWidget::deinitWidgets()
{
    if (this->infoWidget)
        delete (this->infoWidget);
    if (this->optionsWidget)
        delete (this->optionsWidget);
}

void MainWidget::setRtsDtrButtonsCaption(bool opened, bool rts, bool dtr)
{
    if (!opened) {
        ui->rtsButton->setText(QString(tr("Control RTS")));
        ui->dtrButton->setText(QString(tr("Control DTR")));
        return;
    }
    (rts) ? ui->rtsButton->setText(QString(tr("Clear RTS"))) : ui->rtsButton->setText(QString(tr("Set RTS")));
    (dtr) ? ui->dtrButton->setText(QString(tr("Clear DTR"))) : ui->dtrButton->setText(QString(tr("Set DTR")));
}

void MainWidget::detectSerialLineStates()
{
    bool opened = this->serial && this->serial->isOpen();
    quint16 line = 0;

    if (opened)
        line = this->serial->lineStatus();

    this->setRtsDtrButtonsCaption(opened,
                                  AbstractSerial::LineRTS & line, AbstractSerial::LineDTR & line);

    ui->ctsLabel->setEnabled(AbstractSerial::LineCTS & line);
    ui->dcdLabel->setEnabled(AbstractSerial::LineDCD & line);
    ui->dsrLabel->setEnabled(AbstractSerial::LineDSR & line);
    ui->dtrLabel->setEnabled(AbstractSerial::LineDTR & line);
    ui->leLabel->setEnabled(AbstractSerial::LineLE & line);
    ui->ringLabel->setEnabled(AbstractSerial::LineRI & line);
    ui->rtsLabel->setEnabled(AbstractSerial::LineRTS & line);
}

void MainWidget::updateInfoData(const QString &name)
{
    if (this->enumerator && this->infoWidget) {
        InfoWidget::InfoData data;

        this->enumerator->setDeviceName(name);
        data.name = name;
        data.bus = this->enumerator->bus();
        data.busy = this->enumerator->isBusy();
        data.description = this->enumerator->description();
        data.driver = this->enumerator->driver();
        data.exists = this->enumerator->isExists();
        data.friendlyName = this->enumerator->friendlyName();
        data.locationInfo = this->enumerator->locationInfo();
        data.manufacturer = this->enumerator->manufacturer();
        data.productID = this->enumerator->productID();
        data.revision = this->enumerator->revision();
        data.service = this->enumerator->service();
        data.shortName = this->enumerator->shortName();
        data.subSystem = this->enumerator->subSystem();
        data.systemPath = this->enumerator->systemPath();
        data.vendorID = this->enumerator->vendorID();

        this->infoWidget->updateInfo(data);
    }
}

void MainWidget::updateOptionsData()
{
    if (this->initOptionsWidget() && this->serial && this->serial->isOpen()) {
        OptionsWidget::OptionsData data;

        data.baud = this->serial->baudRate();
        data.data = this->serial->dataBits();
        data.parity = this->serial->parity();
        data.stop = this->serial->stopBits();
        data.flow = this->serial->flowControl();

        data.name = this->serial->deviceName();

        this->optionsWidget->updateOptions(data);
    }
}
