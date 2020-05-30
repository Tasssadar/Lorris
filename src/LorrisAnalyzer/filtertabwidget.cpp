/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScrollArea>
#include <QScrollBar>
#include <QCheckBox>
#include <QPropertyAnimation>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

#include <map>

#include "filtertabwidget.h"
#include "packet.h"
#include "DataWidgets/datawidget.h"
#include "../misc/datafileparser.h"
#include "labellayout.h"
#include "../misc/utils.h"
#include "../ui/editorwidget.h"

FilterTabWidget::FilterTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    m_filterIdCounter = 0;
    m_header = NULL;

    setTabPosition(QTabWidget::South);

    addEmptyFilter();

    QPushButton *btn = new QPushButton(tr("Filters"), this);
    btn->setIcon(QIcon(":/icons/system_dark"));
    setCornerWidget(btn, Qt::BottomLeftCorner);

    connect(btn, SIGNAL(clicked()), SLOT(showSettings()));
}

FilterTabWidget::~FilterTabWidget()
{
    delete_vect(m_filters);
}

void FilterTabWidget::Save(DataFileParser *file)
{
    file->writeBlockIdentifier(BLOCK_FILTERS);
    file->writeVal(m_filterIdCounter);
    file->writeVal((quint32)(m_filters.size()-1));

    for(quint32 i = 1; i < m_filters.size(); ++i)
        m_filters[i]->save(file);
}

void FilterTabWidget::Load(DataFileParser *file, bool skip)
{
    removeAll();

    if(file->seekToNextBlock(BLOCK_DEVICE_TABS, BLOCK_DATA))
    {
        loadLegacy(file);
        return;
    }
    else if(!file->seekToNextBlock(BLOCK_FILTERS, BLOCK_DATA))
        return;

    m_filterIdCounter = file->readVal<quint32>();

    quint32 count = file->readVal<quint32>();
    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock("dataFilter", BLOCK_DATA))
            break;

        quint8 type = file->readVal<quint8>();
        quint32 id = file->readVal<quint32>();
        QString name = file->readString();

        DataFilter *f = DataFilter::createFilter(type, id, name, this);
        if(!f)
            continue;

        f->load(file);
        addFilter(f);
    }

    if(skip)
        removeAll();
}

void FilterTabWidget::loadLegacy(DataFileParser *file)
{
    QHash<qint16, std::set<qint16> > data;
    qint16 id;

    quint32 count = file->readVal<quint32>();
    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_DEVICE_TAB, 0))
            break;

        id = file->readVal<qint16>();

        if(!file->seekToNextBlock(BLOCK_CMD_TABS, BLOCK_DEVICE_TAB))
            break;

        std::set<qint16> cmds;

        quint32 countCmd = file->readVal<quint32>();
        for(quint32 y = 0; y < countCmd; ++y)
        {
            if(!file->seekToNextBlock(BLOCK_CMD_TAB, BLOCK_CMD_TABS))
                break;
            cmds.insert(file->readVal<qint16>());
        }
        data[id] = cmds;
    }

    QString devname;
    QString cmdname;
    for(QHash<qint16, std::set<qint16> >::iterator itr = data.begin(); itr != data.end(); ++itr)
    {
        if(itr.key() != -1)
            devname = tr("Dev 0x%1").arg(itr.key(), 2, 16, QChar('0'));
        else
            devname.clear();

        for(std::set<qint16>::iterator sitr = (*itr).begin(); sitr != (*itr).end(); ++sitr)
        {
            if(itr.key() == -1 && *sitr == -1)
                continue;

            if(*sitr != -1)
                cmdname = tr("Cmd 0x%1").arg(*sitr, 2, 16, QChar('0'));
            else
                cmdname.clear();

            ConditionFilter *f = new ConditionFilter(generateId(), QString(), this);
            if(!devname.isEmpty() && !cmdname.isEmpty())
                f->setName(devname + " " + cmdname);
            else
                f->setName(devname + cmdname);

            if(itr.key() != -1)
                f->addCondition(new DevFilterCondition(itr.key()));
            if(*sitr != -1)
                f->addCondition(new CmdFilterCondition(*sitr));

            addFilter(f);
        }
    }
}

DataFilter *FilterTabWidget::getFilterByOldInfo(const data_widget_infoV1& old_info) const
{
    bool matchCmd;
    bool matchDev;
    for(quint32 i = 0; i < m_filters.size(); ++i)
    {
        DataFilter *f = m_filters[i];

        matchCmd = old_info.command == -1;
        matchDev = old_info.device == -1;

        if(f->getType() == FILTER_EMPTY)
        {
            if(matchCmd && matchDev)
                return f;
            continue;
        }

        const std::vector<FilterCondition*>& conds = ((ConditionFilter*)f)->getConditions();
        for(quint32 i = 0; (!matchCmd || !matchDev) && i < conds.size(); ++i)
        {
            switch(conds[i]->getType())
            {
                case COND_DEV:
                {
                    if(!matchDev && old_info.device == ((DevFilterCondition*)conds[i])->getDev())
                        matchDev = true;
                    break;
                }
                case COND_CMD:
                {
                    if(!matchCmd && old_info.command == ((CmdFilterCondition*)conds[i])->getCmd())
                        matchCmd = true;
                    break;
                }
                default:
                    break;
            }
        }
        if(matchCmd && matchDev)
            return f;
    }
    return NULL;
}

void FilterTabWidget::setHeader(analyzer_header *h)
{
    m_header = h;
    for(quint32 i = 0; i < m_filters.size(); ++i)
        m_filters[i]->setHeader(h);
}

void FilterTabWidget::removeAll()
{
    m_filterIdCounter = 0;
    delete_vect(m_filters);

    addEmptyFilter();
}

void FilterTabWidget::reset(analyzer_header *header)
{
    m_header = header;
    removeAll();
}

void FilterTabWidget::handleData(analyzer_data *data, quint32 index)
{
    for(quint32 i = 0; i < m_filters.size(); ++i)
        m_filters[i]->handleData(data, index);
}

void FilterTabWidget::showSettings()
{
    FilterDialog d(this);
    d.exec();
}

void FilterTabWidget::addFilter(DataFilter *f)
{
    QScrollArea *area = new QScrollArea(this);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ScrollDataLayout *layout = NULL;
    QWidget *w = new QWidget();

    QPalette p = area->palette();
    p.setColor(QPalette::Window, QColor("#F5F5F5"));
    area->setPalette(p);
    area->setAutoFillBackground(true);

    if(m_header)
    {
        layout = new ScrollDataLayout(m_header, false, true, this, w);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    area->setWidget(w);
    addTab(area, f->getName());

    f->setAreaAndLayout(area, layout);
    m_filters.push_back(f);

    if(f->getId() >= m_filterIdCounter) {
        m_filterIdCounter = f->getId() + 1;
    }

    connect(f, SIGNAL(activateTab()), SLOT(activateTab()));
}

void FilterTabWidget::removeFilter(DataFilter *f)
{
    for(quint32 i = 0; i < m_filters.size(); ++i)
    {
        if(m_filters[i] != f)
            continue;

        m_filters.erase(m_filters.begin()+i);
        delete f;
    }
}

void FilterTabWidget::addEmptyFilter()
{
    EmptyFilter *f = new EmptyFilter(generateId(), tr("All data"), this);
    addFilter(f);
}

void FilterTabWidget::setFilterName(DataFilter *f, const QString &name)
{
    for(quint32 i = 0; i < m_filters.size(); ++i)
    {
        if(f == m_filters[i])
        {
            f->setName(name);
            setTabText(i, name);
            return;
        }
    }
}

quint32 FilterTabWidget::getCurrFilterId() const
{
    return getCurrFilter()->getId();
}

DataFilter* FilterTabWidget::getCurrFilter() const
{
    if(currentIndex() >= (int)m_filters.size())
    {
        Q_ASSERT(false);
        return NULL;
    }

    return m_filters[currentIndex()];
}

DataFilter *FilterTabWidget::getFilter(quint32 id) const
{
    for(quint32 i = 0; i < m_filters.size(); ++i)
        if(id == m_filters[i]->getId())
            return m_filters[i];
    return NULL;
}

void FilterTabWidget::sendLastData()
{
    DataFilter *f;
    quint32 idx;

    analyzer_data data;
    Q_ASSERT(parent()->inherits("LorrisAnalyzer"));

    data.setPacket(analyzer()->getPacket());

    for(quint32 i = 0; i < m_filters.size(); ++i)
    {
        f = m_filters[i];
        idx = f->getLastIdx();

        data.setData(analyzer()->getDataAt(idx));
        if(data.hasData())
            f->handleData(&data, idx);
    }
}

void FilterTabWidget::clearLastData()
{
    for(quint32 i = 0; i < m_filters.size(); ++i)
        m_filters[i]->clearLastData();
}

void FilterTabWidget::activateTab()
{
    Q_ASSERT(sender() && sender()->inherits("DataFilter"));
    for(quint32 i = 0; i < m_filters.size(); ++i)
    {
        if(m_filters[i] == sender())
        {
            setCurrentIndex(i);
            return;
        }
    }
}

FilterDialog::FilterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::FilterDialog)
{
    ui->setupUi(this);

    m_scriptEditor = EditorWidget::getEditor(sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE), this);
    if(!m_scriptEditor)
        m_scriptEditor = EditorWidget::getEditor(EDITOR_INTERNAL, this);
    m_scriptEditor->setHighlighter(HIGHLIGHT_JSCRIPT);
    ui->condGrid->addWidget(m_scriptEditor->getWidget(), ui->condGrid->rowCount()-2, 0, 1, 2);

    m_jsonEditor = EditorWidget::getEditor(sConfig.get(CFG_QUINT32_SCRIPTEDITOR_TYPE), this);
    if(!m_jsonEditor)
        m_jsonEditor = EditorWidget::getEditor(EDITOR_INTERNAL, this);
    m_jsonEditor->setHighlighter(HIGHLIGHT_JSCRIPT);
    ui->jsonPageLayout->insertWidget(0, m_jsonEditor->getWidget(), 1);

    setCondVisibility(COND_MAX);

    connect(ui->typeBox, SIGNAL(currentIndexChanged(int)), SLOT(setCondVisibility(int)));
    connect(m_scriptEditor,    SIGNAL(textChangedByUser()),      SLOT(scriptModified()));
    connect(m_jsonEditor,   SIGNAL(textChangedByUser()), SLOT(jsonModified()));

    loadFilters();
}

FilterDialog::~FilterDialog()
{
    delete m_scriptEditor;
    delete m_jsonEditor;
    delete ui;
}

void FilterDialog::loadFilters() {
    ui->filterList->clear();

    const std::vector<DataFilter*>& filters = tabWidget()->getFilters();
    for(quint32 i = 1; i < filters.size(); ++i)
    {
        DataFilter *f = filters[i];

        QListWidgetItem *it = new QListWidgetItem(ui->filterList);
        it->setText(f->getName());
        it->setData(Qt::UserRole, QVariant::fromValue((void*)f));
    }
    ui->filterList->setCurrentRow(0);
    ui->propsStack->setCurrentIndex(filters.size() > 1 ? 0 : 1);
}

void FilterDialog::setCondVisibility(int cond)
{
    ui->typeLabel->setVisible(cond != COND_MAX);
    ui->typeBox->setVisible(cond != COND_MAX);

    ui->devLabel->setVisible(cond == COND_DEV);
    ui->devEdit->setVisible(cond == COND_DEV);

    ui->cmdLabel->setVisible(cond == COND_CMD);
    ui->cmdEdit->setVisible(cond == COND_CMD);

    ui->bytePosLabel->setVisible(cond == COND_BYTE);
    ui->bytePosBox->setVisible(cond == COND_BYTE);
    ui->byteValLabel->setVisible(cond == COND_BYTE);
    ui->byteValEdit->setVisible(cond == COND_BYTE);

    ui->langLabel->setVisible(cond == COND_SCRIPT);
    ui->langBox->setVisible(cond == COND_SCRIPT);
    m_scriptEditor->getWidget()->setVisible(cond == COND_SCRIPT);
    ui->errorLabel->setVisible(cond == COND_SCRIPT);
    ui->applyBtn->setVisible(cond == COND_SCRIPT);
}

void FilterDialog::on_addBtn_clicked()
{
    ConditionFilter *f = new ConditionFilter(tabWidget()->generateId(), getNewFilterName(), parent());
    tabWidget()->addFilter(f);

    QListWidgetItem *it = new QListWidgetItem(ui->filterList);
    it->setText(f->getName());
    it->setData(Qt::UserRole, QVariant::fromValue((void*)f));

    ui->filterList->setCurrentRow(ui->filterList->count()-1);
}

void FilterDialog::on_filterList_currentItemChanged(QListWidgetItem *current, QListWidgetItem * /*prev*/)
{
    ui->propsStack->setCurrentIndex(current == NULL);
    ui->rmBtn->setEnabled(current != NULL);
    if(!current)
        return;

    ConditionFilter *f = (ConditionFilter*)current->data(Qt::UserRole).value<void*>();
    Q_ASSERT(f);

    ui->nameEdit->setText(f->getName());

    ui->condTree->clear();
    const std::vector<FilterCondition*>& conds = f->getConditions();
    for(quint32 i = 0; i < conds.size(); ++i)
    {
        FilterCondition *c = conds[i];

        QTreeWidgetItem *it = new QTreeWidgetItem(ui->condTree);
        it->setText(0, c->getDesc());
        it->setData(0, Qt::UserRole, QVariant::fromValue((void*)c));

        ui->condTree->setCurrentItem(it);
    }
}

void FilterDialog::on_condTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*prev*/)
{
    ui->rmCondBtn->setEnabled(current != NULL);
    if(!current)
    {
        setCondVisibility(COND_MAX);
        return;
    }

    FilterCondition *c = (FilterCondition*)current->data(0, Qt::UserRole).value<void*>();
    Q_ASSERT(c);

    ui->typeBox->setCurrentIndex(c->getType());
    setCondVisibility(c->getType());
    fillCondData(c);
}

void FilterDialog::on_addCondBtn_clicked()
{
    QListWidgetItem *it = ui->filterList->currentItem();
    if(!it)
        return;

    ConditionFilter *f = (ConditionFilter*)it->data(Qt::UserRole).value<void*>();
    Q_ASSERT(f);

    FilterCondition *c = FilterCondition::createCondition(ui->typeBox->currentIndex());
    if(!c)
        return;

    f->addCondition(c);

    QTreeWidgetItem *tit = new QTreeWidgetItem(ui->condTree);
    tit->setText(0, c->getDesc());
    tit->setData(0, Qt::UserRole, QVariant::fromValue((void*)c));

    ui->condTree->setCurrentItem(tit);
}

void FilterDialog::on_typeBox_currentIndexChanged(int idx)
{
    ConditionFilter *f = getCurrFilter();
    FilterCondition *c = getCurrCondition();

    if(!f || !c || c->getType() == idx)
        return;

    FilterCondition *newCond = FilterCondition::createCondition(idx);
    if(!newCond)
        return;

    std::vector<FilterCondition*>& conds = f->getConditions();
    for(quint32 i = 0; i < conds.size(); ++i)
    {
        if(conds[i] == c)
        {
            conds[i] = newCond;
            break;
        }
    }
    delete c;

    QTreeWidgetItem *it = ui->condTree->currentItem();
    it->setText(0, newCond->getDesc());
    it->setData(0, Qt::UserRole, QVariant::fromValue((void*)newCond));
    on_condTree_currentItemChanged(it, NULL);
}

void FilterDialog::on_devEdit_textEdited(const QString &text)
{
    bool ok = false;
    int res = 0;
    if((((res = text.toInt(&ok)) || true) && ok) ||
       (((res = text.toInt(&ok, 16)) || true) && ok))
    {
        FilterCondition *c = getCurrCondition();
        if(!c || c->getType() != COND_DEV)
            return;
        ((DevFilterCondition*)c)->setDev(res);
        ui->condTree->currentItem()->setText(0, c->getDesc());
    }
}

void FilterDialog::on_cmdEdit_textEdited(const QString &text)
{
    bool ok = false;
    int res = 0;
    if((((res = text.toInt(&ok)) || true) && ok) ||
       (((res = text.toInt(&ok, 16)) || true) && ok))
    {
        FilterCondition *c = getCurrCondition();
        if(!c || c->getType() != COND_CMD)
            return;
        ((CmdFilterCondition*)c)->setCmd(res);
        ui->condTree->currentItem()->setText(0, c->getDesc());
    }
}

void FilterDialog::on_byteValEdit_textEdited(const QString &text)
{
    bool ok = false;
    int res = 0;
    if((((res = text.toInt(&ok)) || true) && ok) ||
       (((res = text.toInt(&ok, 16)) || true) && ok))
    {
        FilterCondition *c = getCurrCondition();
        if(!c || c->getType() != COND_BYTE)
            return;
        ((ByteFilterCondition*)c)->setByte(res);
        ui->condTree->currentItem()->setText(0, c->getDesc());
    }
}

void FilterDialog::on_bytePosBox_valueChanged(int val)
{
    FilterCondition *c = getCurrCondition();
    if(!c || c->getType() != COND_BYTE)
        return;

    ((ByteFilterCondition*)c)->setPos(val);
    ui->condTree->currentItem()->setText(0, c->getDesc());
}

void FilterDialog::on_nameEdit_textEdited(const QString &text)
{
    ConditionFilter *f = getCurrFilter();
    if(!f)
        return;

    tabWidget()->setFilterName(f, text);

    QListWidgetItem *it = ui->filterList->currentItem();
    it->setText(text);
}

void FilterDialog::on_rmBtn_clicked()
{
    ConditionFilter *f = getCurrFilter();
    if(!f)
        return;

    tabWidget()->removeFilter(f);

    QListWidgetItem *it = ui->filterList->currentItem();

    if(ui->filterList->count() > 1)
    {
        int row = ui->filterList->currentRow();
        ui->filterList->setCurrentRow(std::max(0, row-1));
    }

    delete it;
}

void FilterDialog::on_rmCondBtn_clicked()
{
    FilterCondition *c = getCurrCondition();
    ConditionFilter *f = getCurrFilter();
    if(!c || !f)
        return;

    f->removeCondition(c);

    delete ui->condTree->currentItem();
}

void FilterDialog::on_applyBtn_clicked()
{
    FilterCondition *c = getCurrCondition();
    if(!c || c->getType() != COND_SCRIPT)
        return;

    ui->errorLabel->clear();
    ui->errorLabel->setToolTip(QString());

    ScriptFilterCondition *sc = (ScriptFilterCondition*)c;
    sc->setScript(m_scriptEditor->getText());
    m_scriptEditor->setModified(false);
    ui->applyBtn->setEnabled(false);

    QString error = sc->getError();
    if(!error.isEmpty())
    {
        ui->errorLabel->setText(tr("Error! Mouseover to see details."));
        ui->errorLabel->setToolTip(error);
    }
}

void FilterDialog::on_jsonModeBtn_toggled(bool value) {
    ui->mainStack->setCurrentIndex(value ? 1 : 0);

    if(value) {
        if(ui->jsonErrorLabel->text().isEmpty()) {
            m_jsonEditor->setText(QString(toJson().toJson(QJsonDocument::Indented)));
            ui->jsonApplyBtn->setEnabled(false);
        }
        return;
    }

    const QString err = fromJson();
    if(!err.isNull()) {
        ui->jsonErrorLabel->setText(err);
        ui->jsonModeBtn->setChecked(true);
        return;
    }
    ui->jsonErrorLabel->setText("");
}

void FilterDialog::on_jsonApplyBtn_clicked() {
    const QString err = fromJson();
    ui->jsonErrorLabel->setText(!err.isNull() ? err : "");
    if(err.isNull())
        ui->jsonApplyBtn->setEnabled(false);
}

void FilterDialog::jsonModified() {
    ui->jsonApplyBtn->setEnabled(true);
}

QJsonDocument FilterDialog::toJson() const {
    QJsonArray filters;
    for(auto *f : tabWidget()->getFilters())
    {
        if(f->getType() != FILTER_CONDITION)
            continue;

        QJsonArray conditions;
        for(auto *c : ((ConditionFilter*)f)->getConditions()) {
            QJsonObject cond;
            cond["type"] = c->getType();
            switch(c->getType()) {
                case COND_DEV:
                    cond["dev"] = ((DevFilterCondition*)c)->getDev();
                    break;
                case COND_CMD:
                    cond["cmd"] = ((CmdFilterCondition*)c)->getCmd();
                    break;
                case COND_BYTE:
                {
                    ByteFilterCondition *d = (ByteFilterCondition*)c;
                    cond["idx"] = (int)d->getPos();
                    cond["value"] = d->getByte();
                    break;
                }
                case COND_SCRIPT:
                {
                    ScriptFilterCondition *d = (ScriptFilterCondition*)c;
                    cond["script"] = d->getScript();
                    cond["engine"] = d->getEngine();
                    break;
                }
            }
            conditions.append(cond);
        }

        QJsonObject filter;
        filter["name"] = f->getName();
        filter["conditions"] = conditions;
        filters.append(filter);
    }

    return QJsonDocument(filters);
}

QString FilterDialog::fromJson() {
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(m_jsonEditor->getText().toUtf8(), &err);
    if(doc.isNull()) {
        return err.errorString();
    }

    if(!doc.isArray()) {
        return tr("The root value is not an array.");
    }

    std::map<QString, std::unique_ptr<std::vector<ConditionFilter*>> > filterMap;
    for(auto *f : tabWidget()->getFilters()) {
        if(f->getType() != FILTER_CONDITION)
            continue;

        std::vector<ConditionFilter*> *vec = nullptr;
        auto itr = filterMap.find(f->getName());
        if(itr == filterMap.end()) {
            vec = new std::vector<ConditionFilter*>();
            filterMap.emplace(f->getName(), std::unique_ptr<std::vector<ConditionFilter*>>(vec));
        } else {
            vec = itr->second.get();
        }
        vec->push_back((ConditionFilter*)f);
    }

    std::vector<DataFilter*> newFilters;
    std::vector<std::unique_ptr<DataFilter>> newFilterDestroyer;
    const auto filters = doc.array();
    for(int i = 0; i < filters.size(); ++i) {
        if(!filters[i].isObject()) {
            return tr("Filter at position %1 is not an object.").arg(i);
        }

        QString err;
        bool isNewFilter = false;
        ConditionFilter *f = parseFilter(i, filters[i].toObject(), filterMap, err, isNewFilter);
        if(!err.isEmpty()) {
            return err;
        }

        newFilters.push_back(f);
        if(isNewFilter) {
            newFilterDestroyer.emplace_back(std::unique_ptr<DataFilter>(f));
        }
    }

    delete tabWidget()->getFilters()[0]; // EmptyFilter
    tabWidget()->getFilters().clear();
    tabWidget()->removeAll();
    for(auto *f : newFilters) {
        tabWidget()->addFilter(f);
    }
    loadFilters();

    for(auto& p : newFilterDestroyer) {
        p.release();
    }

    for(auto itr = filterMap.begin(); itr != filterMap.end(); ++itr) {
        for(ConditionFilter *oldf : *itr->second) {
            delete oldf;
        }
    }

    return QString();
}

ConditionFilter *FilterDialog::parseFilter(int pos, QJsonObject obj,
                                                           std::map<QString, std::unique_ptr<std::vector<ConditionFilter*>> >& filterMap,
                                                           QString &error, bool& isNewFilter) const {
    if(obj.isEmpty()) {
        error = tr("Filter at position %1 is not an object or is empty.").arg(pos);
        return nullptr;
    }

    const auto name = obj["name"];
    if(!name.isString()) {
        error = tr("Invalid type of property 'name' of filter %1, expected string").arg(pos);
        return nullptr;
    }

    const auto condsValue = obj["conditions"];
    if(!condsValue.isArray()) {
        error = tr("Invalid type of property 'conditions' of filter %1, expected array").arg(pos);
        return nullptr;
    }
    const auto conds = condsValue.toArray();

    std::vector<std::unique_ptr<FilterCondition>> newConditions;
    for(int i = 0; i < conds.size(); ++i) {
        const auto cond = conds[i].toObject();
        if(cond.isEmpty()) {
            error = tr("Invalid type of 'conditions' array member %1 of filter %2, expected object").arg(i).arg(pos);
            return nullptr;
        }

        std::unique_ptr<FilterCondition> filter_cond;
        switch(cond["type"].toInt(-1)) {
        case COND_DEV:
        {
            const auto dev = cond["dev"].toInt(-255);
            if(dev == -255) {
                error = tr("Invalid value of 'dev' property of a filter condition %1 of filter %2, expected int").arg(i).arg(pos);
                return nullptr;
            }
            filter_cond.reset(new DevFilterCondition(dev));
            break;
        }
        case COND_CMD:
        {
            const auto cmd = cond["cmd"].toInt(-255);
            if(cmd == -255) {
                error = tr("Invalid value of 'cmd' property of a filter condition %1 of filter %2, expected int").arg(i).arg(pos);
                return nullptr;
            }
            filter_cond.reset(new CmdFilterCondition(cmd));
            break;
        }
        case COND_BYTE:
        {
            const auto idx = cond["idx"].toInt(-1);
            if(idx == -1) {
                error = tr("Invalid value of 'idx' property of a filter condition %1 of filter %2, expected int").arg(i).arg(pos);
                return nullptr;
            }
            const auto byte = cond["value"].toInt(-255);
            if(idx == -255) {
                error = tr("Invalid value of 'byte' property of a filter condition %1 of filter %2, expected int").arg(i).arg(pos);
                return nullptr;
            }
            filter_cond.reset(new ByteFilterCondition(idx, byte));
            break;
        }
        case COND_SCRIPT:
        {
            const auto engine = cond["engine"].toInt(-1);
            if(engine != 0) {
                error = tr("Invalid value of 'engine' property of a filter condition %1 of filter %2, expected int == 0").arg(i).arg(pos);
                return nullptr;
            }
            const auto script = cond["script"].toString();
            if(script.isNull()) {
                error = tr("Invalid value of 'script' property of a filter condition %1 of filter %2, expected string").arg(i).arg(pos);
                return nullptr;
            }
            auto *sc = new ScriptFilterCondition(engine);
            sc->setScript(script);
            filter_cond.reset(sc);
            break;
        }
        default:
            error = tr("Invalid condition type: %1").arg(cond["type"].toDouble(-1));
            return nullptr;
        }

        newConditions.emplace_back(std::move(filter_cond));
    }

    ConditionFilter *res_filter = nullptr;
    const auto itr = filterMap.find(name.toString());
    if(itr != filterMap.end()) {
        auto *oldFilters = itr->second.get();
        res_filter = oldFilters->front();
        oldFilters->erase(oldFilters->begin());
        if(oldFilters->empty()) {
            filterMap.erase(itr);
        }
        isNewFilter = false;
    } else {
        res_filter = (ConditionFilter*)DataFilter::createFilter(FILTER_CONDITION, tabWidget()->generateId(), name.toString(), parent());
        isNewFilter = true;
    }

    for(FilterCondition *c : res_filter->getConditions()) {
        delete c;
    }
    res_filter->getConditions().clear();

    for(auto& p : newConditions) {
        res_filter->addCondition(p.release());
    }

    return res_filter;
}

void FilterDialog::scriptModified()
{
    ui->applyBtn->setEnabled(true);
}

FilterCondition *FilterDialog::getCurrCondition()
{
    QTreeWidgetItem *it = ui->condTree->currentItem();
    if(!it)
        return NULL;

    return (FilterCondition*)it->data(0, Qt::UserRole).value<void*>();
}

ConditionFilter *FilterDialog::getCurrFilter()
{
    QListWidgetItem *it = ui->filterList->currentItem();
    if(!it)
        return NULL;

    return (ConditionFilter*)it->data(Qt::UserRole).value<void*>();
}

void FilterDialog::fillCondData(FilterCondition *c)
{
    switch(c->getType())
    {
        case COND_DEV:
        {
            DevFilterCondition *d = (DevFilterCondition*)c;
            ui->devEdit->setText(QString("0x%1").arg(d->getDev(), 2, 16, QChar('0')));
            break;
        }
        case COND_CMD:
        {
            CmdFilterCondition *d = (CmdFilterCondition*)c;
            ui->cmdEdit->setText(QString("0x%1").arg(d->getCmd(), 2, 16, QChar('0')));
            break;
        }
        case COND_BYTE:
        {
            ByteFilterCondition *d = (ByteFilterCondition*)c;
            ui->byteValEdit->setText(QString("0x%1").arg(d->getByte(), 2, 16, QChar('0')));
            ui->bytePosBox->setValue(d->getPos());
            break;
        }
        case COND_SCRIPT:
        {
            ui->errorLabel->clear();
            ui->errorLabel->setToolTip(QString());

            ScriptFilterCondition *d = (ScriptFilterCondition*)c;
            m_scriptEditor->setText(d->getScript());
            m_scriptEditor->setModified(false);
            ui->applyBtn->setEnabled(false);
            ui->langBox->setCurrentIndex(d->getEngine());

            QString error = d->getError();
            if(!error.isEmpty())
            {
                ui->errorLabel->setText(tr("Error! Mouseover to see details."));
                ui->errorLabel->setToolTip(error);
            }
            break;
        }
    }
}

QString FilterDialog::getNewFilterName()
{
    static const QString base = tr("Filter%1");
    QString res = tr("Filter");
    int counter = 1;

    const std::vector<DataFilter*>& filters = tabWidget()->getFilters();
    for(quint32 i = 0; i < filters.size(); ++i)
    {
        if(res == filters[i]->getName())
        {
            res = base.arg(counter++);
            i = 0;
        }
    }
    return res;
}
