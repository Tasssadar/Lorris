/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <stdio.h>

#include "updater.h"
#include "revision.h"
#include "ui/mainwindow.h"
#include "config.h"

#include "connection/connectionmgr2.h"
#include "WorkTab/WorkTabMgr.h"

static bool checkArgs(int argc, char** argv, QStringList& openFiles)
{
    if(argc < 2)
        return true;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            printf("Lorris release %s, git revision %u\n", VERSION, REVISION);
            return false;
        }
        else if(strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: %s [ARGUMENTS...]  Run Lorris\n\n"
                "Lorris, GUI tool for robotics - https://github.com/Tasssadar/Lorris\n\n"
                 "Command line argumens:\n"
                "       %s --help          Display this help and exit\n"
                "       %s --version       Display version info and exit\n"
                "       %s -v\n",
                argv[0], argv[0], argv[0], argv[0]);
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
    QLocale newLang = QLocale(langs[sConfig.get(CFG_QUINT32_LANGUAGE)]);

    QTranslator* qtTranslator = new QTranslator(&a);
    qtTranslator->load("qt_" + newLang.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(qtTranslator);

    QTranslator *translator = new QTranslator(&a);
    bool loaded = translator->load("Lorris." + newLang.name(), "translations");
    if(!loaded)
        loaded = translator->load("Lorris." + newLang.name(), "/usr/share/lorris");

    if(loaded)
        a.installTranslator(translator);
}

int main(int argc, char *argv[])
{
    //TODO: found an organization
    //QCoreApplication::setOrganizationName("-");
    QCoreApplication::setOrganizationDomain("github.com/Tasssadar");
    QCoreApplication::setApplicationName("Lorris");

    // Sort tab infos after they were added by static variables
    sWorkTabMgr.SortTabInfos();

    QStringList openFiles;
    if(!checkArgs(argc, argv, openFiles))
        return 0;

    QApplication a(argc, argv);
    psConMgr2 = new ConnectionManager2(&a);
    installTranslator(a);

    if(Updater::doUpdate(true))
        return 0;

    MainWindow w;
    w.show(openFiles);
    return a.exec();
}
