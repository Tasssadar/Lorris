/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef OBJFILELOADER_H
#define OBJFILELOADER_H

#include <QString>
#include <vector>
#include <QHash>

#include "glmodel.h"

class ObjFileLoader
{
public:
    static bool load(const QString& file, std::vector<GLModel*>& modelList);

private:
    static int getObjType(const QString& line);
    static int getMtlType(const QString& line);
    static void loadMaterials(const QString& file, QHash<QString, material>& materials);
    static bool loadFace(const QStringList& parts, polygonFace *f);
};

#endif // OBJFILELOADER_H
