/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GLMODEL_H
#define GLMODEL_H

#include <vector>
#include <QVector4D>
#include <QVector3D>
#include <QString>
#include <QGLFunctions>
#include <utility>

namespace GLUtils {
    struct vector3;
    struct vector4;
}

struct material
{
    material()
    {
        Ka[0] = Ka[1] = Ka[2] = 0.2f;
        Kd[0] = Kd[1] = Kd[2] = 0.8f;
        Ks[0] = Ks[1] = Ks[2] = 1.0f;

        Ka[3] = Kd[3] = Ks[3] = 1.0f; // alpha

        d = 1.0f;
        Ns = 0.0f;
        illum = 1;
    }

    material(const material& m)
    {
        memcpy(Ka, m.Ka, sizeof(Ka));
        memcpy(Kd, m.Kd, sizeof(Kd));
        memcpy(Ks, m.Ks, sizeof(Ks));
        d = m.d;
        Ns = m.Ns;
        illum = m.illum;
    }

    void dump()
    {
        qDebug("Ka: %f %f %f %f", Ka[0], Ka[1], Ka[2], Ka[3]);
        qDebug("Kd: %f %f %f %f", Kd[0], Kd[1], Kd[2], Kd[3]);
        qDebug("Ks: %f %f %f %f", Ks[0], Ks[1], Ks[2], Ks[3]);
        qDebug("d: %f", d);
        qDebug("Ns: %f", Ns);
        qDebug("illum: %d", illum);
    }

    //QString name;

    float Ka[4]; // ambient color
    float Kd[4]; // diffuse color
    float Ks[4]; // specular color
    float d; // alpha (Tr == d)
    float Ns; // shininess
    int illum; // illumination
    //char *map_Ka;
};

struct polygonFace
{
    void add(int v1, int vt1 = 0, int vn1 = 0)
    {
        v.push_back(--v1);
        vt.push_back(--vt1);
        vn.push_back(--vn1);
    }

    std::vector<int> v;
    std::vector<int> vt;
    std::vector<int> vn;
};

class GLModel
{
public:
    GLModel(const QString& name);

    void addVertex(double *coords);
    void addNormal(double *coords);
    void addFace(const polygonFace& face);

    void createNormals();
    void dump(bool mini);
    void draw();

    void setSmoothShading(bool on)
    {
        m_smooth_shading = on;
    }

    void addMaterial(const material &mtl);

private:
    std::vector<GLUtils::vector4> m_vertices;
    std::vector<GLUtils::vector3> m_normals;
    std::vector<polygonFace> m_faces;
    std::vector<std::pair<quint32, material> > m_materials;

    QString m_name;
    bool m_smooth_shading;
};

#endif // GLMODEL_H
