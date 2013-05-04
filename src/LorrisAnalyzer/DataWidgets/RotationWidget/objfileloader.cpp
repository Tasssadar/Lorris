/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDir>
#include <QGLFunctions>

#include "objfileloader.h"
#include "../../../misc/utils.h"

static const QString objEntryTypes[] =
{
    "v",          // 0 - vertex
    "f",          // 1 - face
    "vt",         // 2 - textures
    "vn",         // 3 - normals
    "s",          // 4 - smooth shading
    "o",          // 5 - model
    "mtllib",     // 6 - material lib
    "usemtl",     // 7 - use material
    "g",          // 8 - group
};

static const QString mtlEntryTypes[] =
{
    "newmtl",     // 0 - new material
    "Ka",         // 1 - ambient color
    "Kd",         // 2 - diffuse color
    "Ks",         // 3 - specular color
    "Ns",         // 4 - shininess
    "d",          // 5 - alpha
    "Tr",         // 6 - alpha (d == Tr)
    "illum"       // 7 - illumination
};

bool ObjFileLoader::load(const QString& file, std::vector<GLModel*>& modelList)
{
    QFile f(file);
    if(!f.open(QIODevice::ReadOnly))
        return false;

    QString path = "./";
    int idx = file.lastIndexOf('/');
    if(idx != -1)
    {
        path = file.left(idx+1);
    }

    QHash<QString, material> materials;
    QHash<QString, material>::iterator m_itr;
    QTextStream str(&f);

    GLModel *model = NULL;
    int vertexTotal = 0;
    int vertexModel = 0;

    int normalsTotal = 0;
    int normalsModel = 0;

    for(QString line = str.readLine(); !line.isNull(); line = str.readLine())
    {
        if(line.startsWith('#'))
            continue;

        QStringList parts = line.split(' ');
        if(parts.size() < 2)
            continue;

        int type = getObjType(parts[0]);
        if(type == -1)
            continue;

        switch(type)
        {
            case 5: // model
            {
                if(model)
                {
                    model->createNormals();
                    modelList.push_back(model);
                }
                model = new GLModel(parts.back());
                if(type == 5)
                {
                    vertexTotal += vertexModel;
                    vertexModel = 0;
                    normalsTotal += normalsModel;
                    normalsModel = 0;
                }
                continue;
            }
            case 8: // group
            {
                if(!model)
                    model = new GLModel(parts.back());
                continue;
            }
            case 6: // mtllib
                loadMaterials(path + parts.back(), materials);
                continue;
        }

        if(!model)
            continue;

        switch(type)
        {
            case 0: // vertex
            {
                double coords[4] = { 0.0, 0.0, 0.0, 1.f };
                for(int i = 1; i < parts.size() && i < 5; ++i)
                    coords[i-1] = parts[i].toDouble();
                model->addVertex(coords);
                ++vertexModel;
                break;
            }
            case 1: // face
            {
                polygonFace f;
                if(loadFace(parts, &f, vertexTotal, normalsTotal))
                    model->addFace(f);
                break;
            }
            case 2: // textures
                break;
            case 3: // normals
            {
                double coords[3] = { 0.0, 0.0, 0.0 };
                for(int i = 1; i < parts.size() && i < 4; ++i)
                    coords[i-1] = parts[i].toDouble();
                model->addNormal(coords);
                ++normalsModel;
                break;
            }
            case 4: // smooth shading
                model->setSmoothShading(parts.back() == "1");
                break;
            case 7: // usemtl
            {
                if(parts.back().isNull())
                    parts.back() = "";

                m_itr = materials.find(parts.back());
                if(m_itr == materials.end())
                    break;
                model->addMaterial(*m_itr);
                break;
            }
        }
    }
    if(model)
    {
        model->createNormals();
        modelList.push_back(model);
    }
    return true;
}

void ObjFileLoader::loadMaterials(const QString& file, QHash<QString, material>& materials)
{
    QFile f(file);
    if(!f.open(QIODevice::ReadOnly))
        return;

    QTextStream str(&f);
    material mtl;
    QString name;
    for(QString line = str.readLine(); !line.isNull(); line = str.readLine())
    {
        if(line.startsWith('#'))
            continue;

        QStringList parts = line.split(' ');
        if(parts.size() < 2)
            continue;

        int type = getMtlType(parts[0]);
        if(type == -1)
            continue;

        switch(type)
        {
            case 0: // newmtl
                if(!name.isNull())
                    materials.insert(name, mtl);

                mtl = material();
                name = parts.back();
                if(name.isNull())
                    name = "";
                break;
            case 1: // Ka
                for(int i = 1; i < 4 && i < parts.size(); ++i)
                    mtl.Ka[i-1] = parts[i].toFloat();
                break;
            case 2: // Kd
                for(int i = 1; i < 4 && i < parts.size(); ++i)
                    mtl.Kd[i-1] = parts[i].toFloat();
                break;
            case 3: // Ks
                for(int i = 1; i < 4 && i < parts.size(); ++i)
                    mtl.Ks[i-1] = parts[i].toFloat();
                break;
            case 4: // Ns
                // Wavefront has range 0 to 1000, opengl has 0 to 128
                mtl.Ns = parts.back().toFloat()*0.128;
                break;
            case 5: // d
            case 6: // Tr
                mtl.d = parts.back().toFloat();
                break;
            case 7: // illum
                mtl.illum = parts.back().toInt();
                break;
        }
    }

    if(!name.isNull())
        materials.insert(name, mtl);
}

int ObjFileLoader::getObjType(const QString &line)
{
    for(quint32 i = 0; i < sizeof_array(objEntryTypes); ++i)
    {
        if(objEntryTypes[i] == line)
            return i;
    }
    return -1;
}

int ObjFileLoader::getMtlType(const QString &line)
{
    for(quint32 i = 0; i < sizeof_array(mtlEntryTypes); ++i)
    {
        if(mtlEntryTypes[i] == line)
            return i;
    }
    return -1;
}

bool ObjFileLoader::loadFace(const QStringList &parts, polygonFace *f, int vertexTotal, int normalsTotal)
{
    if(parts.size() < 4)
        return false;

    static const QRegExp typeExp[] =
    {
        QRegExp("^[0-9]+$"),               // 0 - v1
        QRegExp("^[0-9]+/[0-9]+$"),        // 1 - v1/vt1
        QRegExp("^[0-9]+/[0-9]+/[0-9]+$"), // 2 - v1/vt1/vn1
        QRegExp("^[0-9]+//[0-9]+$"),       // 3 - v1//vn1
    };

    int type = -1;
    for(quint32 i = 0; type == -1 && i < sizeof_array(typeExp); ++i)
        if(typeExp[i].exactMatch(parts[1]))
            type = i;

    if(type == -1)
    {
        qWarning("ObjFileLoader::loadFace(): invalid face format %s", parts[1].toStdString().c_str());
        return false;
    }

    bool ok[3];
    for(int i = 1; i < parts.size(); ++i)
    {
        switch(type)
        {
            case 0:
            {
                int v = parts[i].toInt(ok);
                if(ok[0])
                    f->add(v - vertexTotal);
                break;
            }
            case 1:
            {
                QStringList split = parts[i].split('/', QString::SkipEmptyParts);
                if(split.size() != 2)
                    break;

                int v = split[0].toInt(ok);
                int vt = split[1].toInt(ok + 1);
                if(ok[0] && ok[1])
                    f->add(v - vertexTotal, vt);
                break;
            }
            case 2:
            {
                QStringList split = parts[i].split('/', QString::SkipEmptyParts);
                if(split.size() != 3)
                    break;

                int v = split[0].toInt(ok);
                int vt = split[1].toInt(ok + 1);
                int vn = split[2].toInt(ok + 2);

                if(ok[0] && ok[1] && ok[2])
                    f->add(v - vertexTotal, vt, vn);
                break;
            }
            case 3:
            {
                QStringList split = parts[i].split("//", QString::SkipEmptyParts);
                if(split.size() != 2)
                    break;

                int v = split[0].toInt(ok);
                int vn = split[1].toInt(ok + 1);
                if(ok[0] && ok[1])
                    f->add(v- vertexTotal, -1, vn - normalsTotal);
                break;
            }
        }
    }
    return (f->v.size() >= 3);
}
