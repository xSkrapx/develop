#include <QCoreApplication>
#include <iostream>
#include "linebreaker.h"
#include "diskpart.h"
#include <QFile>
#include <QDir>
#include <QThread>
#include <QTime>
#include <QRandomGenerator>
#include "windows.h"
#include "fileapi.h"

bool part_1(diskpart *diskCommand, QString driveName)
{
    if(!diskCommand->formatVolume(driveName, "ntfs", 4096, formattingType::QUICK, false)){
        return false;
    }
    return diskCommand->chkdsk(driveName);
}

bool waitingBetweenActions(quint8 time, QString text, QString driveName)
{
    std::cout << text.toStdString() << std::endl;
    if(!diskpart::chkdsk(driveName)){
        return false;
    }
    QThread::sleep(time + 8);
    return true;
}

bool part_2(diskpart *diskCommand, QString driveName)
{
    QFile file(driveName + ":/Новый текстовый документ.txt");
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }
    file.close();
    if(!waitingBetweenActions(2, "Create file", driveName))
        return false;

    file.rename(driveName + ":/HELLO!.txt");
    if(!waitingBetweenActions(2, "Rename file", driveName))
        return false;

    char data[] = {char(0xAA), char(0xAA)};
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }
    file.write(data, 2);
    file.close();
    if(!waitingBetweenActions(2, "Write 0xAA 0xAA in file", driveName))
        return false;


    file.remove();
    if(!waitingBetweenActions(2, "delete file", driveName))
        return false;

    file.rename(driveName + ":/HELLO!.txt");
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }
    file.close();
    if(!waitingBetweenActions(2, "Create HELLO file", driveName))
        return false;

    char data2[] = {char(0xBB), char(0xBB)};
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }
    file.write(data2, 2);
    file.close();
    if(!waitingBetweenActions(2, "Write 0xBB 0xBB in file", driveName))
        return false;

    return true;
}

bool part_3(diskpart *diskCommand, QString driveName)
{
    QDir dir(driveName + ":/");
    QString name("Новая папка");
    QString newName("New Folder");
    if(!dir.mkdir(name)){
        return false;
    }
    if(!waitingBetweenActions(2, "make dir", driveName))
        return false;

    dir.rename(name, newName);
    if(!waitingBetweenActions(2, "rename", driveName))
        return false;

    QStringList names = {"what is it.txt", "File.rar",
                         "file333.fs", "file12341234.txt"};
    QFile file[4];
    uint8_t i = 0;
    for(auto &nameFiles : names){
        file[i].setFileName(driveName + ":/" + newName + "/" + nameFiles);
        i++;
    }
    for(uint8_t i = 0; i < 2; ++i){
        if(!file[i].open(QIODevice::ReadWrite)){
            return false;
        }
        file[i].close();
    }
    if(!waitingBetweenActions(2, "Create two files", driveName))
        return false;

    QString pathForRemove(driveName + ":/" + newName);
    dir.setPath(pathForRemove);
    dir.removeRecursively();
    dir.setPath(driveName + ":/");
    if(!waitingBetweenActions(2, "Delete folder", driveName))
        return false;

    if(!dir.mkdir(newName)){
        return false;
    }
    if(!waitingBetweenActions(2, "Create folder", driveName))
        return false;

    for(uint8_t i = 2; i < 4; ++i){
        if(!file[i].open(QIODevice::ReadWrite)){
            return false;
        }
        file[i].close();
    }
    if(!waitingBetweenActions(2, "Create two files", driveName))
        return false;
    return true;
}

bool part_4(diskpart *diskCommand, QString driveName)
{
    QVector<QPair<QString, quint16>> FS = {{"ntfs", 4096}, {"fat32",4096}, {"exfat",32768}};
    for(auto &fs : FS){
        if(!diskCommand->formatVolume(driveName, fs.first, fs.second, formattingType::QUICK, false)){
            continue;
        }
        if(!waitingBetweenActions(5, QString("Format in fs - %1 clst - %2").arg(fs.first).arg(fs.second), driveName))
            return false;

        QFile file(driveName + ":/Новый текстовый документ.txt");
        if(!file.open(QIODevice::ReadWrite)){
            return false;
        }
        file.close();
        if(!waitingBetweenActions(2, "Create file", driveName))
            return false;

        file.rename(driveName + ":/HELLO!.txt");
        if(!waitingBetweenActions(2, "Rename file", driveName))
            return false;
    }
    return true;
}

bool part_5(diskpart *diskCommand, QString driveName, LineBreaker *com, QVector<quint32> sizes){
    quint64 numberPhysical = diskCommand->changeLogicalToPhysical(driveName);
    if(!diskCommand->cleanDisk(numberPhysical)){
        std::cout << "reconnect disk?(y/n): ";
        uint16_t response;
        std::cin >> response;
        if(response == 'y' or response == 'Y'){
            com->lineBreakerReconnect();
        } else{
            std::cout << "_reconnect_" << std::endl;
        }
    }
    diskCommand->createPart(numberPhysical, sizes.at(0));
    diskCommand->createPart(numberPhysical, sizes.at(1));
    diskCommand->createPart(numberPhysical, 0);
    QVector<uint64_t> volumes = diskCommand->getVolumeFromDisk(numberPhysical);
    QVector<QString> namesList = {"x", "y", "z"};
    QVector<QPair<QString, quint32>> FS = {{"ntfs", 4096}, {"exFat",131072}, {"fat32",4096}};
    uint32_t i = 0;

    for(auto &fs : FS){
        QString volName = diskCommand->volumeNumberToVolName(volumes.at(i), namesList.at(i)).toUpper();

        while(diskCommand->formatVolume(QString::number(volumes.at(i)), fs.first, fs.second, formattingType::QUICK, false) == false){
            fs.second = fs.second / 2;
        }

        if(!waitingBetweenActions(5, QString("format vol: %4; num: %1; number: %3 / %2").arg(volumes.at(i)).arg(FS.size()).arg(i + 1).arg(volName), volName))
            return false;

        QFile file(volName + ":/Новый Документ.doc");
        if(!file.open(QIODevice::ReadWrite)){
            return false;
        }
        file.close();
        if(!waitingBetweenActions(2, "Create file", volName))
            return false;

        file.rename(volName.toUpper() + ":/" + fs.first);
        if(!waitingBetweenActions(2, "Rename file", volName))
            return false;

        i++;
    }
    return true;
}

bool part_6(diskpart *diskCommand, QString driveName){

    return true;
}

bool createFiles(quint16 countFiles, QString driveName)
{
    QVector<QPair<QString, QByteArray>> nameAndData;
    for(quint16 i = 0; i < countFiles; ++i){
        QString name = "Name" + QString::number(i);
        QByteArray data = "Data" + QByteArray::number( i*i*i );
        QPair<QString, QByteArray> tempPair = {name, data};
        nameAndData.append(tempPair);
    }

    for(auto &data : nameAndData){
        QFile file(driveName + ":/" + data.first + ".txt");
        if(!file.open(QIODevice::ReadWrite)){
            return false;
        }
        file.write(data.second);
        file.close();
        if(!waitingBetweenActions(1, data.first, driveName))
            return false;
    }
    return true;
}

bool createFileAndWriteData(diskpart *diskCommand, QString driveName, quint32 count)
{
    QString pathToPattern = "C:/Qt/Qt5.14.2/Docs";
    for(quint16 i = 0; i < count; ++i){
        quint64 numberPhysical = diskCommand->changeLogicalToPhysical(driveName);
        QVector<uint64_t> numberVolumes = diskCommand->getVolumeFromDisk(numberPhysical);
        QString volName;
        for(auto &vol : numberVolumes){
            volName = diskCommand->volumeNumberToVolName(vol, "");
            if(!diskCommand->copyFiles(pathToPattern, QString("%1:/").arg(volName))){
                return false;
            }
            if(!waitingBetweenActions(10, QString("copy: %1").arg(i), driveName)){
                return false;
            }
        }
        QDir dir(QString("%1:/Qt-5.14.2").arg(volName));
        dir.removeRecursively();

        if(!waitingBetweenActions(10, QString("remove: %1").arg(i), driveName)){
            return false;
        }
    }
    return true;
}

bool getImage()
{
    HANDLE handle;
    std::wstring deviceName = L"\\\\.\\E:";

    handle = CreateFile((const wchar_t *)deviceName.data(), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(handle == INVALID_HANDLE_VALUE)
        return GetLastError();

    char data[1000] = {0};
    LPDWORD size = 0;
    int res = ReadFile(handle, data, 1000, size, 0);
    if(res == 0)
        std::cout << GetLastError();

    char buf[1000];
    memcpy(buf, data, 1000);
    return true;
}

QString generateRandomeName()
{
    QTime time(0,0,0);
    QRandomGenerator generator;
    generator.seed(time.secsTo(QTime::currentTime()));
    int lenght = generator.bounded(2,15);
    QString name;
    for(int i = 0; i < lenght; i++){
        char test = generator.bounded(32,126);
        name.append(test);
    }
    return name;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString newName = generateRandomeName();
    QString newName1 = generateRandomeName();
    QString newName2 = generateRandomeName();
    //PARAMETRS-------------
    bool oneSession = false;
    //PARAMETRS-------------

    char drive;
    std::cin >> drive;
    static QString driveName = QString(drive);
    int state;

    diskpart diskCommand;
    LineBreaker com;
    com.driveName = driveName;

//    createFileAndWriteData(&diskCommand, driveName, 100);

//    QString data = diskCommand.chkRAMconsuption("qtcreator.exe");

    std::cout << "PART_1" << std::endl;
    if(!part_1(&diskCommand, driveName)){
        std::cout << "PART_1 ERROR!" << std::endl;
    }
    if(!oneSession){
        std::cout << "Get journal 1: ";
        std::cin >> state;
        com.lineBreakerReconnect();
    }

//    std::cout << "PART_2" << std::endl;
//    if(!part_2(&diskCommand, driveName)){
//        std::cout << "PART_2 ERROR!" << std::endl;
//    }
//    if(!oneSession){
//        std::cout << "Get journal 2: ";
//        std::cin >> state;
//        com.lineBreakerReconnect();
//    }

//    std::cout << "PART_3" << std::endl;
//    if(!part_3(&diskCommand, driveName)){
//        std::cout << "PART_3 ERROR!" << std::endl;
//    }
//    if(!oneSession){
//        std::cout << "Get journal 3: ";
//        std::cin >> state;
//        com.lineBreakerReconnect();
//    }

//    std::cout << "PART_4" << std::endl;
//    if(!part_4(&diskCommand, driveName)){
//        std::cout << "PART_4 ERROR!" << std::endl;
//    }
//    if(!oneSession){
//        std::cout << "Get journal 4: ";
//        std::cin >> state;
//        com.lineBreakerReconnect();
//    }

    QTime time(0,0,0);
    QRandomGenerator generator;
    generator.seed(time.secsTo(QTime::currentTime()));
    QVector<QVector<quint32>> randomSizes;
    for(uint8_t i = 0; i < 100; ++i){
        QVector<quint32> rand;
        rand.append(generator.bounded(500,501));
        rand.append(generator.bounded(500,501));
        randomSizes.append(rand);
    }

    for(auto &sizes : randomSizes){
        std::cout << "PART_5" << std::endl;
        if(!part_5(&diskCommand, driveName, &com, sizes)){
            std::cout << "PART_5 ERROR!" << std::endl;
        }
        if(!oneSession){
            std::cout << "Get journal 5: ";
            std::cin >> state;
            com.lineBreakerReconnect();
        }

        std::cout << "PART_6" << std::endl;
        if(!part_6(&diskCommand, driveName)){
            std::cout << "PART_6 ERROR!" << std::endl;
        }
        if(!oneSession){
            std::cout << "Get journal 6: ";
            std::cin >> state;
            com.lineBreakerReconnect();
        }
        quint64 numberDisk =  diskCommand.changeLogicalToPhysical(driveName);
        diskCommand.cleanDisk(numberDisk);
        diskCommand.createPart(numberDisk, 0);
        part_1(&diskCommand, driveName);
    }

    std::cout << " END " << std::endl;
    return a.exec();
}
