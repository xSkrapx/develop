#ifndef DISKPART_H
#define DISKPART_H

#include <QString>

enum formattingType{
    FULL = 0,
    QUICK = 1
};

class diskpart
{
public:
    diskpart();

    quint64 changeLogicalToPhysical(QString diskName, bool formatInternalDisk = false);
    bool cleanDisk(quint64 numberPhysical);
    bool createPart(quint64 numberPhysical, quint64 sizeOfPart);
    QVector<quint64> getVolumeFromDisk(quint64 numberPhysical);
    QString volumeNumberToVolName(quint64 volumeNumber, QString name);
    bool setVolName(quint64 volumeNumber, QString name);
    bool copyFiles(QString pathToPattern, QString targetDir);


    QString chkRAMconsuption(QString imageName);

    bool formatVolume(QString diskName, QString format, qint64 clusterSize, formattingType formType, bool formatInternalDisk);
    static bool chkdsk(QString driveName);
private:
    QString scriptDiskpart(QString script);
};

#endif // DISKPART_H
