/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QResizeEvent>

#include "minishupitoui.h"
#include "../../misc/utils.h"
#include "../lorrisshupito.h"
#include "flashbuttonmenu.h"
#include "../../misc/datafileparser.h"

MiniShupitoUI::MiniShupitoUI(QObject *parent) :
    ShupitoUI(UI_MINIMAL, parent), ui(new Ui::MiniShupitoUI)
{
    m_fileSet = false;
    m_isVertical = true;
}

MiniShupitoUI::~MiniShupitoUI()
{
    Utils::deleteLayoutMembers(ui->mainLayout);
    delete ui->mainLayout;
    delete ui;
}

void MiniShupitoUI::setupUi(LorrisShupito *widget)
{
    ShupitoUI::setupUi(widget);

    ui->setupUi(widget);

    ui->verticalLayout->setAlignment(ui->writeButton, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->startStopBtn, Qt::AlignHCenter);

    connect(ui->writeButton,  SIGNAL(clicked()),         SLOT(writeFlashBtn()));
    connect(ui->startStopBtn, SIGNAL(clicked()), widget, SLOT(startstopChip()));
    connect(ui->loadBtn,      SIGNAL(clicked()), widget, SLOT(loadFromFile()));
    connect(this, SIGNAL(enableButtons(bool)),           SLOT(enableWrite(bool)));
    connect(this, SIGNAL(enableButtons(bool)), ui->startStopBtn, SLOT(setEnabled(bool)));

    emit enableButtons(widget->m_buttons_enabled);

    m_widget->createConnBtn(ui->connectButton);

    widget->installEventFilter(this);
}

void MiniShupitoUI::setChipId(const QString &text)
{
    QString str = text;
    if(str.size() >= 15)
        str = str.left(15) + "...";
    ui->chipIdLabel->setText(str);
    ui->chipIdLabel->setToolTip(text);
}

void MiniShupitoUI::setFileAndTime(const QString &file, const QDateTime &time)
{
    m_fileSet = true;
    enableWrite(true);

    ui->loadBtn->setToolTip(file);

    QString str = time.toString(tr("h:mm:ss M.d.yyyy"));
    ui->filedate->setText(str);
    ui->filedate->setToolTip("Changed on " + str);
}

void MiniShupitoUI::enableWrite(bool enable)
{
    ui->writeButton->setEnabled(enable && m_fileSet && m_widget->m_buttons_enabled);
}

void MiniShupitoUI::saveData(DataFileParser *file)
{
    ShupitoUI::saveData(file);

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

void MiniShupitoUI::loadData(DataFileParser *file)
{
    ShupitoUI::loadData(file);

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

void MiniShupitoUI::setVertical(bool vertical)
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
        QLayoutItem *i = from->itemAt(0);
        from->removeItem(i);
        to->addItem(i);
    }

    to->removeItem(to->itemAt(to->count()-1));
    to->addStretch(1);
}

bool MiniShupitoUI::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::Resize)
        return QObject::eventFilter(obj, event);

    QResizeEvent *e = (QResizeEvent*)event;
    setVertical(e->size().width() < e->size().height());
    return QObject::eventFilter(obj, event);
}
