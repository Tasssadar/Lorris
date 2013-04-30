/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef BYTEVALIDATOR_H
#define BYTEVALIDATOR_H

#include <QValidator>

class ByteValidator : public QValidator
{
    Q_OBJECT
public:
    explicit ByteValidator(QObject *parent = 0);

    State validate(QString &in, int &pos) const;
};

#endif // BYTEVALIDATOR_H
