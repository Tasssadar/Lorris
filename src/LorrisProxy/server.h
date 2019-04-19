/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QHash>
#include <QHostAddress>

class QAbstractSocket;

class Server : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void newData(const QByteArray& data);
    void newConnection(QString address, quint32 id);
    void removeConnection(quint32 id);

public:
    typedef QHash<quint32, QAbstractSocket*> socketMap;

    Server(QObject *parent = NULL);
    virtual ~Server();

    virtual bool listen(const QString& address, quint16 port) = 0;
    virtual void stopListening() = 0;
    virtual void closeConnection(quint32 id);
    virtual bool isListening() const = 0;
    virtual QString errorString() const = 0;

    QString getAddress();

public slots:
    virtual void SendData(const QByteArray& data) = 0;

protected:
    virtual QHostAddress serverAddress() const = 0;

    quint32 m_con_counter;
};

#endif // SERVER_H
