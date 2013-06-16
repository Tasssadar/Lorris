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

float& vector4::operator [](int b)
{
    switch(b)
    {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default: throw "Invalid vector index";
    }
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

vector3 vector3::operator /(const float& div) const
{
    vector3 res(*this);
    res.x /= div;
    res.y /= div;
    res.z /= div;
    return res;
}

float& vector3::operator [](int b)
{
    switch(b)
    {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw "Invalid vector index";
    }
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
    float sqr = sqrt(x.x * x.x + x.y * x.y + x.z * x.z);

    if(sqr != 0)
        return x / sqr;
    else
        return x;
}

vector3 GLUtils::normalOfPlane(const vector3 &x, const vector3 &y)
{
    vector3 res;
    res.x = x.y * y.z - x.z * y.y;
    res.y = x.z * y.x - x.x * y.z;
    res.z = x.x * y.y - x.y * y.x;
    return res;
}
