#include "diskpart.h"
#include <QProcess>
#include <QFile>
#include <QThread>
#include <iostream>
#include <QTextCodec>
#include <QDir>

diskpart::diskpart()
{

}

QString diskpart::scriptDiskpart(QString script){
    if(QFile::exists("script.txt")){
        QFile::remove("script.txt");
    }
    QFile file("script.txt");
    if(!file.open(QIODevice::ReadWrite)){
        return {};
    }
    file.write(script.toLatin1());
    file.close();

    QThread::sleep(1);
    QProcess process;
    process.setWorkingDirectory("C:/Windows/System32");
    QString PROGRAM_NAME = QString("diskpart.exe /s %1/script.txt").arg(QDir::currentPath());
    process.start(PROGRAM_NAME);

    if (!process.waitForStarted(-1))
    {
        std::cout << "Cannot start diskpart.exe!" << std::endl;
        return {};
    }

    if (!process.waitForFinished(-1))
    {
        std::cout << "Cannot stop diskpart.exe!"<< std::endl;
        return {};
    }

    auto data = process.readAllStandardOutput();
    data.append(process.readAllStandardError());
    QString decodeData(QTextCodec::codecForName("IBM866")->toUnicode(data));
    return  decodeData;
}

quint64 diskpart::changeLogicalToPhysical(QString diskName, bool formatInternalDisk)
{   
    QString data = scriptDiskpart(QString("select volume %1\r\ndetail disk").arg(diskName));
    auto dataOfDisk = data.split('\n');

    bool startData = false;
    for(auto &line: dataOfDisk){
        if(startData and line != ""){
            QString decodeLine = line;

            if(formatInternalDisk){
                if(decodeLine.contains("Системны") or decodeLine.contains("Загрузоч")){
                    std::cout << "format Boot Disk!" << std::endl;
                    return {};
                } else if(decodeLine.contains("DVD") or decodeLine.contains("CD")){
                    std::cout << "format cd/dvd" << std::endl;
                    return {};
                } else{
                    diskName = decodeLine.mid((decodeLine.lastIndexOf("Том") + 4), 1);
                    break;
                }
            } else{
                if(decodeLine.contains("Сменный")){
                    break;
                } else{
                    std::cout << "format internal disk" << std::endl;
                    return {};
                }
            }
        }
        if(line.contains("-----") and !startData){
            startData = true;
        }
    }

    QFile::remove("script.txt");

    QString selectVolume = QString("select volume %1").arg(diskName);
    QString detailVolume = QString("detail volume");
    QString parametrs = QString("%1\r\n%2").arg(selectVolume).arg(detailVolume);
    data = scriptDiskpart(parametrs);

    dataOfDisk.clear();
    dataOfDisk = data.split('\n');
    quint64 pysicalName = 999;
    startData = false;
    bool nextIsNumberOfDisk = false;
    for(auto &line: dataOfDisk){
        QString decodeLine = line;
        if(startData){
            auto decodeName = decodeLine.split(" ");
            for(auto &name :decodeName){
                if(nextIsNumberOfDisk){
                    pysicalName = name.at(0).digitValue();
                    nextIsNumberOfDisk = false;
                    return pysicalName;
                }
                if(name.contains("Диск")){
                    nextIsNumberOfDisk = true;
                }
            }
        }
        if (decodeLine.contains("-----")){
            startData = true;
        }
    }
    return pysicalName;
}

bool diskpart::cleanDisk(quint64 numberOfDDisk)
{
    for(quint8 i = 0; i < 5; ++i){
        QString detailDisk = QString("select disk %1\r\nclean").arg(QString::number(numberOfDDisk));
        QString data = scriptDiskpart(detailDisk);

        if(data.contains("очистка диска выполнена успешно")){
            std::cout << "clean disk: Access" << std::endl;
            return true;
        }
        else if(data.contains("Отказано в доступе")){
             std::cout << "clean disk: Access denied" << std::endl;
        }
    }
    return false;
}



bool diskpart::createPart(quint64 numberPhysical, quint64 sizeOfPart, bool allSize)
{
    QFile::remove("script.txt");
    QString PROGRAM_NAME = QString("diskpart.exe /s %1/script.txt").arg(QDir::currentPath());
    QString createPartitionPrimary;
    if(!(allSize) or !(sizeOfPart == 0)){
        createPartitionPrimary = QString("select disk %1\r\ncreate partition primary size=%2").arg(QString::number(numberPhysical)).arg(QString::number(sizeOfPart));
    } else{
        createPartitionPrimary = QString("select disk %1\r\ncreate partition primary").arg(QString::number(numberPhysical));
    }
    QString data = scriptDiskpart(createPartitionPrimary);
    if(!data.contains("указанный раздел успешно создан")){
        return false;
    }
    QThread::sleep(10);
    std::cout << "Part " << numberPhysical << " created. Size = " << sizeOfPart << std::endl;
    return true;
}

QVector<quint64> diskpart::getVolumeFromDisk(quint64 numberPhysical)
{
    QString createPartitionPrimary = QString("select disk %1\r\ndetail disk").arg(QString::number(numberPhysical));
    QString data = scriptDiskpart(createPartitionPrimary);

    QStringList decodeListData = data.split('\n');
    bool startData = false;
    QVector<quint64> volumes;
    for(auto &line: decodeListData){
        if(startData){
            if(line.contains("Том")){
                volumes.push_back(QString(line.mid((line.lastIndexOf("Том") + 4), 1)).at(0).digitValue());
                std::cout << "vol - " ;
                std::cout << QString(line.mid((line.lastIndexOf("Том") + 4), 1)).at(0).digitValue() << std::endl;
            }
        }
        if(line.contains("-----") and !startData){
            startData = true;
        }
    }
    return volumes;
}

bool diskpart::setVolName(quint64 volumeNumber, QString name){
    QString script = QString("select volume %1\r\nassign letter %2").arg(volumeNumber).arg(name);
    QString data = scriptDiskpart(script);
    if(data.contains("успешно")){
        std::cout << "vol: - " << volumeNumber << " is " << name.toStdString() << " ";
        return true;
    }
    return false;
}

QString diskpart::volumeNumberToVolName(quint64 volumeNumber, QString name)
{
    QString createPartitionPrimary = QString("select volume %1\r\ndetail disk").arg(QString::number(volumeNumber));
    QString data = scriptDiskpart(createPartitionPrimary);

    QStringList decodeListData = data.split('\n');
    bool startData = false;
    QVector<quint64> volumes;
    for(auto &line: decodeListData){
        if(startData){
            QString decodeName = line.simplified();
            QStringList listName = decodeName.split(' ');

            if(listName.at(3).size() > 1){
                setVolName(volumeNumber, name);
                return name;
            }

            return  listName.at(3);
        }
        if(line.contains("-----") and !startData){
            startData = true;
        }
    }
    return {};
}


bool diskpart::formatVolume(QString diskName, QString format, qint64 clusterSize, formattingType formType, bool formatInternalDisk)
{
    const QByteArray diskBusy = "\x82\xEB\xA1\xE0\xA0\xAD\xAD\xEB\xA9 \xE2\xAE\xAC \xA8\xAB\xA8 \xE0\xA0\xA7\xA4\xA5\xAB \xE3\xA6\xA5 \xA8\xE1\xAF\xAE\xAB\xEC\xA7\xE3\xA5\xE2\xE1\xEF";
    const QByteArray clusterTooSmall = "\xE1\xAB\xA8\xE8\xAA\xAE\xAC \xAC\xA0\xAB";
    const QByteArray clusterTooBig = "\xE1\xAB\xA8\xE8\xAA\xAE\xAC \xA2\xA5\xAB\xA8\xAA";
    const QByteArray success = "DiskPart \xE3\xE1\xAF\xA5\xE8\xAD\xAE \xAE\xE2\xE4\xAE\xE0\xAC\xA0\xE2\xA8\xE0\xAE\xA2\xA0\xAB\xA0 \xE2\xAE\xAC";

    //    QByteArray data = process.readAllStandardOutput();

    QFile::remove("script.txt");
    QFile file("script.txt");
    QString detailDisk;

    detailDisk = QString("select volume %1\r\ndetail disk").arg(diskName);
    if(file.open(QIODevice::ReadWrite)){
        file.write(detailDisk.toLatin1());
        file.close();
    }

    QThread::sleep(1);
    QProcess process;
    process.setWorkingDirectory("C:/Windows/System32");
    QString PROGRAM_NAME = QString("diskpart.exe /s %1/script.txt").arg(QDir::currentPath());
    process.start(PROGRAM_NAME);

    if (!process.waitForStarted(-1))
    {
        std::cout << "Cannot start diskpart.exe!" << std::endl;
        return false;
    }

    if (!process.waitForFinished(-1))
    {
        std::cout << "Cannot stop diskpart.exe!"<< std::endl;
        return false;
    }

    auto data = process.readAllStandardOutput();
    data.append(process.readAllStandardError());

    auto dataOfDisk = data.split('\n');

    bool startData = false;
    for(auto &line: dataOfDisk){
        if(startData and line != ""){
            QString decodeLine = QTextCodec::codecForName("IBM866")->toUnicode(line);

            if(formatInternalDisk){
                if(decodeLine.contains("Системны") or decodeLine.contains("Загрузоч")){
                    std::cout << "format Boot Disk!" << std::endl;
                    return false;
                } else if(decodeLine.contains("DVD") or decodeLine.contains("CD")){
                    std::cout << "format cd/dvd" << std::endl;
                    return false;
                } else{
                    diskName = decodeLine.mid((decodeLine.lastIndexOf("Том") + 4), 1);
                    break;
                }
            } else{
                if(decodeLine.contains("Сменный")){
                    break;
                } else{
                    std::cout << "format internal disk" << std::endl;
                    return false;
                }
            }
        }
        if(line.contains("-----") and !startData){
            startData = true;
        }
    }

    QFile::remove("script.txt");
    QString selectDisk = QString("select volume %1").arg(diskName);
    QString formatDisk;

    if(formType == formattingType::QUICK){
        formatDisk = QString("format quick fs=%1 unit=%2").arg(format).arg(clusterSize);
    }
    if(formType == formattingType::FULL){
        formatDisk = QString("format fs=%1 unit=%2").arg(format).arg(clusterSize);
    }

    QString parametrs = QString("%1\r\n%2").arg(selectDisk).arg(formatDisk);
    if(file.open(QIODevice::ReadWrite)){
        file.write(parametrs.toUtf8());
        file.close();
    }

    process.setWorkingDirectory("C:/Windows/System32");
    process.start(PROGRAM_NAME);
    if(!process.waitForStarted(-1) || !process.waitForFinished(-1)) {
        return false;
    }
    data = process.readAllStandardOutput();
    data.append(process.readAllStandardError());
    if (data.contains(clusterTooSmall)){
        std::cout << "Cluster too small" << std::endl;
        return false;
    } else if(data.contains(clusterTooBig)){
        std::cout << "Cluster too big" << std::endl;
        return false;
    } else if(data.contains(diskBusy)){
        std::cout << "Disk busy" << std::endl;
        return false;
    } else if(data.contains(success)){
        std::cout << "Format success: fs = " << format.toStdString() << " clst = " << clusterSize << std::endl;
        return true;
    }
    return false;
}

QString diskpart::chkRAMconsuption(QString imageName){
    QProcess process;
    QThread::sleep(1);
    process.start(QString("tasklist /FI \"IMAGENAME eq %1\"").arg(imageName));
    if(!process.waitForStarted(-1) || !process.waitForFinished(-1)) {
        return "tasklist error";
    }
    QByteArray data = process.readAllStandardOutput();
    data.append(process.readAllStandardError());
    QString rawData = QTextCodec::codecForName("IBM866")->toUnicode(data);
    if(rawData.contains("отсутствуют")){
        return QString("%1 no exist").arg(imageName);
    }

    return rawData;
}

bool diskpart::chkdsk(QString driveName){
    QProcess proc;
    QThread::sleep(1);
    proc.start(QString("chkdsk %1: /F").arg(driveName));
    if(!proc.waitForStarted()){
        std::cout << "error chkdsk waitForStarted" << std::endl;
        return false;
    }
    if (!proc.waitForFinished()) {
        std::cout << "error chkdsk waitForFinished" << std::endl;
        return false;
    }
    return true;
}

bool diskpart::copyFiles(QString pathToPattern, QString targetDir){
    auto currDir = QDir::currentPath();

    if (QDir check(pathToPattern); !check.exists())
        return false;

    if (QDir check(targetDir); !check.exists())
        return false;

    QString selectDisk = QString("Xcopy /S /I /E /H %1 %2 /Y").arg(pathToPattern.replace('/', "\\")).arg(targetDir.replace('/', "\\")); //TODO /H

    QFile fileScriptCopy("scriptCopy.bat");
    fileScriptCopy.remove();
    if(fileScriptCopy.open(QIODevice::ReadWrite)){
        fileScriptCopy.write(selectDisk.toUtf8());
        fileScriptCopy.close();
    }

    QString res(QString(targetDir).replace("\\", "/"));

    QProcess cmdStart;
    std::cout << "copyFiles" << std::endl;
    cmdStart.start("scriptCopy.bat");
    if(!cmdStart.waitForStarted(-1) || !cmdStart.waitForFinished(-1)) {
        return false;
    }
    if (QDir check(res); !check.exists())
        return false;
    std::cout << "copy finish" << std::endl;
    return true;
}
