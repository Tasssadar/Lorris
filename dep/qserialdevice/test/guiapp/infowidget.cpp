#include "infowidget.h"
#include "ui_infowidget.h"

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget)
{
    ui->setupUi(this);
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::updateInfo(const InfoData &data)
{
    this->setWindowTitle(QString(tr("Info about: %1")).arg(data.name));
    ui->busLabel->setText(QString(tr("Bus: %1")).arg(data.bus));
    ui->busyLabel->setText(QString(tr("Is busy: %1")).arg(data.busy));
    ui->descrLabel->setText(QString(tr("Description: %1")).arg(data.description));
    ui->driverLabel->setText(QString(tr("Driver: %1")).arg(data.driver));
    ui->exLabel->setText(QString(tr("Is exists: %1")).arg(data.exists));
    ui->friendlyLabel->setText(QString(tr("Friendly name: %1")).arg(data.friendlyName));
    ui->locationLabel->setText(QString(tr("Location: %1")).arg(data.locationInfo));
    ui->mfgLabel->setText(QString(tr("Manufacturer: %1")).arg(data.manufacturer));
    ui->pidLabel->setText(QString(tr("Product ID: %1")).arg(data.productID));
    ui->revLabel->setText(QString(tr("Revision: %1")).arg(data.revision));
    ui->serviceLabel->setText(QString(tr("Service: %1")).arg(data.service));
    ui->shortLabel->setText(QString(tr("Short name: %1")).arg(data.shortName));
    ui->subsysLabel->setText(QString(tr("Subsystem: %1")).arg(data.subSystem));
    ui->syspathLabel->setText(QString(tr("System path: %1")).arg(data.systemPath));
    ui->vidLabel->setText(QString(tr("Vendor ID: %1")).arg(data.vendorID));
}


