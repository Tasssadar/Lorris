/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SINGLETON_H
#define SINGLETON_H

template <class T>
class Singleton
{
public:
    static T &GetSingleton()
    {
        static T singleton;
        return singleton;
    }
};

#endif
