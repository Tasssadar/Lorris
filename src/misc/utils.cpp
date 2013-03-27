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

QFont Utils::getMonospaceFont(quint8 size)
{
    return QFont("Courier New", size);
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

void Utils::swapEndian(char *val, quint8 size)
{
    for(qint8 i = size; i > 0; i -= 2, ++val)
        std::swap(*val, *(val + i - 1));
}

// FIXME: some better implementation?
#ifdef Q_OS_WIN
#include <windows.h>
void Utils::playErrorSound()
{
    PlaySound(TEXT("SystemHand"), NULL, (SND_ASYNC | SND_ALIAS | SND_NOWAIT));
}
#else
void Utils::playErrorSound()
{
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

    QRect s;
    for(int i = 0; i < params.size(); ++i)
    {
        int val = params[i].toInt();
        switch(i)
        {
            case 0:
                if(val != 0)
                {
                    w->setWindowState(Qt::WindowMaximized);
                    return;
                }
                break;
            case 1: s.setWidth(val); break;
            case 2: s.setHeight(val);break;
            case 3: s.setX(val); break;
            case 4: s.setY(val); break;
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
