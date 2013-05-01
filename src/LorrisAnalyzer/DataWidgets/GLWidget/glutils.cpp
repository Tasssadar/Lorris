/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <math.h>
#include "glutils.h"

using namespace GLUtils;

vector4::vector4(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

vector4 vector4::operator +(const vector4& b) const
{
    vector4 res(*this);
    res.x += b.x;
    res.y += b.y;
    res.z += b.z;
    res.w += b.w;
    return res;
}

vector4 vector4::operator -(const vector4& b) const
{
    vector4 res(*this);
    res.x -= b.x;
    res.y -= b.y;
    res.z -= b.z;
    res.w -= b.w;
    return res;
}

vector4 vector4::operator *(const float& ratio) const
{
    vector4 res(*this);
    res.x *= ratio;
    res.y *= ratio;
    res.z *= ratio;
    res.w *= ratio;
    return res;
}


vector3::vector3(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

vector3::vector3(const vector4& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
}

vector3 vector3::operator +(const vector3& b) const
{
    vector3 res(*this);
    res.x += b.x;
    res.y += b.y;
    res.z += b.z;
    return res;
}

vector3 vector3::operator -(const vector3& b) const
{
    vector3 res(*this);
    res.x -= b.x;
    res.y -= b.y;
    res.z -= b.z;
    return res;
}

vector3 vector3::operator *(const float& ratio) const
{
    vector3 res(*this);
    res.x *= ratio;
    res.y *= ratio;
    res.z *= ratio;
    return res;
}

/// cross product of two vectors
vector3 GLUtils::cross(const vector3& x, const vector3& y)
{
    return vector3(x.y * y.z - y.y * x.z,
                   x.z * y.x - y.z * x.x,
                   x.x * y.y - y.x * x.y);
}

/// normalize vector
vector3 GLUtils::normalize(const vector3& x)
{
    double sqr = sqrt(x.x * x.x + x.y * x.y + x.z * x.z);

    if(sqr > 0)
        return x * (1.0 / sqr);
    else
        return x;
}
