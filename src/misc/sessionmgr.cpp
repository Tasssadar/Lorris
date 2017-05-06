/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFile>
#include <QMenu>
#include <QDir>
#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>

#include "sessionmgr.h"
#include "config.h"
#include "datafileparser.h"
#include "utils.h"
#include "../ui/tabview.h"
#include "../WorkTab/WorkTabMgr.h"

static const char CLDTA_DATA_MAGIC[] = { (char)0xFF, (char)0x80, 0x68 };

SessionMgr::SessionMgr(QObject *parent) : QObject(parent)
{
    m_sig_map = new QSignalMapper(this);
    connect(m_sig_map, SIGNAL(mapped(QString)), SLOT(loadSession(QString)), Qt::QueuedConnection);

    updateSessions();
}

QString SessionMgr::getFolder()
{
    if(sConfig.get(CFG_BOOL_PORTABLE))
    {
        if(QFile::exists("./data/sessions/"))
            return "./data/sessions/";
        else
            return "./data/";
    }
    else
    {
        static int idx = -1;
        static const QString locations[] = {
            Utils::storageLocation(Utils::DataLocation) + "/",
            Utils::storageLocation(Utils::DocumentsLocation) + "/Lorris/sessions/",
        };

        if(idx == -1)
        {
            if(QFile::exists(locations[0]))
                idx = 0;
            else
                idx = 1;
        }
        return locations[idx];
    }
}

QByteArray SessionMgr::openSession(const QString& name)
{
    return openSessionFilePath(getFolder() + name + ".cldta");
}

QByteArray SessionMgr::openSessionFilePath(const QString& path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return QByteArray();

    QByteArray data;
    QByteArray str = file.read(4);
    file.seek(0);

    if(str == QByteArray::fromRawData("LDTA", 4))
    {
        try {
            data = DataFileBuilder::readAndCheck(file, DATAFILE_SESSION);
        }
        catch(const QString& ex) {
            Utils::showErrorBox(tr("Error loading session file: %1").arg(ex));
            return data;
        }
    }
    // Legacy
    else
    {
        QByteArray magic = file.read(sizeof(CLDTA_DATA_MAGIC));
        if(magic.size() != sizeof(CLDTA_DATA_MAGIC))
            return QByteArray();

        for(quint8 i = 0; i < sizeof(CLDTA_DATA_MAGIC); ++i)
            if(magic[i] != CLDTA_DATA_MAGIC[i])
                return QByteArray();

        data = qUncompress(file.readAll());
    }

    return data;
}

void SessionMgr::initMenu(QMenu *menu)
{
    m_menus.insert(menu);

    if(m_sessions.empty())
    {
        QAction *empty = menu->addAction(tr("No saved sessions"));
        empty->setEnabled(false);
    }
    else
    {
        for(int i = 0; i < m_sessions.size(); ++i)
        {
            QAction *act = menu->addAction(m_sessions[i].remove(".cldta"));
            m_sig_map->setMapping(act, act->text());
            connect(act, SIGNAL(triggered()), m_sig_map, SLOT(map()));
        }
    }

    menu->addSeparator();

    QAction *saveAct = menu->addAction(tr("Save this session..."));
    QAction *managerAct = menu->addAction(tr("Session manager..."));

    connect(saveAct,    SIGNAL(triggered()), SLOT(saveSessionAct()));
    connect(managerAct, SIGNAL(triggered()), SLOT(openManager()));
    connect(menu, SIGNAL(destroyed(QObject*)), SLOT(removeMenu(QObject*)));
}

void SessionMgr::openManager()
{
    SessionDialog d(this);
    d.exec();
}

void SessionMgr::saveSession(QString name)
{
    QDir dir(getFolder());
    if(!dir.exists())
        dir.mkpath(dir.absolutePath());

    if(m_sessions.contains(name))
        removeSession(name);

    QByteArray data;
    DataFileParser parser(&data, QIODevice::WriteOnly, getFolder(), name);

    sWorkTabMgr.saveData(&parser);

    parser.close();

    DataFileBuilder::writeWithHeader(getFolder() + name + ".cldta", data, true, DATAFILE_SESSION);
}

void SessionMgr::saveSessionAct()
{
    QString name = SessionSaveDialog::getSessionName(this);
    if(name.isEmpty())
        return;

    name.replace(QRegExp("[/\\\\<>\\*:\"\\|\\?]"), "_");

    if(name.startsWith("..") || name.startsWith('_'))
        name.prepend("-");

    try {
        saveSession(name);
        sWorkTabMgr.printToAllStatusBars(tr("Session %1 saved.").arg(name));
    } catch(const QString& ex) {
        Utils::showErrorBox(ex);
    }

    updateSessions();
}

void SessionMgr::loadSession(QString name, bool closeOthers)
{
    if(name.isEmpty())
        name = tr("[Last session]");

    QByteArray data;
    if(name.contains("cldta"))
        data = openSessionFilePath(name);
    else
        data = openSession(name);

    if(data.isEmpty())
        return;

    DataFileParser parser(&data, QIODevice::ReadOnly, getFolder());
    sWorkTabMgr.loadData(&parser, closeOthers);
}

void SessionMgr::updateSessions()
{
    QDir dir(getFolder());
    m_sessions = dir.entryList((QStringList() << "[^_]*.cldta"), (QDir::Files | QDir::Readable), QDir::Name);

    for(std::set<QMenu*>::iterator itr = m_menus.begin(); itr != m_menus.end(); ++itr)
    {
        QMenu *menu = *itr;

        QAction *separator = NULL;
        QList<QAction*> actions = menu->actions();
        for(int i = 0; i < actions.size(); ++i)
        {
            if(actions[i]->isSeparator())
            {
                separator = actions[i];
                break;
            }
            delete actions[i];
        }

        if(m_sessions.empty())
        {
            QAction *empty = new QAction(tr("No saved sessions"), this);
            menu->insertAction(separator, empty);
            empty->setEnabled(false);
        }
        else
        {
            for(int i = 0; i < m_sessions.size(); ++i)
            {
                QAction *act = new QAction(m_sessions[i].remove(".cldta"), this);
                menu->insertAction(separator, act);
                m_sig_map->setMapping(act, act->text());
                connect(act, SIGNAL(triggered()), m_sig_map, SLOT(map()));
            }
        }
    }
}

void SessionMgr::removeSession(QString name)
{
    if(!name.contains("cldta"))
        name.append(".cldta");

    QString folder = getFolder();
    QFile::remove(folder + name);

    // Remove attachment files
    int at = 0;
    name.remove(".cldta");
    name = QString("%1_%2_at%3.cldta").arg(folder).arg(name);
    while(QFile::remove(name.arg(at)))
        ++at;

    updateSessions();
}

void SessionMgr::removeMenu(QObject *menu)
{
    m_menus.erase((QMenu*)menu);
}

SessionDialog::SessionDialog(SessionMgr *mgr, QWidget *parent) :
    QDialog(parent), ui(new Ui::SessionDialog)
{
    ui->setupUi(this);

    m_mgr = mgr;

    ui->sessionList->addItems(m_mgr->getSessions());
    ui->loadLastBox->setChecked(sConfig.get(CFG_BOOL_LOAD_LAST_SESSION));
    ui->autoConnBox->setChecked(sConfig.get(CFG_BOOL_SESSION_CONNECT));
}

SessionDialog::~SessionDialog()
{
    delete ui;
}

void SessionDialog::on_sessionList_itemSelectionChanged()
{
    ui->removeBtn->setEnabled(!ui->sessionList->selectedItems().isEmpty());
}

void SessionDialog::on_sessionList_doubleClicked(QModelIndex idx)
{
    m_mgr->loadSession(idx.data().toString());
    close();
}

void SessionDialog::on_addBtn_clicked()
{
    m_mgr->saveSessionAct();

    ui->sessionList->clear();
    ui->sessionList->addItems(m_mgr->getSessions());
}

void SessionDialog::on_removeBtn_clicked()
{
    if(ui->sessionList->selectedItems().isEmpty())
        return;

    QString name = ui->sessionList->selectedItems().front()->text();
    m_mgr->removeSession(name);

    ui->sessionList->clear();
    ui->sessionList->addItems(m_mgr->getSessions());
}

void SessionDialog::on_loadLastBox_clicked(bool checked)
{
    sConfig.set(CFG_BOOL_LOAD_LAST_SESSION, checked);
}

void SessionDialog::on_autoConnBox_clicked(bool checked)
{
    sConfig.set(CFG_BOOL_SESSION_CONNECT, checked);
}

SessionSaveDialog::SessionSaveDialog(SessionMgr *mgr, QWidget *parent) :
    QDialog(parent), ui(new Ui::SessionSaveDialog)
{
    ui->setupUi(this);

    ui->nameBox->addItems(mgr->getSessions());
    ui->nameBox->clearEditText();
}

SessionSaveDialog::~SessionSaveDialog()
{
    delete ui;
}

void SessionSaveDialog::on_nameBox_editTextChanged(const QString &text)
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!text.isEmpty());
}

QString SessionSaveDialog::getSessionName(SessionMgr *mgr)
{
    SessionSaveDialog d(mgr);
    if(d.exec() == QDialog::Accepted)
        return d.ui->nameBox->currentText();
    else
        return QString();
}
