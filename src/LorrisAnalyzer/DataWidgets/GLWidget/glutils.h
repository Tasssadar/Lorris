/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GLUTILS_H
#define GLUTILS_H

namespace GLUtils
{
    struct vector4
    {
        vector4(float x = 0, float y = 0, float z = 0, float w = 0);
        const float *d() const { return &x;}

        vector4 operator +(const vector4& b) const;
        vector4 operator -(const vector4& b) const;
        vector4 operator *(const float& ratio) const;

        float x;
        float y;
        float z;
        float w;
    };

    struct vector3
    {
        vector3(float x = 0, float y = 0, float z = 0);
        vector3(const vector4& vec);

        const float *d() const { return &x;}

        vector3 operator +(const vector3& b) const;
        vector3 operator -(const vector3& b) const;
        vector3 operator *(const float& ratio) const;

        float x;
        float y;
        float z;
    };

    /// cross product of two vectors
    vector3 cross(const vector3& x, const vector3& y);

    /// normalize vector
    vector3 normalize(const vector3& x);
}

#endif // GLUTILS_H
