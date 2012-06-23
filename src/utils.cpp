/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>

#include "utils.h"
#include "../dep/ecwin7/ecwin7.h"

QStatusBar *Utils::m_status_bar = NULL;
EcWin7 *Utils::m_win7 = NULL;

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

void Utils::ThrowException(const QString& text, QWidget* parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setTextFormat(Qt::RichText);
    box.setText(text);
    box.exec();
}

void Utils::setStatusBar(QStatusBar *bar)
{
    m_status_bar = bar;
}

void Utils::printToStatusBar(const QString& msg, int timeout)
{
    if(m_status_bar)
        m_status_bar->showMessage(msg, timeout);
}

void Utils::setWin7(EcWin7 *win7)
{
    m_win7 = win7;
}

void Utils::setProgress(int val)
{
    if(!m_win7)
        return;

    if(val == -1 || val == 100)
        m_win7->setProgressState(EcWin7::NoProgress);
    else
    {
        m_win7->setProgressState(EcWin7::Normal);
        m_win7->setProgressValue(val, 100);
    }
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
