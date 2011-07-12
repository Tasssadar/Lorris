#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>

#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load("Lorris." + QLocale::system().name());
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
