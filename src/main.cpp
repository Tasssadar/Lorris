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

#include "ui/mainwindow.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //TODO: found an organization
    //QCoreApplication::setOrganizationName("-");
    QCoreApplication::setOrganizationDomain("github.com/Tasssadar");
    QCoreApplication::setApplicationName("Lorris");

    QLocale newLang = QLocale(langs[sConfig.get(CFG_QUINT32_LANGUAGE)]);

    QTranslator translator;
    bool loaded = translator.load("Lorris." + newLang.name(), "translations");
    if(!loaded)
        loaded = translator.load("Lorris." + newLang.name(), "/usr/share/lorris");

    if(loaded)
        a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
