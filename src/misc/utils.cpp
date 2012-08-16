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

    QString result(withZeroEx ? "0x" : "");
    result += hex[data >> 4];
    result += hex[data & 0x0F];
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
        QLayoutItem *item = layout->itemAt(0);
        layout->removeItem(item);
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
    }
}
