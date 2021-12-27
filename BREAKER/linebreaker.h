#ifndef LINEBREAKER_H
#define LINEBREAKER_H
#include <QObject>

#include <QDebug>

class LineBreaker : public QObject
{
    Q_OBJECT
public:
    bool lineBreakerReconnect();
    void on();
    QString driveName = "";
private:
    bool sendToSerialPort(const QByteArray &data, QString portNumber, bool forMecanicalBreaker) const;
    QString findMecanicalBreaker();
    bool waitingForDiskConnectionDisconnection(bool connection);

    QString COMnumber;
};

#endif // LINEBREAKER_H
