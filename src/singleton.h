/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

// (c) AbyssX Group
#ifndef SINGLETON_H
#define SINGLETON_H

//! Scott Bilas' Singleton implementation, with proper casting mod.
template <class T> class Singleton
{
    public:
        static T &GetSingleton(void)
        {
            static T singleton;
            return singleton;
        }
};

#endif
