#include "linebreaker.h"
#include "QThread"
#include <iostream>
#include <QTime>
#include <QtConcurrent/QtConcurrent>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

bool LineBreaker::lineBreakerReconnect()
{
    if(COMnumber.isEmpty()){
        COMnumber = findMecanicalBreaker();
    }

    QByteArray onUSB = "on 17\r\n";
    QByteArray offUSB = "off 17\r\n";
    QThread::sleep(1);

    if(!sendToSerialPort(offUSB, COMnumber, true)){ // размыкание
        return false;
    }
    if(!waitingForDiskConnectionDisconnection(false)){
        return false;
    }

    QThread::sleep(1);
    if(!sendToSerialPort(onUSB, COMnumber, true)){ // Поключение
        return false;
    }
    if(!waitingForDiskConnectionDisconnection(true)){
        return false;
    }

    QThread::sleep(1);
    return true;
}


QString LineBreaker::findMecanicalBreaker()
{
    QByteArray onUSB = "help\r\n";
    if(true){
        QSerialPortInfo info;
        auto test = info.availablePorts();

        for(auto &com : info.availablePorts()){
            QSerialPort theComPort(com.portName());
            theComPort.setParity(QSerialPort::EvenParity);
            theComPort.setBaudRate(9600);
            theComPort.setFlowControl(QSerialPort::FlowControl::UnknownFlowControl);
            theComPort.setDataBits(QSerialPort::DataBits::Data8);
            theComPort.setStopBits(QSerialPort::StopBits::OneStop);

            if(theComPort.open(QIODevice::ReadWrite)){
                theComPort.clearError();
                theComPort.setRequestToSend(false);
                theComPort.setDataTerminalReady(false);
                theComPort.setReadBufferSize(256);
                theComPort.setBreakEnabled(false);

                if (theComPort.write(onUSB) &&
                        theComPort.waitForBytesWritten())
                {
                    QByteArray readData = theComPort.readAll();
                    theComPort.waitForReadyRead(3000);
                    readData += theComPort.readAll();
                    if (readData.contains("set_step_delay ms"))
                    {
                        theComPort.close();
                        std::cout << "Mechanical breaker is " << com.portName().toStdString() << std::endl;
                        return com.portName();
                    }
                }
                theComPort.close();
            }
        }
        std::cout << "------Mechanical breaker not found!-------" << std::endl;
    }
    return { };
}


bool LineBreaker::sendToSerialPort(const QByteArray &data, QString portNumber, bool forMecanicalBreaker) const
{
    //    for (auto thePort : QSerialPortInfo::availablePorts())
    //    {
    //        if (thePort.portName().startsWith("COM") &&
    //                portNumber == thePort.portName().remove(0, 3).toUInt())
    //        {

    QSerialPort theComPort(portNumber);

    if(forMecanicalBreaker){
        theComPort.setParity(QSerialPort::EvenParity);
        theComPort.setBaudRate(9600);
        theComPort.setFlowControl(QSerialPort::FlowControl::UnknownFlowControl);

    } else{
        theComPort.setParity(QSerialPort::Parity::NoParity);
        theComPort.setBaudRate(921600);
        theComPort.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    }
    theComPort.setDataBits(QSerialPort::DataBits::Data8);
    theComPort.setStopBits(QSerialPort::StopBits::OneStop);

    if (theComPort.open(QIODevice::ReadWrite))
    {
        theComPort.clearError();
        theComPort.setRequestToSend(false);
        theComPort.setDataTerminalReady(false);
        theComPort.setReadBufferSize(256);
        theComPort.setBreakEnabled(false);

        if (theComPort.write(data) &&
                theComPort.waitForBytesWritten())
        {
            QByteArray readData = theComPort.readAll();
            if(!forMecanicalBreaker){
                for (int i = 0; i < 10; ++i)
                {
                    theComPort.waitForReadyRead(3000);
                    readData += theComPort.readAll();

                    if (readData.contains("Saving the SSD... OK"))
                    {
                        theComPort.close();
                        return true;
                    }
                }
            } else{
                theComPort.waitForReadyRead(3000);
                readData += theComPort.readAll();
                if (readData.contains("[OK]"))
                {
                    theComPort.close();
                    return true;
                }
            }

            theComPort.close();
            return false;
        }
    } else {
        return false;
    }
    //        }
    //    }

    return { };
}


bool LineBreaker::waitingForDiskConnectionDisconnection(bool connection){
    QTimer timer;
    QEventLoop loop;

    connect(&timer, &QTimer::timeout, this, [&](){
        std::cout << "Fail drive connect" << std::endl;
        loop.exit();
        return false;
    });

    timer.start(50000);

    auto future = QtConcurrent::run([&]()
    {
        while (!(QFile::exists(driveName + ":/") == connection)) {
        }
        loop.exit();
    });

    loop.exec();

//    if (QFile::exists(driveName)){
//        std::cout << "Drive is connect" << std::endl;
//    } else {
//        std::cout << "Drive is disconnect" << std::endl;
//    }
    return true;
}
