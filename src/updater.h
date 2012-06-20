/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class Updater : public QObject
{
    Q_OBJECT
public:

    static bool doUpdate();

private:
    static bool checkForUpdate();
    static bool askForUpdate();
};

#endif // UPDATER_H
