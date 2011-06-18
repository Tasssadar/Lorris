#ifndef COMMON_H
#define COMMON_H
#include <QtCore/qglobal.h>

#if defined(LORRISPROBE_LIBRARY)
#  define LORRISPROBESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LORRISPROBESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // COMMON_H
