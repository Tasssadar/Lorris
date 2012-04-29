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

#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <stdio.h>

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

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + newLang.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator translator;
    bool loaded = translator.load("Lorris." + newLang.name(), "translations");
    if(!loaded)
        loaded = translator.load("Lorris." + newLang.name(), "/usr/share/lorris");

    if(loaded)
        a.installTranslator(&translator);
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

    MainWindow w;
    w.show(openFiles);
    return a.exec();
}
