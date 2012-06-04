/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFileDialog>

#include "mcu_prototype.h"
#include "lorrisavr.h"

LorrisAVR::LorrisAVR() : WorkTab(),ui(new Ui::LorrisAVR)
{
    ui->setupUi(this);

    m_prototypes.push_back(&atmega328p);

    for(quint32 i = 0; i < m_prototypes.size(); ++i)
        ui->mcuBox->addItem(m_prototypes[i]->name);

    ui->fileText->setText(sConfig.get(CFG_STRING_AVR_HEX));
    ui->freqBox->setValue(sConfig.get(CFG_QUINT32_AVR_FREQ));

    connect(&m_mcu, SIGNAL(realFreq(quint32)), SLOT(realFreq(quint32)));
    connect(&m_mcu, SIGNAL(hackToEmulator(char)), SLOT(hackToTerm(char)));
}

LorrisAVR::~LorrisAVR()
{
    delete ui;
}

void LorrisAVR::on_browseBtn_clicked()
{
    static const QString filter = tr("Intel HEX file (*.hex)");
    QString name = QFileDialog::getOpenFileName(this, tr("Choose file"), sConfig.get(CFG_STRING_AVR_HEX), filter);
    if(name.isEmpty())
        return;

    sConfig.set(CFG_STRING_AVR_HEX, name);
    ui->fileText->setText(name);
}

void LorrisAVR::on_startBtn_clicked()
{
    if(!m_mcu.isRunning())
    {
        try
        {
            HexFile file;
            file.LoadFromFile(ui->fileText->text());

            m_mcu.init(&file, m_prototypes[ui->mcuBox->currentIndex()]);
            m_mcu.startMCU();

            ui->startBtn->setText(tr("Stop"));
            ui->pauseBtn->setEnabled(true);
            ui->fileText->setEnabled(false);
            ui->mcuBox->setEnabled(false);
        }
        catch(const QString& text)
        {
            Utils::ThrowException(text, this);
        }
    }
    else
    {
        if(m_mcu.isPaused())
            on_pauseBtn_clicked();

        m_mcu.stopMCU();

        ui->startBtn->setText(tr("Start"));
        ui->realFreq->setText(tr("N/A"));
        ui->pauseBtn->setEnabled(false);
        ui->fileText->setEnabled(true);
        ui->mcuBox->setEnabled(true);
    }
}

void LorrisAVR::on_freqBox_valueChanged(int i)
{
    m_mcu.setFreq(i);
    sConfig.set(CFG_QUINT32_AVR_FREQ, i);
}

void LorrisAVR::on_pauseBtn_clicked()
{
    m_mcu.setPaused(!m_mcu.isPaused());
    if(m_mcu.isPaused())
        ui->pauseBtn->setText(tr("Unpause"));
    else
        ui->pauseBtn->setText(tr("Pause"));
}

void LorrisAVR::realFreq(quint32 freq)
{
    static const QString app[] = { "%1 Hz", "%1 KHz", "%1 MHz" };
    static const float div[] = { 1, 1000, 1000000 };

    int appIdx = 0;
    if     (freq >= 1000000)   appIdx = 2;
    else if(freq >= 1000)      appIdx = 1;

    float res = freq/div[appIdx];
    ui->realFreq->setText(app[appIdx].arg(res, -3, 'f', 2, ' '));
}

void LorrisAVR::hackToTerm(char c)
{
    ui->terminal->appendText(QByteArray(1, c));
}
