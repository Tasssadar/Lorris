#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <QObject>

class Connection : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(QByteArray data);

public:
    virtual ~Connection();
    explicit Connection();

    virtual QString GetIDString() { return QString(""); }
    virtual bool Open() { return false; }
    virtual void Close() { }

protected:
    bool opened;
};

#endif // CONNECTION_H
