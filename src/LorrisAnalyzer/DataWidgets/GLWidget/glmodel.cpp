/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "glmodel.h"
#include "glutils.h"

using namespace GLUtils;

GLModel::GLModel(const QString &name)
{
    m_name = name;
    m_smooth_shading = false;
}

void GLModel::addVertex(double *coords)
{
    m_vertices.push_back(vector4(coords[0], coords[1], coords[2], coords[3]));
}

void GLModel::addNormal(double *coords)
{
    m_normals.push_back(vector3(coords[0], coords[1], coords[2]));
}

void GLModel::addFace(const polygonFace &face)
{
    m_faces.push_back(face);
}

void GLModel::dump(bool mini)
{
    qDebug("\nModel %s", m_name.toStdString().c_str());
    //m_material.dump();
    qDebug("Vertex count: %lu", m_vertices.size());
    for(quint32 i = 0; !mini && i < m_vertices.size(); ++i)
    {
        const vector4& v = m_vertices[i];
        qDebug("  %u: %f %f %f %f", i, v.x, v.y, v.z, v.w);
    }

    qDebug("Faces count: %lu", m_faces.size());
    qDebug("Normals count: %lu", m_normals.size());

    for(quint32 i = 0; !mini && i < m_normals.size(); ++i)
    {
        const vector3& v = m_normals[i];
        qDebug("  %u: %f %f %f", i, v.x, v.y, v.z);
    }
}

void GLModel::createNormals()
{
    if(!m_normals.empty())
        return;

    if(!m_smooth_shading)
    {
        m_normals.resize(m_vertices.size(), vector3());
        std::vector<int> seen(m_vertices.size(), 0);

        for(quint32 i = 0; i < m_faces.size(); ++i)
        {
            polygonFace& f = m_faces[i];

            int v[3] = { f.v[0], f.v[1], f.v[2] };
            vector3 normal = GLUtils::normalize(GLUtils::cross(
                                vector3(m_vertices[v[1]]) - vector3(m_vertices[v[0]]),
                                vector3(m_vertices[v[2]]) - vector3(m_vertices[v[0]])));

            for(int j = 0; j < 3; ++j)
            {
                int cur_v = v[j];
                f.vn[j] = cur_v;

                if(++seen[cur_v] == 1)
                    m_normals[cur_v] = normal;
                else
                {
                    vector3& vec = m_normals[cur_v];
                    vec.x = vec.x * (1.0 - 1.0/seen[cur_v]) + normal.x * 1.0/seen[cur_v];
                    vec.y = vec.y * (1.0 - 1.0/seen[cur_v]) + normal.y * 1.0/seen[cur_v];
                    vec.z = vec.z * (1.0 - 1.0/seen[cur_v]) + normal.z * 1.0/seen[cur_v];
                    vec = GLUtils::normalize(vec);
                }
            }
        }
    }
    else
    {
        for(quint32 i = 0; i < m_faces.size(); ++i)
        {
            polygonFace& f = m_faces[i];

            int v[3] = { f.v[0], f.v[1], f.v[2] };
            vector3 normal = GLUtils::normalize(GLUtils::cross(
                                vector3(m_vertices[v[1]]) - vector3(m_vertices[v[0]]),
                                vector3(m_vertices[v[2]]) - vector3(m_vertices[v[0]])));
            m_normals.push_back(normal);
            std::fill(f.vn.begin(), f.vn.end(), m_normals.size()-1);
        }
    }
}

void GLModel::draw()
{
    bool isTransparent = false;
    /*
    if (hasTexture()) {
        glBindTexture (GL_TEXTURE_2D, m_pTexture->getID());
        isTransparent = m_pTexture->isTransparent();
    }*/

    bool isDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (isTransparent)
        glDisable(GL_DEPTH_TEST);

    int shading = 0;
    glGetIntegerv(GL_SHADE_MODEL, &shading);
    glShadeModel(m_smooth_shading ? GL_SMOOTH : GL_FLAT);

    std::vector<std::pair<quint32, material> >::iterator m_itr = m_materials.begin();
    for(quint32 i = 0; i < m_faces.size(); ++i)
    {
        polygonFace& f = m_faces[i];

        if(m_itr != m_materials.end() && i == (*m_itr).first)
        {
            const material& mtl = (*m_itr).second;
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mtl.Ka);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mtl.Kd);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mtl.Ks);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mtl.Ns);
            ++m_itr;
        }

        glColor3f(1, 1, 1);
        glBegin(GL_POLYGON);

        for (quint32 y = 0; y < f.v.size(); ++y)
        {
            glNormal3fv(m_normals[f.vn[y]].d());

            //if (hasTexture() && hasTextureCoords())
              //  glTexCoord3fv(m_texVector[(*itFace).vt[y]]);

            glVertex4fv(m_vertices[f.v[y]].d());
        }
        glEnd();	//	GL_POLYGON
    }
    //	restore depth test
    if (isDepthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glShadeModel(shading);
}

void GLModel::addMaterial(const material &mtl)
{
    m_materials.push_back(std::make_pair((quint32)m_faces.size(), mtl));
}
