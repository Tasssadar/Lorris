/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <stdio.h>
#include <qtsingleapplication/qtsingleapplication.h>
#include <QScopedPointer>
#include <QMetaType>
#include <QMenuBar>
#include <QProxyStyle>

#include "revision.h"
#include "ui/mainwindow.h"
#include "misc/config.h"
#include "connection/connectionmgr2.h"
#include "WorkTab/WorkTabMgr.h"
#include "ui/settingsdialog.h"
#include "misc/datafileparser.h"

// metatypes
#include "ui/colorbutton.h"
#include "LorrisAnalyzer/DataWidgets/GraphWidget/graphcurve.h"

#ifdef Q_OS_WIN
 #include "misc/updater.h"
#endif

static bool checkArgs(const QStringList& args, QStringList& openFiles, QString& session)
{
    for(int i = 1; i < args.size(); ++i)
    {
        if(args[i] == "--version" || args[i] == "-v")
        {
            utils_printf("Lorris release %s, git revision %u\n", VERSION, REVISION);
            return false;
        }
        else if(args[i] == "--help" || args[i] == "-h")
        {
            utils_printf("Usage: %s [ARGUMENTS...] [*.cldta file]\n\n"
                "Lorris, GUI tool for robotics - https://github.com/Tasssadar/Lorris\n\n"
                "Command line argumens:\n"
                "           --dump-cldta=FILE              Dump contents of *.cldta file and exit\n"
                "           --move-data                    Move config.ini and sessions to user's documents folder\n"
                "       -h, --help                         Display this help and exit\n"
                "       -s NAME|FILE, --session=NAME|FILE  Open session NAME or FILE\n"
                "       -v, --version                      Display version info and exit\n",
                args[0].toStdString().c_str());
            return false;
        }
        else if(args[i].startsWith("--dump-cldta="))
        {
            // This will not handle non-ASCII characters on Windows. /care
            int idx = args[i].indexOf('=');
            DataFileBuilder::dumpFileInfo(args[i].mid(idx+1));
            return false;
        }
        else if(args[i] == "--move-data")
        {
            Utils::moveDataFolder();
            return false;
        }
        else if(args[i] == "-s" && i+1 < args.size())
        {
            session = args[i+1];
            ++i;
        }
        else if(args[i].startsWith("--session="))
        {
            int idx = args[i].indexOf('=');
            session = args[i].mid(idx+1);
        }
        else
        {
            QStringList parts = args[i].split(".", QString::SkipEmptyParts);

            if(parts.empty())
               continue;

            if(sWorkTabMgr.isFileHandled(parts.back()))
                openFiles << args[i];
        }
    }
    return true;
}

static void installTranslator(QApplication& a)
{
    struct transl_entry { QString name; QStringList paths; };
    static const transl_entry trans[] =
    {
        { "qt_%1", (QStringList() << QLibraryInfo::location(QLibraryInfo::TranslationsPath) << "translations") },
        { "Lorris.%1", (QStringList() << "translations" << "/usr/share/lorris" << "/usr/local/share/lorris") }
    };

    QString lang = QLocale(langs[sConfig.get(CFG_QUINT32_LANGUAGE)]).name();
    QScopedPointer<QTranslator> translator;

    for(quint32 x = 0; x < sizeof_array(trans); ++x)
    {
        if(translator.isNull())
            translator.reset(new QTranslator(&a));

        const transl_entry& e = trans[x];

        QString name = e.name.arg(lang);
        if(x == 0)
            name = name.left(e.name.length());

        for(int y = 0; y < e.paths.size(); ++y)
        {
            if(!translator->load(name, e.paths[y]))
                continue;

            a.installTranslator(translator.take());
            break;
        }
    }
}

static void registerMetaTypes()
{
    qRegisterMetaType<ColorButton>("ColorButton");
    qRegisterMetaType<GraphCurve>("GraphCurve");
}

int main(int argc, char *argv[])
{
    //TODO: found an organization
    //QCoreApplication::setOrganizationName("-");
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setOrganizationDomain("github.com/Tasssadar");
    QCoreApplication::setApplicationName("Lorris");

    // Sort tab infos after they were added by static variables
    // Also adds handled filetypes, so must be before checkArgs
    sWorkTabMgr.SortTabInfos();

    QtSingleApplication a(argc, argv);

#ifdef Q_OS_MAC
    //Temporaly solution, does not set icon during app lunch!
    QApplication::setWindowIcon(QIcon(":/icons/icons/icon.icns"));
#endif

    QStringList openFiles;
    QString session;
    if(!checkArgs(a.arguments(), openFiles, session))
    {
        utils_flush();
        return 0;
    }

    if(a.isRunning() && sConfig.get(CFG_BOOL_ONE_INSTANCE) && a.sendMessage("newWindow|" + openFiles.join(";")))
    {
        qWarning("Running instance activated");
        return 0;
    }

    QObject::connect(&a, SIGNAL(messageReceived(QString)), &sWorkTabMgr, SLOT(instanceMessage(QString)));

    ConnectionManager2 conmgr(&a);
    installTranslator(a);

#ifdef Q_OS_WIN
    Updater::checkForUpdate(true);
#endif

    registerMetaTypes();

    sWorkTabMgr.initialize(openFiles, session);
    return a.exec();
}
