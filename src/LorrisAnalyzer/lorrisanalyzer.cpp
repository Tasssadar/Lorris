/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#define QT_USE_FAST_CONCATENATION

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
#include <QFileDialog>
#include <QStringBuilder>

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
#include "DataWidgets/colorwidget.h"
#include "DataWidgets/GraphWidget/graphwidget.h"
#include "DataWidgets/ScriptWidget/scriptwidget.h"
#include "DataWidgets/terminalwidget.h"
#include "DataWidgets/buttonwidget.h"
#include "sourceselectdialog.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab(),ui(new Ui::LorrisAnalyzer)
{
    ui->setupUi(this);

    minUpdateDelay = sConfig.get(CFG_QUINT32_ANALYZER_UPDATE_TIME);
    ui->updateTimeBox->setValue(minUpdateDelay);

    connect(ui->structureBtn,    SIGNAL(clicked()),         SLOT(editStruture()));
    connect(ui->connectButton,   SIGNAL(clicked()),         SLOT(connectButton()));
    connect(ui->collapseTop,     SIGNAL(clicked()),         SLOT(collapseTopButton()));
    connect(ui->collapseRight,   SIGNAL(clicked()),         SLOT(collapseRightButton()));
    connect(ui->collapseLeft,    SIGNAL(clicked()),         SLOT(collapseLeftButton()));
    connect(ui->clearButton,     SIGNAL(clicked()),         SLOT(clearDataButton()));
    connect(ui->timeSlider,      SIGNAL(valueChanged(int)), SLOT(timeSliderMoved(int)));
    connect(ui->timeBox,         SIGNAL(valueChanged(int)), SLOT(timeBoxChanged(int)));
    connect(ui->updateTimeBox,   SIGNAL(valueChanged(int)), SLOT(updateTimeChanged(int)));
    connect(ui->playFrame,       SIGNAL(setPos(int)),           ui->timeBox,    SLOT(setValue(int)));
    connect(ui->timeSlider,      SIGNAL(valueChanged(int)),     ui->playFrame,  SLOT(valChanged(int)));
    connect(ui->timeSlider,      SIGNAL(rangeChanged(int,int)), ui->playFrame,  SLOT(rangeChanged(int,int)));
    connect(ui->playFrame,       SIGNAL(enablePosSet(bool)),    ui->timeBox,    SLOT(setEnabled(bool)));
    connect(ui->playFrame,       SIGNAL(enablePosSet(bool)),    ui->timeSlider, SLOT(setEnabled(bool)));
    connect(ui->dataArea,        SIGNAL(updateData()),      SLOT(updateData()));
    connect(ui->devTabs,         SIGNAL(updateData()),      SLOT(updateData()));
    connect(ui->dataArea,        SIGNAL(mouseStatus(bool,data_widget_info,qint32)),
                                 SLOT(widgetMouseStatus(bool,data_widget_info, qint32)));
    connect(this,                SIGNAL(newData(analyzer_data*,quint32)), ui->devTabs,
                                 SLOT(handleData(analyzer_data*, quint32)));


    int h = ui->collapseLeft->fontMetrics().height()+10;
    ui->collapseLeft->setFixedWidth(h);
    ui->collapseRight->setFixedWidth(h);
    ui->collapseTop->setFixedHeight(h);
    ui->collapseLeft->setRotation(ROTATE_270);
    ui->collapseRight->setRotation(ROTATE_90);

    QMenu* menuData = new QMenu(tr("&Data"), this);

    QAction* newSource = menuData->addAction(tr("New source..."));
    menuData->addSeparator();
    QAction* openAct = menuData->addAction(tr("Open data..."));
    QAction* saveAct = menuData->addAction(tr("Save data..."));
    menuData->addSeparator();
    QAction* clearAct = menuData->addAction(tr("Clear received data"));
    QAction* clearAllAct = menuData->addAction(tr("Clear everything"));

    openAct->setShortcut(QKeySequence("Ctrl+O"));
    saveAct->setShortcut(QKeySequence("Ctrl+S"));

    connect(newSource, SIGNAL(triggered()), SLOT(onTabShow()));
    connect(openAct,   SIGNAL(triggered()), SLOT(openFile()));
    connect(saveAct,   SIGNAL(triggered()), SLOT(saveDataButton()));
    connect(clearAct,  SIGNAL(triggered()), SLOT(clearDataButton()));
    connect(clearAllAct,  SIGNAL(triggered()), SLOT(clearAllButton()));

    addTopMenu(menuData);

    // Time box update consumes hilarious CPU time on X11,
    // this makes it better
#if defined(Q_WS_X11)
    ui->timeBox->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
    ui->timeBox->setAttribute(Qt::WA_PaintOnScreen, true);
#endif

    m_storage = new AnalyzerDataStorage(this);
    ui->dataArea->setAnalyzerAndStorage(this, m_storage);

    ui->devTabs->addDevice();

    QWidget *tmp = new QWidget(this);
    QVBoxLayout *widgetBtnL = new QVBoxLayout(tmp);
    widgetBtnL->addWidget(new NumberWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new BarWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new ColorWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new GraphWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new ScriptWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new TerminalWidgetAddBtn(tmp));
    widgetBtnL->addWidget(new ButtonWidgetAddBtn(tmp));

    widgetBtnL->addWidget(new QWidget(tmp), 4);
    ui->widgetsScrollArea->setWidget(tmp);

    m_packet = NULL;
    m_state = 0;
    highlightInfoNotNull = false;
    m_curData = NULL;

    updateTime.start();

    setAreaVisibility(AREA_LEFT, false);
    setAreaVisibility(AREA_RIGHT, true);
    setAreaVisibility(AREA_TOP, true);
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    delete m_storage;
    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    delete ui->devTabs;
    delete ui;
    delete m_curData;
}

void LorrisAnalyzer::connectButton()
{
    if(m_state & STATE_DISCONNECTED)
    {
        ui->connectButton->setText(tr("Connecting..."));
        ui->connectButton->setEnabled(false);
        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
    else
    {
        m_con->Close();
        m_state |= STATE_DISCONNECTED;

        ui->connectButton->setText(tr("Connect"));
    }
}

void LorrisAnalyzer::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    ui->connectButton->setEnabled(true);
    if(!result)
    {
        ui->connectButton->setText(tr("Connect"));
        showErrorBox(tr("Can't open connection!"));
    }
}

void LorrisAnalyzer::connectedStatus(bool connected)
{
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        ui->connectButton->setText(tr("Disconnect"));
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        ui->connectButton->setText(tr("Connect"));
    }
}

void LorrisAnalyzer::readData(const QByteArray& data)
{
    if((m_state & STATE_DIALOG) != 0 || !m_packet)
        return;

    if(!m_curData)
        m_curData = new analyzer_data(m_packet);

    char *d_start = (char*)data.data();
    char *d_itr = d_start;
    char *d_end = (char*)data.data() + data.size();

    static bool first = true;
    quint32 curRead = 1;

    while(d_itr != d_end)
    {
        if(first || curRead == 0)
        {
            int index = data.indexOf(m_curData->getStaticData(), d_itr - d_start);
            if(index == -1)
                break;
            d_itr = d_start + index;
            first = false;
        }
        curRead = m_curData->addData(d_itr, d_end);
        d_itr += curRead;

        if(m_curData->isValid())
        {
            m_storage->addData(m_curData);
            m_curData = new analyzer_data(m_packet);
        }
    }

    if(!canUpdateUi())
        return;

    int size = m_storage->getSize();
    bool update = ui->timeSlider->value() == ui->timeSlider->maximum();

    ui->timeSlider->setMaximum(size);
    ui->timeBox->setMaximum(size);

    static const QString ofString = tr(" of ");
    ui->timeBox->setSuffix(ofString % QString::number(size));

    if(update)
        ui->timeBox->setValue(size);
}

void LorrisAnalyzer::onTabShow()
{
    m_state |= STATE_DIALOG;
    SourceSelectDialog *s = new SourceSelectDialog(this);

    if(m_con->getType() == CONNECTION_FILE)
        s->DisableNew();

    qint8 res = s->get();
    switch(res)
    {
        case -1:
            m_state &= ~(STATE_DIALOG);
            break;
        case 0:
        {
            QString file = s->getFileName();
            quint8 mask = s->getDataMask();
            load(&file, mask);
            break;
        }
        case 1:
        {
            SourceDialog *d = new SourceDialog(NULL, this);
            m_state |= STATE_DIALOG;
            connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SLOT(readData(QByteArray)));

            analyzer_packet *packet = d->getStructure();
            delete d;

            {
                m_state &= ~(STATE_DIALOG);
                if(!packet)
                    return;

                m_curData = new analyzer_data(packet);

                if(m_packet)
                {
                    delete m_packet->header;
                    delete m_packet;
                }
                ui->dataArea->clear();
                m_storage->Clear();
                ui->devTabs->removeAll();
                ui->devTabs->setHeader(packet->header);
                ui->devTabs->addDevice();

                m_storage->setPacket(packet);
                m_packet = packet;
            }
            break;
        }
    }
    delete s;
}

void LorrisAnalyzer::timeSliderMoved(int value)
{
    bool down = ui->timeSlider->isSliderDown();
    bool enabled = ui->timeSlider->isEnabled();
    if(value != 0)
        updateData(down || !enabled);

    if(down && enabled)
        ui->timeBox->setValue(value);
}

void LorrisAnalyzer::timeBoxChanged(int value)
{
    ui->timeSlider->setValue(value);
}

void LorrisAnalyzer::updateData(bool ignoreTime)
{
    if(!canUpdateUi(ignoreTime))
        return;

    updateTime.restart();

    int val = ui->timeSlider->value();

    if(val != 0 && (quint32)val <= m_storage->getSize())
        emit newData(m_storage->get(val-1), val-1);
}

analyzer_data *LorrisAnalyzer::getLastData(quint32 &idx)
{
    int val = ui->timeSlider->value();
    if(!val)
        return NULL;

    idx = --val;
    return m_storage->get(val);
}

void LorrisAnalyzer::load(QString *name, quint8 mask)
{
    quint32 idx = 0;
    analyzer_packet *packet = m_storage->loadFromFile(name, mask, ui->dataArea, ui->devTabs, idx);
    if(!packet)
        return;

    // old packet deleted in AnalyzerDataStorage::loadFromFile()
    m_packet = packet;

    delete m_curData;
    m_curData = new analyzer_data(m_packet);

    if(!ui->devTabs->count())
    {
        ui->devTabs->removeAll();
        ui->devTabs->setHeader(packet->header);
        ui->devTabs->addDevice();
    }

    if(!idx)
        idx = m_storage->getSize();

    ui->timeSlider->setMaximum(m_storage->getSize());
    ui->timeSlider->setValue(idx);
    ui->timeBox->setMaximum(m_storage->getSize());
    ui->timeBox->setSuffix(tr(" of ") % QString::number(m_storage->getSize()));
    ui->timeBox->setValue(idx);
    m_state &= ~(STATE_DIALOG);

    updateData(true);
}

void LorrisAnalyzer::saveDataButton()
{
    m_storage->SaveToFile(ui->dataArea, ui->devTabs);
}

void LorrisAnalyzer::widgetMouseStatus(bool in, const data_widget_info &info, qint32 parent)
{
    if(parent != -1)
    {
        DataWidget *w = ui->dataArea->getWidget(parent);
        if(!w)
            return;

        if(in)
        {
            w->setStyleSheet("color: red");
            w->setLineWidth(2);
        }
        else
        {
            w->setLineWidth(1);
            w->setStyleSheet("");
        }
    }
    else
    {
        if(in)
        {
            if(highlightInfoNotNull && highlightInfo != info)
                ui->devTabs->setHighlightPos(highlightInfo, false);

            bool found = ui->devTabs->setHighlightPos(info, true);

            if(found)
            {
                highlightInfo = info;
                highlightInfoNotNull = true;
                return;
            }
        }

        if(highlightInfoNotNull)
        {
            ui->devTabs->setHighlightPos(highlightInfo, false);
            highlightInfoNotNull = false;
        }
    }
}

void LorrisAnalyzer::collapseTopButton()
{
    setAreaVisibility(AREA_TOP, !isAreaVisible(AREA_TOP));
}

void LorrisAnalyzer::collapseRightButton()
{
    setAreaVisibility(AREA_RIGHT, !isAreaVisible(AREA_RIGHT));
}

void LorrisAnalyzer::collapseLeftButton()
{
    setAreaVisibility(AREA_LEFT, !isAreaVisible(AREA_LEFT));
}

bool LorrisAnalyzer::isAreaVisible(quint8 area)
{
    switch(area)
    {
        case AREA_TOP:   return ui->devTabs->isVisible();
        case AREA_RIGHT: return ui->widgetsScrollArea->isVisible();
        case AREA_LEFT:  return ui->playFrame->isVisible();
    }
    return false;
}

void LorrisAnalyzer::setAreaVisibility(quint8 area, bool visible)
{
    if(area & AREA_TOP)
        ui->devTabs->setVisible(visible);

    if(area & AREA_RIGHT)
        ui->widgetsScrollArea->setVisible(visible);

    if(area & AREA_LEFT)
        ui->playFrame->setVisible(visible);
}

void LorrisAnalyzer::clearAllButton()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("Clear everything?"));
    box.setText(tr("Do you really want to clear data, widgets and packet structure?"));
    box.addButton(tr("Yes"), QMessageBox::YesRole);
    box.addButton(tr("No"), QMessageBox::NoRole);
    box.setIcon(QMessageBox::Question);

    if(box.exec())
        return;

    analyzer_packet *packet = m_packet;
    m_packet = NULL;

    ui->devTabs->removeAll();
    ui->devTabs->setHeader(NULL);
    ui->devTabs->addDevice();

    ui->dataArea->clear();

    delete m_curData;
    m_curData = NULL;
    m_storage->Clear();
    m_storage->setPacket(NULL);

    ui->timeSlider->setMaximum(0);
    ui->timeBox->setMaximum(0);
    ui->timeBox->setSuffix(tr(" of ") % "0");

    if(packet)
    {
        delete packet->header;
        delete packet;
    }

    setAreaVisibility(AREA_TOP | AREA_RIGHT, true);
    setAreaVisibility(AREA_LEFT, false);

    updateData(true);
}

void LorrisAnalyzer::clearDataButton()
{
    m_storage->Clear();

    ui->timeSlider->setMaximum(0);
    ui->timeBox->setMaximum(0);
    ui->timeBox->setSuffix(tr(" of ") % "0");

    updateData(true);
}

void LorrisAnalyzer::updateTimeChanged(int value)
{
    minUpdateDelay = value;
    sConfig.set(CFG_QUINT32_ANALYZER_UPDATE_TIME, minUpdateDelay);
}

void LorrisAnalyzer::openFile()
{
    static const QString filters = QObject::tr("Lorris data files (*.ldta *.cldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Load data file"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);

    if(filename.length() == 0)
        return;

    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename);

    load(&filename, (STORAGE_STRUCTURE | STORAGE_DATA | STORAGE_WIDGETS));
}

void LorrisAnalyzer::editStruture()
{
    SourceDialog *d = new SourceDialog(m_packet, this);
    m_state |= STATE_DIALOG;
    connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SLOT(readData(QByteArray)));

    analyzer_packet *packet = d->getStructure();
    delete d;

    if(packet)
    {
        delete m_curData;
        m_curData = new analyzer_data(packet);

        if(m_packet)
        {
            delete m_packet->header;
            delete m_packet;
        }
        ui->devTabs->setHeader(packet->header);

        if(!m_packet)
        {
            ui->devTabs->removeAll();
            ui->devTabs->addDevice();
        }

        m_storage->setPacket(packet);
        m_packet = packet;

        updateData(true);
    }
    m_state &= ~(STATE_DIALOG);
}

quint32 LorrisAnalyzer::getCurrentIndex()
{
    return ui->timeBox->value();
}
