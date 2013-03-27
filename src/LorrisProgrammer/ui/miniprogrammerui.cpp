/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QResizeEvent>

#include "miniprogrammerui.h"
#include "../../misc/utils.h"
#include "../lorrisprogrammer.h"
#include "../../misc/datafileparser.h"
#include "../../ui/tooltipwarn.h"

MiniProgrammerUI::MiniProgrammerUI(QObject *parent) :
    ProgrammerUI(UI_MINIMAL, parent), ui(new Ui::MiniProgrammerUI)
{
    m_fileSet = false;
    m_isVertical = true;
}

MiniProgrammerUI::~MiniProgrammerUI()
{
    Utils::deleteLayoutMembers(ui->mainLayout);
    delete ui->mainLayout;
    delete ui;
}

void MiniProgrammerUI::setupUi(LorrisProgrammer *widget)
{
    ProgrammerUI::setupUi(widget);

    ui->setupUi(widget);

    ui->verticalLayout->setAlignment(ui->writeButton, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->startStopBtn, Qt::AlignHCenter);

    connect(ui->writeButton,  SIGNAL(clicked()),         SLOT(writeFlashBtn()));
    connect(ui->startStopBtn, SIGNAL(clicked()), widget, SLOT(startstopChip()));
    connect(ui->loadBtn,      SIGNAL(clicked()), widget, SLOT(loadFromFile()));

    this->enableButtons(widget->m_buttons_enabled);

    m_widget->createConnBtn(ui->connectButton);

    widget->installEventFilter(this);
}

void MiniProgrammerUI::setChipId(const QString &text)
{
    QString str = text;
    if(str.size() >= 15)
        str = str.left(15) + "...";
    ui->chipIdLabel->setText(str);
    ui->chipIdLabel->setToolTip(text);
}

void MiniProgrammerUI::setFileAndTime(const QString &file, const QDateTime &time)
{
    m_fileSet = true;
    enableWrite(true);

    ui->loadBtn->setToolTip(file);

    QString str = time.toString(tr("h:mm:ss M.d.yyyy"));
    ui->filedate->setText(str);
    ui->filedate->setToolTip("Changed on " + str);
}

void MiniProgrammerUI::enableWrite(bool enable)
{
    ui->writeButton->setEnabled(enable && m_fileSet && m_widget->m_buttons_enabled);
}

void MiniProgrammerUI::saveData(DataFileParser *file)
{
    ProgrammerUI::saveData(file);

    file->writeBlockIdentifier("LorrShupitoTermSett");
    file->writeString(m_termSett);
    file->writeVal(m_termFmt);

    file->writeBlockIdentifier("LorrShupitoTermData");
    file->writeVal(m_termData.size());
    file->write(m_termData);

    file->writeBlockIdentifier("LorrShupitoSett");
    file->writeVal(m_curTabIdx);
    file->writeVal(m_showLog);
    file->writeVal(m_showFuses);
    file->writeVal(m_showSettings);

    file->writeBlockIdentifier("LorrShupitoProgSett");
    file->writeVal(m_widget->m_prog_speed_hz);
    file->writeVal(m_warnBox);

    file->writeBlockIdentifier("LorrShupitoTunnel");
    file->writeVal(m_tunnelEnabled);
    file->writeVal(m_tunnelSpeed);

    file->writeBlockIdentifier("LorrShupitoOvervcc");
    file->writeVal(m_over_enable);
    file->writeVal(m_over_turnoff);
    file->writeVal(m_over_val);
}

void MiniProgrammerUI::loadData(DataFileParser *file)
{
    ProgrammerUI::loadData(file);

    if(file->seekToNextBlock("LorrShupitoTermSett", BLOCK_WORKTAB))
    {
        m_termSett = file->readString();
        m_termFmt = file->readVal<int>();
    }

    if(file->seekToNextBlock("LorrShupitoTermData", BLOCK_WORKTAB))
    {
        int size = file->readVal<int>();
        m_termData = file->read(size);
    }

    if(file->seekToNextBlock("LorrShupitoSett", BLOCK_WORKTAB))
    {
        m_curTabIdx = file->readVal<int>();

        m_showLog = file->readVal<bool>();
        m_showFuses = file->readVal<bool>();
        m_showSettings = file->readVal<bool>();
    }

    if(file->seekToNextBlock("LorrShupitoProgSett", BLOCK_WORKTAB))
    {
        file->readVal<quint32>();
        m_warnBox = file->readVal<bool>();
    }

    if(file->seekToNextBlock("LorrShupitoTunnel", BLOCK_WORKTAB))
    {
        m_tunnelEnabled = file->readVal<bool>();
        m_tunnelSpeed = file->readVal<quint32>();
    }

    if(file->seekToNextBlock("LorrShupitoOvervcc", BLOCK_WORKTAB))
    {
        m_over_enable = file->readVal<bool>();
        m_over_turnoff = file->readVal<bool>();
        m_over_val = file->readVal<double>();
    }
}

void MiniProgrammerUI::writeSelectedMem()
{
    this->writeFlashBtn();
}

void MiniProgrammerUI::setVertical(bool vertical)
{
    if(m_isVertical == vertical)
        return;

    m_isVertical = vertical;

    QBoxLayout *from = ui->horLayout;
    QBoxLayout *to = ui->vertLayout;

    if(!vertical)
        std::swap(from, to);

    while(from->count() != 0)
    {
        QLayoutItem *i = from->takeAt(0);
        if(i->layout())
        {
            i->layout()->setParent(0);
            to->addLayout(i->layout());
        }
        else
            to->addItem(i);
    }

    delete to->takeAt(to->count()-1);
    to->addStretch(1);
}

bool MiniProgrammerUI::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::Resize)
        return QObject::eventFilter(obj, event);

    QResizeEvent *e = (QResizeEvent*)event;
    setVertical(e->size().width() < e->size().height());
    return QObject::eventFilter(obj, event);
}

void MiniProgrammerUI::warnSecondFlash()
{
    if(m_warnBox)
    {
        new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->writeButton, m_widget);
        Utils::playErrorSound();
    }
}

void MiniProgrammerUI::setHexData(quint32 memid, const QByteArray& data)
{
    if (memid == MEM_JTAG)
        m_svfData = data;
    else
        m_hexData[memid] = data;
}

QByteArray MiniProgrammerUI::getHexData(quint32 memid) const
{
    if (memid == MEM_JTAG)
        return m_svfData;
    else
        return m_hexData[memid];
}

void MiniProgrammerUI::enableButtons(bool enable)
{
    this->enableWrite(enable);
    ui->startStopBtn->setEnabled(enable);
}
