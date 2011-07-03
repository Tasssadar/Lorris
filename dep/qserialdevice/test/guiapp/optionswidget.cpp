#include <QtGui/QMessageBox>

#include "optionswidget.h"
#include "ui_optionswidget.h"

OptionsWidget::OptionsWidget(const QStringList baudList,
                             const QStringList dataList,
                             const QStringList parityList,
                             const QStringList stopList,
                             const QStringList flowList,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsWidget)
{
    ui->setupUi(this);
    this->fillingBoxes(baudList, dataList, parityList, stopList, flowList);
    this->initButtonConnections();
    this->initBoxConnections();
}

OptionsWidget::~OptionsWidget()
{
    delete ui;
}

void OptionsWidget::updateOptions(const OptionsData &data)
{
    QString datatext;
    int counter = 5; // 5 - count options boxes

    this->setWindowTitle(QString(tr("Device Settings: %1").arg(data.name)));

    while (counter--) {
        QComboBox *box = 0;

        switch (counter) {
        case 0: box = ui->baudBox; datatext = data.baud; break;
        case 1: box = ui->dataBox; datatext = data.data; break;
        case 2: box = ui->parityBox; datatext = data.parity; break;
        case 3: box = ui->stopBox; datatext = data.stop; break;
        case 4: box = ui->flowBox; datatext = data.flow; break;
        default: return;
        }

        QString boxtext = box->currentText();
        if (box && (boxtext != datatext)) {
            int idx = box->findText(datatext);
            if (-1 != idx)
                box->setCurrentIndex(idx);
        }
    }
}

void OptionsWidget::procAppliedOptions(bool applied, const QStringList &notAppliedList)
{
    if (!applied) {
        QMessageBox msgBox;
        msgBox.setText("Error.");
        QString notApplStr;
        foreach (QString s, notAppliedList) {
            notApplStr.append(QString("\n %1").arg(s));
        }

        msgBox.setInformativeText(QString(tr("Not applied: %1").arg(notApplStr)));
        msgBox.exec();
    }
    ui->applyButton->setEnabled(!applied);
}

void OptionsWidget::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    ui->applyButton->setEnabled(false);
}

void OptionsWidget::procBoxChanged()
{
    if (this->isVisible())
        ui->applyButton->setEnabled(true);
}

void OptionsWidget::procApplyButtonClick()
{
    QStringList list;
    list << ui->baudBox->currentText()
            << ui->dataBox->currentText() << ui->parityBox->currentText()
            << ui->stopBox->currentText() << ui->flowBox->currentText();
    emit this->applyOptions(list);
}

void OptionsWidget::fillingBoxes(const QStringList baudList,
                                 const QStringList dataList,
                                 const QStringList parityList,
                                 const QStringList stopList,
                                 const QStringList flowList)
{
    ui->baudBox->addItems(baudList);
    ui->dataBox->addItems(dataList);
    ui->parityBox->addItems(parityList);
    ui->stopBox->addItems(stopList);
    ui->flowBox->addItems(flowList);
}

void OptionsWidget::initButtonConnections()
{
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(procApplyButtonClick()));
}

void OptionsWidget::initBoxConnections()
{
    connect(ui->baudBox, SIGNAL(currentIndexChanged(int)), this, SLOT(procBoxChanged()));
    connect(ui->dataBox, SIGNAL(currentIndexChanged(int)), this, SLOT(procBoxChanged()));
    connect(ui->parityBox, SIGNAL(currentIndexChanged(int)), this, SLOT(procBoxChanged()));
    connect(ui->stopBox, SIGNAL(currentIndexChanged(int)), this, SLOT(procBoxChanged()));
    connect(ui->flowBox, SIGNAL(currentIndexChanged(int)), this, SLOT(procBoxChanged()));
}
