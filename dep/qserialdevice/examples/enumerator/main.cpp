#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include "enumerator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Enumerator en;

    return a.exec();
}
