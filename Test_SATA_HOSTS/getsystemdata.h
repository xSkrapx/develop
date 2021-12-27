#ifndef GETSYSTEMDATA_H
#define GETSYSTEMDATA_H

#include <QString>
#include <QDateTime>
#include <QSysInfo>
#include <sysinfoapi.h>
#include <windows.h>

struct sysInfo{
    QString PCName;
    QDateTime dataTime;
    bool result;
    QString OS;
    QString motherBoard;
    QString controller;
};

class getSystemData
{
public:
    getSystemData();
    sysInfo getSysInfo();

private:
    QString getNamePC();
};

#endif // GETSYSTEMDATA_H
