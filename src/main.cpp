#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>

#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    if(!translator.load("Lorris." + QLocale::system().name(), "translations"))
        translator.load("Lorris." + QLocale::system().name(), "/opt/Lorris");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
