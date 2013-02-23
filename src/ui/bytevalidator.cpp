/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QStringList>
#include "bytevalidator.h"

ByteValidator::ByteValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State ByteValidator::validate(QString &in, int &pos) const
{
    if(in.isEmpty())
        return Intermediate;

    if(pos > 0)
    {
        static const char *allowed = " 01234567890xXABCDEFabcdef-";
        if(!strchr(allowed, in[pos-1].toAscii()))
            return Invalid;
    }

    QStringList tok = in.split(' ', QString::SkipEmptyParts);
    if(tok.isEmpty())
        return Intermediate;

    bool ok = false;
    int num;
    for(int i = 0; i < tok.size(); ++i)
    {
        const QString& c = tok[i];

        num = c.toInt(&ok, 0);
        if(!ok)
            return Intermediate;

        if(num < 0)
            num += 128;

        if(num > 255 || num < 0)
            return Invalid;
    }
    return Acceptable;
}
