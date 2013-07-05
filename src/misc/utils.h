/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef NUM_FUNC_H
#define NUM_FUNC_H

#include <QString>
#include <QThread>
#include <QFont>
#include <stdint.h>

// Workaround for GCC 4.6, it does not know override keyword
#if (__GNUC__ == 4 && __GNUC_MINOR__< 7)
  #define override
#endif

class QLayout;

#define sizeof_array(a) (sizeof(a)/sizeof(a[0]))
#define delete_vect(vec) \
    for(quint32 i = 0; i < vec.size(); ++i) \
        delete vec[i]; \
    vec.clear();

#ifdef Q_CC_MSVC
    #define PACK_STRUCT(structure) __pragma( pack(push, 1) ) structure __pragma( pack(pop) )
#elif defined(Q_CC_GNU)
    #define PACK_STRUCT(structure) structure __attribute__((gcc_struct,packed))
#else
    #error PACK_STRUCT is not defined for this compiler
#endif

#if defined(__i386) || defined(__i386__) || defined(_M_IX86) || \
    defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
  #define PROCESSOR_X86
#endif

class Utils : public QThread
{
    Q_OBJECT
public:
    enum StandardLocation
    {
        DataLocation,
        DocumentsLocation
    };

    static QString hexToString(quint8 data, bool withZeroEx = false);
    static QString parseChar(char c);

    static QByteArray convertByteStr(QString str);

    template <typename T> static void swapEndian(T& val);
    static void swapEndian(uint32_t& val);
    static void swapEndian(uint16_t& val);
    static void swapEndian(float& val);
    static void swapEndian(char *val, quint8 size);

    static QString toBase16(quint8 const * first, quint8 const * last);
    static QString toBinary(std::size_t width, int value);
    static void toBase16(char * ptr, uint8_t v);

    static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
    static void sleep (unsigned long secs)  { QThread::sleep(secs); }
    static void usleep(unsigned long usecs) { QThread::usleep(usecs); }

    static QFont getMonospaceFont(int size = -1);

    static void showErrorBox(const QString& text, QWidget* parent = 0);

    static void playErrorSound();

    static QString getFontSaveString(const QFont& font);
    static QFont getFontFromString(const QString& str);

    static QString saveWindowParams(QWidget *w);
    static void loadWindowParams(QWidget *w, const QString& param);

    static void deleteLayoutMembers(QLayout *layout);

    static bool isInRect(int px, int py, int rx, int ry, int rw, int rh);
    static bool isInRect(const QPoint& p, int rx, int ry, int rw, int rh);
    static bool isInRect(const QPoint& p, const QPoint& rp, const QPoint& rs);

    static size_t align(size_t & offset, size_t & size, size_t alignment);

    static QString storageLocation(StandardLocation loc);

    static void moveDataFolder();
};

template <typename T>
void Utils::swapEndian(T& val)
 {
    int8_t *p = (int8_t*)&val;
    for(size_t i = sizeof(val); i > 0; i -= 2, ++p)
        std::swap(*p, *(p + i - 1));
}

#ifdef Q_LITTLE_ENDIAN

template <typename T>
T deserialize_le(uint8_t const * p, size_t size = sizeof(T))
{
    T res = 0;
    memcpy(&res, p, size);
    return res;
}

template <typename T>
void serialize_le(uint8_t * p, T const & t, size_t size = sizeof(T))
{
    memcpy(p, &t, size);
}

#endif // Q_LITTLE_ENDIAN

#endif // NUM_FUNC_H
