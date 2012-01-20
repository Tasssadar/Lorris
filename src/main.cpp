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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("-");
    QCoreApplication::setOrganizationDomain("github.com/Tasssadar");
    QCoreApplication::setApplicationName("Lorris");

    QTranslator translator;
    if(!translator.load("Lorris." + QLocale::system().name(), "translations"))
        translator.load("Lorris." + QLocale::system().name(), "/usr/share/lorris");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
