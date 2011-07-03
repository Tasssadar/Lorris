#ifndef ANYMASTER_H
#define ANYMASTER_H

#include <QtCore/QObject>

class AbstractSerial;
class QTimer;
class AnyMaster : public QObject
{
    Q_OBJECT

public:
    AnyMaster(QObject *parent = 0);
    virtual ~AnyMaster();

    void start(bool enable);

private slots:
    void transaction();

private:
    AbstractSerial *port;
    QTimer *timer;
    int responseTimeout;//wait response timeout

    /* maximum and minimum length of the query
    (can be any length and is defined by a specific task.) */
    enum { MaxQueryLen = 50000, MinQueryLen = 500 };

    /* current length of the query
    the initial value of MinQueryLen byte, is set in the constructor of the class */
    int m_queryLen;

    QByteArray generateRequest();
};

#endif //ANYMASTER_H
