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
#include <QToolBar>

#include "lorrisanalyzer.h"
#include "sourcedialog.h"
#include "ui_lorrisanalyzer.h"
#include "packet.h"
#include "storage.h"
#include "devicetabwidget.h"
#include "widgetarea.h"
#include "sourceselectdialog.h"
#include "packetparser.h"
#include "../ui/mainwindow.h"

#include "DataWidgets/datawidget.h"
#include "DataWidgets/numberwidget.h"
#include "DataWidgets/barwidget.h"
#include "DataWidgets/colorwidget.h"
#include "DataWidgets/GraphWidget/graphwidget.h"
#include "DataWidgets/ScriptWidget/scriptwidget.h"
#include "DataWidgets/terminalwidget.h"
#include "DataWidgets/buttonwidget.h"

LorrisAnalyzer::LorrisAnalyzer()
    : ui(new Ui::LorrisAnalyzer),
      m_connectButton(0)
{
    ui->setupUi(this);

    connect(ui->collapseTop,     SIGNAL(clicked()),         SLOT(collapseTopButton()));
    connect(ui->collapseRight,   SIGNAL(clicked()),         SLOT(collapseRightButton()));
    connect(ui->collapseLeft,    SIGNAL(clicked()),         SLOT(collapseLeftButton()));
    connect(ui->timeSlider,      SIGNAL(valueChanged(int)), SLOT(indexChanged(int)));
    connect(ui->timeBox,         SIGNAL(valueChanged(int)), SLOT(indexChanged(int)));
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

    QAction* newSource = menuData->addAction(QIcon(":/actions/new"), tr("New source..."));
    menuData->addSeparator();
    QAction* openAct = menuData->addAction(QIcon(":/actions/open"), tr("Open..."));
    QAction* saveAct = menuData->addAction(QIcon(":/actions/save"), tr("Save"));
    QAction* saveAsAct = menuData->addAction(QIcon(":/actions/save-as"), tr("Save as..."));
    menuData->addSeparator();
    QAction* clearAct = menuData->addAction(QIcon(":/actions/clear"), tr("Clear received data"));
    QAction* clearAllAct = menuData->addAction(tr("Clear everything"));

    openAct->setShortcut(QKeySequence("Ctrl+O"));
    saveAsAct->setShortcut(QKeySequence("Ctrl+Shift+S"));
    saveAct->setShortcut(QKeySequence("Ctrl+S"));

    QMenu *menuWidgets = new QMenu(tr("Widgets"), this);
    m_title_action = menuWidgets->addAction(tr("Show widget's title bar"));
    m_title_action->setCheckable(true);
    m_title_action->setChecked(true);

    addTopMenu(menuData);
    addTopMenu(menuWidgets);

    QAction *structAct = new QAction(QIcon(":/actions/system"), tr("Change packet structure"), this);

    QToolBar *bar = new QToolBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->topLayout->insertWidget(1, bar);

    bar->addAction(newSource);
    bar->addAction(openAct);
    bar->addAction(saveAct);
    bar->addAction(saveAsAct);
    bar->addSeparator();
    bar->addAction(structAct);
    bar->addSeparator();
    bar->addAction(clearAct);

    connect(newSource,      SIGNAL(triggered()),     SLOT(doNewSource()));
    connect(openAct,        SIGNAL(triggered()),     SLOT(openFile()));
    connect(saveAsAct,      SIGNAL(triggered()),     SLOT(saveAsButton()));
    connect(saveAct,        SIGNAL(triggered()),     SLOT(saveButton()));
    connect(clearAct,       SIGNAL(triggered()),     SLOT(clearDataButton()));
    connect(clearAllAct,    SIGNAL(triggered()),     SLOT(clearAllButton()));
    connect(m_title_action, SIGNAL(triggered(bool)), SLOT(showTitleTriggered(bool)));
    connect(structAct,      SIGNAL(triggered()),     SLOT(editStruture()));

    // Time box update consumes hilarious CPU time on X11,
    // this makes it better
#if defined(Q_WS_X11)
    ui->timeBox->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
    ui->timeBox->setAttribute(Qt::WA_PaintOnScreen, true);
#endif

    m_storage = new Storage(this);
    ui->dataArea->setAnalyzerAndStorage(this, m_storage);

    m_parser = new PacketParser(m_storage, this);
    connect(m_parser, SIGNAL(packetReceived(analyzer_data*,quint32)), SIGNAL(newData(analyzer_data*,quint32)));

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
    highlightInfoNotNull = false;
    m_curIndex = 0;

    setAreaVisibility(AREA_LEFT, false);
    setAreaVisibility(AREA_RIGHT, true);
    setAreaVisibility(AREA_TOP, true);

    m_data_changed = false;

    m_connectButton = new ConnectButton(ui->connectButton);
    connect(m_connectButton, SIGNAL(connectionChosen(PortConnection*)), this, SLOT(setConnection(PortConnection*)));
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    delete m_parser;
    delete m_storage;
    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    delete ui->devTabs;
    delete ui;
}

void LorrisAnalyzer::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    if(!result)
    {
        Utils::ThrowException(tr("Can't open connection!"));
    }
}

void LorrisAnalyzer::connectedStatus(bool)
{

}

void LorrisAnalyzer::readData(const QByteArray& data)
{
    bool update = m_curIndex == ui->timeSlider->maximum();
    if(!m_parser->newData(data, update))
        return;

    m_data_changed = true;
    int size = m_storage->getSize();

    ui->timeSlider->setMaximum(size);
    ui->timeBox->setMaximum(size);

    static const QString ofString = tr(" of ");
    ui->timeBox->setSuffix(ofString % QString::number(size));

    if(update)
    {
        m_curIndex = size;
        ui->timeBox->setValue(size);
        ui->timeSlider->setValue(size);
    }
}

void LorrisAnalyzer::onTabShow()
{
    if (!m_con)
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }

    if (m_con)
        this->doNewSource();
}

void LorrisAnalyzer::doNewSource()
{
    m_parser->setPaused(true);
    SourceSelectDialog *s = new SourceSelectDialog(this);

    if(!m_con)
        s->DisableNew();

    switch(s->get())
    {
        case -1:
            m_parser->setPaused(false);
            break;
        case 0:
        {
            QString file = s->getFileName();
            quint8 mask = s->getDataMask();
            load(file, mask);
            m_data_changed = false;
            break;
        }
        case 1:
        {
            SourceDialog *d = new SourceDialog(NULL, this);
            if (m_con)
                connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SIGNAL(readData(QByteArray)));

            analyzer_packet *packet = d->getStructure();
            delete d;

            m_parser->setPaused(false);
            if(!packet)
                break;

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
            m_parser->setPacket(packet);
            m_packet = packet;
            m_data_changed = true;
            break;
        }
    }
    delete s;
}

bool LorrisAnalyzer::onTabClose()
{
    if(!m_data_changed)
        return true;

    QMessageBox box(this);
    box.setText(tr("Data has been modified."));
    box.setInformativeText(tr("Do you want to save your changes?"));
    box.setIcon(QMessageBox::Question);
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);

    switch(box.exec())
    {
        case QMessageBox::Save:
            saveButton();
            return true;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            return false;
    }
    return true;
}

void LorrisAnalyzer::indexChanged(int value)
{
    if(value == m_curIndex)
        return;

    m_curIndex = value;
    updateData();
}

void LorrisAnalyzer::updateData()
{
    m_data_changed = true;

    ui->timeBox->setValue(m_curIndex);
    ui->timeSlider->setValue(m_curIndex);

    if(m_curIndex && (quint32)m_curIndex <= m_storage->getSize())
        emit newData(m_storage->get(m_curIndex-1), m_curIndex-1);
}

analyzer_data *LorrisAnalyzer::getLastData(quint32 &idx)
{
    if(!m_storage->getSize())
        return NULL;

    idx = m_curIndex-1;
    return m_storage->get(m_curIndex-1);
}

bool LorrisAnalyzer::load(QString &name, quint8 mask)
{
    quint32 idx = 0;
    analyzer_packet *packet = m_storage->loadFromFile(&name, mask, ui->dataArea, ui->devTabs, idx);
    if(!packet)
        return false;

    // old packet deleted in Storage::loadFromFile()
    m_packet = packet;
    m_parser->setPacket(packet);

    if(!ui->devTabs->count())
    {
        ui->devTabs->removeAll();
        ui->devTabs->setHeader(packet->header);
        ui->devTabs->addDevice();
    }

    if(!idx)
        idx = m_storage->getSize();

    m_curIndex = idx;
    ui->timeSlider->setMaximum(m_storage->getSize());
    ui->timeSlider->setValue(idx);
    ui->timeBox->setMaximum(m_storage->getSize());
    ui->timeBox->setSuffix(tr(" of ") % QString::number(m_storage->getSize()));
    ui->timeBox->setValue(idx);
    m_parser->setPaused(false);

    updateData();
    m_data_changed = false;
    return true;
}

void LorrisAnalyzer::saveButton()
{
    m_storage->SaveToFile(ui->dataArea, ui->devTabs);

    if(m_storage->getFilename().isEmpty())
        return;

    QStringList name = m_storage->getFilename().split(QRegExp("[\\/]"), QString::SkipEmptyParts);
    emit statusBarMsg(tr("File \"%1\" was saved").arg(name.last()), 5000);
    m_data_changed = false;
}

void LorrisAnalyzer::saveAsButton()
{
    m_storage->SaveToFile("", ui->dataArea, ui->devTabs);
    if(m_storage->getFilename().isEmpty())
        return;

    QStringList name = m_storage->getFilename().split(QRegExp("[\\/]"), QString::SkipEmptyParts);
    emit statusBarMsg(tr("File \"%1\" was saved").arg(name.last()), 5000);
    m_data_changed = false;
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

    m_data_changed = true;
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

    m_parser->setPacket(NULL);
    m_storage->Clear();
    m_storage->setPacket(NULL);

    m_curIndex = 0;
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

    updateData();
}

void LorrisAnalyzer::clearDataButton()
{
    m_storage->Clear();

    m_curIndex = 0;
    ui->timeSlider->setMaximum(0);
    ui->timeBox->setMaximum(0);
    ui->timeBox->setSuffix(tr(" of ") % "0");

    updateData();
}

void LorrisAnalyzer::openFile(const QString& filename)
{
    if(load((QString&)filename, (STORAGE_STRUCTURE | STORAGE_DATA | STORAGE_WIDGETS)))
        sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename);
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

    load(filename, (STORAGE_STRUCTURE | STORAGE_DATA | STORAGE_WIDGETS));
}

void LorrisAnalyzer::editStruture()
{
    SourceDialog *d = new SourceDialog(m_packet, this);
    m_parser->setPaused(true);
    if (m_con)
        connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SIGNAL(readData(QByteArray)));

    analyzer_packet *packet = d->getStructure();
    delete d;

    if(packet)
    {
        m_parser->setPacket(packet);

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

        updateData();
    }
    m_parser->setPaused(false);
}

quint32 LorrisAnalyzer::getCurrentIndex()
{
    return m_curIndex;
}

void LorrisAnalyzer::showTitleTriggered(bool checked)
{
    m_title_action->setChecked(checked);

    emit setTitleVisibility(checked);
}

void LorrisAnalyzer::setConnection(PortConnection *con)
{
    this->PortConnWorkTab::setConnection(con);
    m_connectButton->setConn(con);

    if(con)
        connect(this, SIGNAL(SendData(QByteArray)), con, SLOT(SendData(QByteArray)));
}
