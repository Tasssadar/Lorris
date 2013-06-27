/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
#include "packet.h"
#include "storage.h"
#include "widgetarea.h"
#include "sourceselectdialog.h"
#include "packetparser.h"
#include "../WorkTab/WorkTabMgr.h"
#include "DataWidgets/datawidget.h"
#include "widgetfactory.h"
#include "searchwidget.h"
#include "../ui/floatinginputdialog.h"

#include "ui_lorrisanalyzer.h"

static bool sortDataWidget(DataWidgetAddBtn *a, DataWidgetAddBtn *b)
{
    return QString::localeAwareCompare(a->text(), b->text()) < 0;
}

LorrisAnalyzer::LorrisAnalyzer()
    : ui(new Ui::LorrisAnalyzer),
     m_storage(this), m_parser(&m_storage, this), m_connectButton(0)
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
    connect(ui->limitBtn,        SIGNAL(clicked()),         SLOT(setPacketLimit()));
    connect(&m_parser,           SIGNAL(packetReceived(analyzer_data*,quint32)), SIGNAL(newData(analyzer_data*,quint32)));
    connect(ui->dataArea,        SIGNAL(mouseStatus(bool,data_widget_info,qint32)),
                                 SLOT(widgetMouseStatus(bool,data_widget_info, qint32)));
    connect(this,                SIGNAL(newData(analyzer_data*,quint32)), ui->filterTabs,
                                 SLOT(handleData(analyzer_data*, quint32)));


    int h = ui->collapseLeft->fontMetrics().height()+10;
    ui->collapseLeft->setFixedWidth(h);
    ui->collapseRight->setFixedWidth(h);
    ui->collapseTop->setFixedHeight(h);
    ui->collapseLeft->setRotation(ROTATE_270);

    ui->playBtn->setFixedWidth(h);
    ui->stopBtn->setFixedWidth(h);

    ui->playFrame->setOuterButtons(ui->playBtn, ui->stopBtn);

    QMenu* menuData = new QMenu(tr("&Data"), this);

    QAction* newSource = menuData->addAction(QIcon(":/actions/new"), tr("New source..."));
    menuData->addSeparator();
    QAction* openAct = menuData->addAction(QIcon(":/actions/open"), tr("Open..."));
    QAction* saveAct = menuData->addAction(QIcon(":/actions/save"), tr("Save"));
    QAction* saveAsAct = menuData->addAction(QIcon(":/actions/save-as"), tr("Save as..."));
    menuData->addSeparator();
    QAction* importAct = menuData->addAction(tr("Import binary data"));
    QAction* exportAct = menuData->addAction(tr("Export binary data"));
    menuData->addSeparator();
    QAction* clearAct = menuData->addAction(QIcon(":/actions/clear"), tr("Clear received data"));
    QAction* clearAllAct = menuData->addAction(tr("Clear everything"));

    openAct->setShortcut(QKeySequence("Ctrl+O"));
    saveAsAct->setShortcut(QKeySequence("Ctrl+Shift+S"));
    saveAct->setShortcut(QKeySequence("Ctrl+S"));

    addTopMenu(menuData);

    QAction *structAct = new QAction(QIcon(":/actions/system"), tr("Change structure"), this);

    exportAct->setStatusTip(tr("Export received bytes as binary file"));
    structAct->setStatusTip(tr("Change structure of incoming data"));

    QToolBar *bar = new QToolBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(16, 16));
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
    connect(clearAct,       SIGNAL(triggered()),     SLOT(clearData()));
    connect(clearAllAct,    SIGNAL(triggered()),     SLOT(clearAllButton()));
    connect(structAct,      SIGNAL(triggered()),     SLOT(editStructure()));
    connect(exportAct,      SIGNAL(triggered()),     SLOT(exportBin()));
    connect(importAct,      SIGNAL(triggered()),     SLOT(importBinAct()));

    ui->dataArea->setAnalyzerAndStorage(this, &m_storage);

    QWidget *tmp = new QWidget(this);
    QVBoxLayout *widgetBtnL = new QVBoxLayout(tmp);
    widgetBtnL->setContentsMargins(0, 0, style()->pixelMetric(QStyle::PM_ScrollBarExtent), 0);

    std::vector<DataWidgetAddBtn*> buttons = sWidgetFactory.getButtons(tmp);
    std::sort(buttons.begin(), buttons.end(), sortDataWidget);

    for(quint32 i = 0; i < buttons.size(); ++i)
    {
        widgetBtnL->addWidget(buttons[i]);
        connect(this, SIGNAL(tinyWidgetBtn(bool)), buttons[i], SLOT(setTiny(bool)));
    }

    widgetBtnL->addWidget(new QWidget(tmp), 4);
    ui->widgetsScrollArea->setWidget(tmp);
    tmp->setAutoFillBackground(false);

    m_packet = NULL;
    m_curIndex = 0;
    m_rightVisible = true;

    setAreaVisibility(AREA_LEFT, false);
    setAreaVisibility(AREA_RIGHT, true);
    setAreaVisibility(AREA_TOP, true);

    m_data_changed = false;

    m_connectButton = new ConnectButton(ui->connectButton);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));

    qApp->installEventFilter(this);
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    qApp->removeEventFilter(this);

    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    delete ui->filterTabs;
    delete ui->dataArea;
    delete ui;
}

void LorrisAnalyzer::connectedStatus(bool)
{

}

void LorrisAnalyzer::readData(const QByteArray& data)
{
    bool atMax = (m_curIndex == ui->timeSlider->maximum());
    bool update = atMax || (m_storage.getSize() >= m_storage.getPacketLimit());
    if(!m_parser.newData(data, update))
        return;

    m_data_changed = true;
    int size = m_storage.getMaxIdx();

    ui->timeSlider->setMaximum(size);
    ui->timeBox->setMaximum(size);

    static const QString ofString = tr(" of ");
    ui->timeBox->setSuffix(ofString % QString::number(size+1));

    if(atMax)
    {
        m_curIndex = size;
        ui->timeBox->setValue(m_curIndex);
        ui->timeSlider->setValue(m_curIndex);
    }
}

void LorrisAnalyzer::onTabShow(const QString& filename)
{
    if(!filename.isEmpty())
        openFile(filename);

    ui->dataArea->setFocus();

    if (!m_con && sConfig.get(CFG_BOOL_CONN_ON_NEW_TAB))
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }

    if(filename.isEmpty())
        this->doNewSource();
}

void LorrisAnalyzer::doNewSource()
{
    if(!askToSave())
        return;

    m_parser.setPaused(true);
    SourceSelectDialog s(this);

    if(!m_con)
        s.DisableNew();

    switch(s.get())
    {
        case -1:
            m_parser.setPaused(false);
            break;
        case 0:
        {
            analyzer_packet *packet = SourceDialog::getStructure(NULL, m_con.data());

            m_parser.setPaused(false);
            if(!packet)
                break;

            if(m_packet)
            {
                delete m_packet->header;
                delete m_packet;
            }

            resetDevAndStorage(packet);
            setPacket(packet);
            m_data_changed = true;
            break;
        }
        case 1:
        {
            QString file = s.getFileName();
            quint8 mask = s.getDataMask();
            load(file, mask);
            m_data_changed = false;
            break;
        }
        case 2:
        {
            importBinary(s.getFileName());
            break;
        }
    }
}

void LorrisAnalyzer::importBinary(const QString& filename, bool reset)
{
    analyzer_packet *packet = SourceDialog::getStructure(reset ? NULL : m_packet, NULL, filename);
    if(!packet)
    {
        m_parser.setPaused(false);
        return;
    }

    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }

    setPacket(packet);

    if(reset)
        resetDevAndStorage(packet);
    else
    {
        ui->filterTabs->setHeader(packet->header);
        m_parser.setPacket(packet);
        m_storage.setPacket(packet);
    }

    m_parser.setPaused(false);

    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly))
        return Utils::showErrorBox(tr("Could not open file %1 for reading!").arg(filename));

    QMessageBox box(QMessageBox::Information, tr("Importing..."), tr("Importing your data..."));
    box.setStandardButtons(QMessageBox::NoButton);
    box.setWindowModality(Qt::ApplicationModal);
    box.open();

    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);

    m_parser.newData(f.readAll(), false);
    f.close();

    quint32 max = m_storage.getMaxIdx();
    ui->timeSlider->setMaximum(max);
    ui->timeSlider->setValue(max);
    ui->timeBox->setMaximum(max);
    ui->timeBox->setSuffix(tr(" of ") % QString::number(m_storage.getSize()));
    ui->timeBox->setValue(max);
    updateData();
}

bool LorrisAnalyzer::onTabClose()
{
    return sWorkTabMgr.askChildrenToClose(getId()) && askToSave();
}

bool LorrisAnalyzer::askToSave()
{
    if(!m_data_changed)
        return true;

    emit activateMe();

    QMessageBox box(this);
    if(m_storage.getFilename().isEmpty())
        box.setText(tr("Data has been modified."));
    else
    {
        box.setText(tr("Data has been modified.\n\n%1").arg(m_storage.getFilename()));
        box.setToolTip(m_storage.getFilename());
    }

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

    if((quint32)m_curIndex < m_storage.getSize())
    {
        m_curData.setData(m_storage.get(m_curIndex));
        emit newData(&m_curData, m_curIndex);
    }
}

analyzer_data *LorrisAnalyzer::getLastData(quint32 &idx)
{
    if(!m_storage.getSize())
        return NULL;

    idx = m_curIndex;
    m_curData.setData(m_storage.get(m_curIndex));
    return &m_curData;
}

QByteArray *LorrisAnalyzer::getDataAt(quint32 idx)
{
    if(idx >= m_storage.getSize())
        return NULL;

    return m_storage.get(idx);
}

bool LorrisAnalyzer::load(QString &name, quint8 mask)
{
    quint32 idx = 0;
    analyzer_packet *packet = m_storage.loadFromFile(&name, mask, ui->dataArea, ui->filterTabs, idx);
    if(!packet)
        return false;

    // old packet deleted in Storage::loadFromFile()
    setPacket(packet);
    m_parser.setPacket(packet);

    if(!ui->filterTabs->count())
        ui->filterTabs->reset(packet->header);

    if(!idx)
        idx = m_storage.getMaxIdx();

    m_curIndex = idx;
    ui->timeSlider->setMaximum(m_storage.getMaxIdx());
    ui->timeSlider->setValue(idx);
    ui->timeBox->setMaximum(m_storage.getMaxIdx());
    ui->timeBox->setSuffix(tr(" of ") % QString::number(m_storage.getSize()));
    ui->timeBox->setValue(idx);
    m_parser.setPaused(false);

    updateData();
    ui->filterTabs->sendLastData();
    m_data_changed = false;
    return true;
}

void LorrisAnalyzer::saveButton()
{
    m_storage.SaveToFile(ui->dataArea, ui->filterTabs);

    if(m_storage.getFilename().isEmpty())
        return;

    QStringList name = m_storage.getFilename().split(QRegExp("[\\/]"), QString::SkipEmptyParts);
    emit statusBarMsg(tr("File \"%1\" was saved").arg(name.last()), 5000);
    m_data_changed = false;
}

void LorrisAnalyzer::saveAsButton()
{
    m_storage.SaveToFile("", ui->dataArea, ui->filterTabs);
    if(m_storage.getFilename().isEmpty())
        return;

    QStringList name = m_storage.getFilename().split(QRegExp("[\\/]"), QString::SkipEmptyParts);
    emit statusBarMsg(tr("File \"%1\" was saved").arg(name.last()), 5000);
    m_data_changed = false;
}

void LorrisAnalyzer::exportBin()
{
    static const QString filters = QObject::tr("Any file (*.*)");
    QString filename = QFileDialog::getSaveFileName(NULL, tr("Export binary data"),
                                                sConfig.get(CFG_STRING_ANALYZER_IMPORT),
                                                filters);
    if(filename.isEmpty())
        return;

    try {
        m_storage.ExportToBin(filename);
    } catch(const QString& ex) {
        return Utils::showErrorBox(ex);
    }

    QString name = filename.split(QRegExp("[\\/]"), QString::SkipEmptyParts).last();
    emit statusBarMsg(tr("Binary data were exported to file \"%1\"").arg(name), 5000);

    sConfig.set(CFG_STRING_ANALYZER_IMPORT, filename);
}

void LorrisAnalyzer::importBinAct()
{
    static const QString filters = QObject::tr("Any file (*.*)");
    QString filename = QFileDialog::getOpenFileName(NULL, tr("Import binary data"),
                                                sConfig.get(CFG_STRING_ANALYZER_IMPORT),
                                                filters);
    if(filename.isEmpty())
        return;

    sConfig.set(CFG_STRING_ANALYZER_IMPORT, filename);
    importBinary(filename, false);
}

void LorrisAnalyzer::widgetMouseStatus(bool in, const data_widget_info &, qint32 parent)
{
    if(parent != -1)
    {
        DataWidget *w = ui->dataArea->getWidget(parent);
        if(!w)
            return;
        w->setHighlighted(in);
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
        case AREA_TOP:   return ui->filterTabs->isVisible();
        case AREA_RIGHT: return m_rightVisible;
        case AREA_LEFT:  return ui->playFrame->isVisible();
    }
    return false;
}

void LorrisAnalyzer::setAreaVisibility(quint8 area, bool visible)
{
    if(area & AREA_TOP)
        ui->filterTabs->setVisible(visible);

    if(area & AREA_RIGHT)
    {
        m_rightVisible = visible;
        emit tinyWidgetBtn(!visible);
        if(visible)
        {
            ui->collapseRight->setIcon(QIcon(":/icons/arrow-right"));
            ui->widgetsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            ui->widgetsScrollArea->setMaximumWidth(0x00FFFFFF);
        }
        else
        {
            ui->collapseRight->setIcon(QIcon(":/icons/arrow-left"));
            ui->widgetsScrollArea->setMaximumWidth(ui->collapseRight->width());
            ui->widgetsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

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
    setPacket(NULL);

    resetDevAndStorage();
    m_storage.clearFilename();

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

void LorrisAnalyzer::clearData()
{
    m_parser.resetCurPacket();
    m_storage.Clear();

    m_curIndex = 0;
    ui->timeSlider->setMaximum(0);
    ui->timeBox->setMaximum(0);
    ui->timeBox->setSuffix(tr(" of ") % "0");

    ui->filterTabs->clearLastData();

    updateData();
}

void LorrisAnalyzer::resetDevAndStorage(analyzer_packet *packet)
{
    ui->filterTabs->reset(packet ? packet->header : NULL);

    ui->dataArea->clear();

    m_parser.setPacket(packet);
    m_storage.Clear();
    m_storage.setPacket(packet);
    m_storage.clearFilename();
}

void LorrisAnalyzer::openFile(const QString& filename)
{
    if(load((QString&)filename, (STORAGE_STRUCTURE | STORAGE_DATA | STORAGE_WIDGETS)))
        sConfig.set(CFG_STRING_ANALYZER_FOLDER, m_storage.getFilename());

    ui->dataArea->setFocus();
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

void LorrisAnalyzer::editStructure()
{
    m_parser.setPaused(true);
    analyzer_packet *packet = SourceDialog::getStructure(m_packet, m_con.data());

    if(packet)
    {
        m_parser.setPacket(packet);

        if(m_packet)
        {
            delete m_packet->header;
            delete m_packet;
        }
        ui->filterTabs->setHeader(packet->header);

        if(!m_packet)
            resetDevAndStorage(packet);

        m_storage.setPacket(packet);
        setPacket(packet);

        updateData();
    }
    m_parser.setPaused(false);
}

quint32 LorrisAnalyzer::getCurrentIndex()
{
    return m_curIndex;
}

void LorrisAnalyzer::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    this->PortConnWorkTab::setPortConnection(con);
    m_connectButton->setConn(con, false);

    if(con)
    {
        connect(this, SIGNAL(SendData(QByteArray)), con.data(), SLOT(SendData(QByteArray)));
        connect(con.data(), SIGNAL(dataRead(QByteArray)), SIGNAL(rawData(QByteArray)));
    }
}

void LorrisAnalyzer::updateForWidget()
{
    Q_ASSERT(sender());
    if(!sender())
        return;

    m_data_changed = true;

    if(m_curIndex && (quint32)m_curIndex < m_storage.getSize())
    {
        m_curData.setData(m_storage.get(m_curIndex));
        ((DataWidget*)sender())->newData(&m_curData, m_curIndex);
    }
}

QString LorrisAnalyzer::GetIdString()
{
    return "LorrisAnalyzer";
}

void LorrisAnalyzer::saveData(DataFileParser *file)
{
    PortConnWorkTab::saveData(file);

    QString filename = m_storage.getFilename();

    if(!filename.isEmpty())
    {
        file->writeBlockIdentifier("LorrAnalyzerFile");
        file->writeString(filename);
    }
    else
    {
        filename = file->getAttachmentFilename();
        if(!filename.isEmpty())
        {
            QString cfg_name = sConfig.get(CFG_STRING_ANALYZER_FOLDER);
            m_storage.SaveToFile(filename, ui->dataArea, ui->filterTabs);
            m_storage.clearFilename();
            sConfig.set(CFG_STRING_ANALYZER_FOLDER, cfg_name);

            file->writeBlockIdentifier("LorrAnalyzerTemp");
            file->writeString(filename);
        }
    }
}

void LorrisAnalyzer::loadData(DataFileParser *file)
{
    PortConnWorkTab::loadData(file);

    if(file->seekToNextBlock("LorrAnalyzerFile", BLOCK_WORKTAB))
        openFile(file->readString());
    else if(file->seekToNextBlock("LorrAnalyzerTemp", BLOCK_WORKTAB))
    {
        QString cfg_name = sConfig.get(CFG_STRING_ANALYZER_FOLDER);
        openFile(file->readString());
        sConfig.set(CFG_STRING_ANALYZER_FOLDER, cfg_name);

        m_storage.clearFilename();
    }
}

void LorrisAnalyzer::addChildTab(ChildTab *tab, const QString &name)
{
    sWorkTabMgr.addChildTab(tab, name, getId());
}

void LorrisAnalyzer::removeChildTab(ChildTab *tab)
{
    sWorkTabMgr.removeChildTab(tab);
}

void LorrisAnalyzer::setPacket(analyzer_packet *packet)
{
    m_packet = packet;
    m_curData.setPacket(packet);
}

DataFilter *LorrisAnalyzer::getFilter(quint32 id)
{
    return ui->filterTabs->getFilter(id);
}

DataFilter *LorrisAnalyzer::getFilterByOldInfo(const data_widget_infoV1 &old_info) const
{
    return ui->filterTabs->getFilterByOldInfo(old_info);
}

void LorrisAnalyzer::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Space)
        new SearchWidget(ui->dataArea, this);
    PortConnWorkTab::keyPressEvent(ev);
}

bool LorrisAnalyzer::eventFilter(QObject */*obj*/, QEvent *event)
{
    if(event->type() != QEvent::KeyPress)
        return false;

    if(((QKeyEvent*)event)->key() != Qt::Key_Space)
        return false;

    if(!isVisible() || !underMouse())
        return false;

    QWidget *focus = qApp->focusWidget();
    if(focus && (focus->inherits("QLineEdit") || focus->inherits("Terminal")))
        return false;

    new SearchWidget(ui->dataArea, this);
    return true;
}

void LorrisAnalyzer::setPacketLimit()
{
    int limit = FloatingInputDialog::getInt(tr("Set maximum number of packets"), m_storage.getPacketLimit(), 1);
    m_storage.setPacketLimit(limit);

    int size = m_storage.getMaxIdx();
    ui->timeSlider->setMaximum(size);
    ui->timeBox->setMaximum(size);
}
