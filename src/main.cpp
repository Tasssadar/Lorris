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

static bool checkArgs(int argc, char** argv, QStringList& openFiles)
{
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            printf("Lorris release %s, git revision %u\n", VERSION, REVISION);
            return false;
        }
        else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("Usage: %s [ARGUMENTS...] [*.cldta file]\n\n"
                "Lorris, GUI tool for robotics - https://github.com/Tasssadar/Lorris\n\n"
                "Command line argumens:\n"
                "           --dump-cldta=FILE    Dump contents of *.cldta file and exit\n"
                "           --move-data          Move config.ini and sessions to user's documents folder"
                "       -h, --help               Display this help and exit\n"
                "       -v, --version            Display version info and exit\n",
                argv[0]);
            return false;
        }
        else if(strstr(argv[i], "--dump-cldta="))
        {
            // This will not handle non-ASCII characters on Windows. /care
            char *p = strchr(argv[i], '=')+1;
            QString path = QString::fromLocal8Bit(p);
            DataFileBuilder::dumpFileInfo(path);
            return false;
        }
        else if(strcmp(argv[i], "--move-data") == 0)
        {
            Utils::moveDataFolder();
            return false;
        }
        else
        {
            QString filename(argv[i]);
            QStringList parts = filename.split(".", QString::SkipEmptyParts);

            if(parts.empty())
               continue;

            if(sWorkTabMgr.isFileHandled(parts.back()))
                openFiles << filename;
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
        { "Lorris.%1", (QStringList() << "translations" << "/usr/share/lorris") }
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
    QCoreApplication::setOrganizationDomain("github.com/Tasssadar");
    QCoreApplication::setApplicationName("Lorris");

    // Sort tab infos after they were added by static variables
    // Also adds handled filetypes, so must be before checkArgs
    sWorkTabMgr.SortTabInfos();

    QStringList openFiles;
    if(!checkArgs(argc, argv, openFiles))
        return 0;

    QtSingleApplication a(argc, argv);
    if(sConfig.get(CFG_BOOL_ONE_INSTANCE) && a.isRunning() && a.sendMessage("newWindow|" + openFiles.join(";")))
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

    sWorkTabMgr.initialize(openFiles);
    return a.exec();
}
