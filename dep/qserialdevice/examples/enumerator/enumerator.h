#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <QtCore/QObject>

class SerialDeviceEnumerator;
class Enumerator : public QObject
{
    Q_OBJECT

public:
    Enumerator(QObject *parent = 0);

private:
    SerialDeviceEnumerator *m_sde;

private slots:
    void slotPrintAllDevices(const QStringList &list);
};

#endif // ENUMERATOR_H
