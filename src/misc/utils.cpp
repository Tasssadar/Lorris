/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QLayout>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QDesktopWidget>

#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

#include "utils.h"
#include "../dep/ecwin7/ecwin7.h"

QString Utils::hexToString(quint8 data, bool withZeroEx)
{
    static const char* hex = "0123456789ABCDEF";

    QString result(withZeroEx ? "0x  " : "  ");
    result[int(withZeroEx)*2]     = hex[data >> 4];
    result[1 + int(withZeroEx)*2] = hex[data & 0x0F];
    return result;
}

QString Utils::parseChar(char c)
{
    switch(c)
    {
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\f': return "\\f";
        default:   return QString((QChar)c);
    }
}

QByteArray Utils::convertByteStr(QString str)
{
    QByteArray res;

    QStringList tok = str.split(' ', QString::SkipEmptyParts);
    if(tok.isEmpty())
        return res;

    bool ok = false;
    int num;
    for(int i = 0; i < tok.size(); ++i)
    {
        num = tok[i].toInt(&ok, 0);
        if(ok && (num & 0xFF) <= 255)
            res.append((char)num);
    }
    return res;
}

QString Utils::toBase16(quint8 const * first, quint8 const * last)
{
    QString res;
    for (; first != last; ++first)
    {
        static char const digits[] = "0123456789abcdef";
        res.append(digits[(*first >> 4)]);
        res.append(digits[(*first & 0xF)]);
    }
    return res;
}

void Utils::toBase16(char * ptr, uint8_t v)
{
    static char const digits[] = "0123456789abcdef";
    ptr[0] = digits[v >> 4];
    ptr[1] = digits[v & 0xf];
}

QString Utils::toBinary(std::size_t width, int value)
{
    QString res("0b");

    for(; width != 0; --width)
    {
        res[(int)(width+1)] = (QChar)((value % 2) ? '1' : '0');
        value >>= 1;
    }
    return res;
}

QFont Utils::getMonospaceFont(int size)
{
    if(size == -1)
        size = qApp->font().pointSize();

    static const char *families[] = {
        "Deja Vu Sans Mono",
        "Droid Sans Mono",
        "Andale Mono",
        "Consolas",
        "Courier New"
    };

    static int selected = -1;
    if(selected == -1)
    {
        for(quint32 i = 0; i < sizeof_array(families); ++i)
        {
            QFont f(families[i], size);
            if(f.exactMatch())
            {
                selected = i;
                return f;
            }
        }
        selected = sizeof_array(families)-1;
    }

    return QFont(families[selected], size);
}

void Utils::showErrorBox(const QString& text, QWidget* parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setTextFormat(Qt::RichText);
    box.setText(text);
    box.exec();
}

// FIXME: some better implementation?
#ifdef Q_OS_WIN
#include <windows.h>
void Utils::playErrorSound()
{
    if(sConfig.get(CFG_BOOL_ENABLE_SOUNDS))
        PlaySound(TEXT("SystemHand"), NULL, (SND_ASYNC | SND_ALIAS | SND_NOWAIT));
}
#else
void Utils::playErrorSound()
{
    if(sConfig.get(CFG_BOOL_ENABLE_SOUNDS))
        qApp->beep();
}
#endif

QString Utils::getFontSaveString(const QFont &font)
{
    QStringList vals;
    vals << font.family() << QString::number(font.pointSize())
         << QString::number((int)font.styleHint()) << QString::number(font.weight());
    return vals.join(";");
}

QFont Utils::getFontFromString(const QString &str)
{
    QStringList vals = str.split(';', QString::SkipEmptyParts);
    if(vals.size() != 4)
        return QFont();

    QFont fnt;

    for(quint8 i = 0; i < 4; ++i)
    {
        if(i == 0)
        {
            fnt.setFamily(vals[i]);
            continue;
        }

        bool ok = false;
        int val = vals[i].toInt(&ok);
        if(!ok)
            return QFont();

        switch(i)
        {
            case 1: // point size
                fnt.setPointSize(val);
                break;
            case 2: // style hint
                fnt.setStyleHint(QFont::StyleHint(val));
                break;
            case 3: // weight
                fnt.setWeight(val);
                break;
        }
    }
    return fnt;
}

QString Utils::saveWindowParams(QWidget *w)
{
    QStringList params;
    params << QString::number(w->isMaximized())
           << QString::number(w->width()) << QString::number(w->height())
           << QString::number(w->x()) << QString::number(w->y());
    return params.join(";");
}

void Utils::loadWindowParams(QWidget *w, const QString &param)
{
    QStringList params = param.split(';', QString::SkipEmptyParts);
    if(params.size() < 5)
        return;

    QDesktopWidget *d = qApp->desktop();
    QRect s;
    for(int i = 0; i < params.size(); ++i)
    {
        int val = params[i].toInt();
        switch(i)
        {
        case 0:
            if(val != 0) {
                w->setWindowState(Qt::WindowMaximized);
                return;
            }
            break;
        case 1:
            s.setWidth((std::min)(d->width(), val));
            break;
        case 2:
            s.setHeight((std::min)(d->height(), val));
            break;
        case 3:
            s.setX((std::max)(0, (std::min)(d->width() - s.width(), val)));
            break;
        case 4:
            s.setY((std::max)(0, (std::min)(d->height() - s.height(), val)));
            break;
        }
    }
    w->resize(s.size());
    w->move(s.topLeft());
}

void Utils::deleteLayoutMembers(QLayout *layout)
{
    while(layout->count())
    {
        QLayoutItem *item = layout->takeAt(0);
        if(item->layout())
        {
            Utils::deleteLayoutMembers(item->layout());
            delete item->layout();
            continue;
        }
        else if(item->widget())
        {
            delete item->widget();
            delete item;
        }
        else if(item->spacerItem())
            delete item->spacerItem();
        else
            delete item;
    }
}

bool Utils::isInRect(const QPoint& p, int rx, int ry, int rw, int rh)
{
    return p.x() >= rx && p.y() >= ry && p.x() <= rx+rw && p.y() <= ry+rh;
}

bool Utils::isInRect(const QPoint& p, const QPoint& rp, const QPoint& rs)
{
    return  p.x() >= rp.x() && p.y() >= rp.y() &&
            p.x() <= rp.x()+rs.x() && p.y() <= rp.y()+rs.y();
}

bool Utils::isInRect(int px, int py, int rx, int ry, int rw, int rh)
{
    return px >= rx && py >= ry && px <= rx+rw && py <= ry+rh;
}

size_t Utils::align(size_t & offset, size_t & size, size_t alignment)
{
    size_t aligned_offset = offset & ~(alignment - 1);
    size_t front_padding = offset - aligned_offset;
    size += front_padding;
    size = (size + alignment - 1) & ~(alignment - 1);
    offset = aligned_offset;
    return front_padding;
}

QString Utils::storageLocation(StandardLocation loc)
{
#if QT_VERSION < 0x050000
    static const QDesktopServices::StandardLocation locations[] = {
        QDesktopServices::DataLocation,
        QDesktopServices::DocumentsLocation
    };
    return QDesktopServices::storageLocation(locations[loc]);
#else
    static const QStandardPaths::StandardLocation locations[] = {
        QStandardPaths::DataLocation,
        QStandardPaths::DocumentsLocation
    };
    return QStandardPaths::writableLocation(locations[loc]);
#endif
}

void Utils::moveDataFolder()
{
    QString data = Utils::storageLocation(Utils::DataLocation) + "/";
    QString documents = Utils::storageLocation(Utils::DocumentsLocation) + "/Lorris/";

    if(!QFile::exists(data))
    {
        fprintf(stderr, "Folder %s does not exist!\n", data.toStdString().c_str());
        return;
    }

    if(QFile::exists(documents))
    {
        fprintf(stderr, "Folder %s exists, please move it!\n", documents.toStdString().c_str());
        return;
    }

    QDir dir(data);
    dir.mkpath(documents + "sessions");

    if(!QFile::copy(data + "config.ini", documents + "config.ini"))
    {
        fprintf(stderr, "Failed to copy config.ini!\n");
        return;
    }
    QFile::remove(data + "config.ini");

    documents.append("sessions/");
    QStringList files = dir.entryList(QDir::Files);
    for(int i = 0; i < files.size(); ++i)
    {
        if(!QFile::copy(data + files[i], documents + files[i]))
        {
            fprintf(stderr, "Failed to copy %s!\n", files[i].toStdString().c_str());
            return;
        }
        QFile::remove(data + files[i]);
    }

    if(dir.rmdir(data))
        utils_printf("Data successfuly moved to %s\n", documents.toStdString().c_str());
    else
        fprintf(stderr, "Failed to remove folder %s", data.toStdString().c_str());
}


void Utils::swapEndian(char *val, quint8 size)
{
    for(qint8 i = size; i > 0; i -= 2, ++val)
        std::swap(*val, *(val + i - 1));
}

#if defined(PROCESSOR_X86) && defined(Q_CC_GNU)
void Utils::swapEndian(uint32_t &val)
{
    __asm__
    (
        "bswap %%eax;"
        :"=a"(val)
        :"a"(val)
    );
}

void Utils::swapEndian(float& val)
{
    __asm__
    (
        "bswap %%eax;"
        :"=a"(val)
        :"a"(val)
    );
}

void Utils::swapEndian(uint16_t &val)
{
    __asm__
    (
        "xchg %%ah,%%al;"
        :"=a"(val)
        :"a"(val)
    );
}
#else
void Utils::swapEndian(uint32_t &val)
{
    val = ((val & 0x000000FF) << 24) |
          ((val & 0x0000FF00) << 8)  |
          ((val & 0x00FF0000) >> 8)  |
          ((val & 0xFF000000) >> 24);
}

void Utils::swapEndian(float& val)
{
    uint32_t *r = (uint32_t*)&val;
    *r = ((*r & 0x000000FF) << 24) |
         ((*r & 0x0000FF00) << 8)  |
         ((*r & 0x00FF0000) >> 8)  |
         ((*r & 0xFF000000) >> 24);
}

void Utils::swapEndian(uint16_t &val)
{
    val = ((val & 0x00FF) << 8) |
          ((val & 0xFF00) >> 8);
}
#endif
