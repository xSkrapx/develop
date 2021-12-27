#ifndef CONSOLEINTERFACE_H
#define CONSOLEINTERFACE_H

#include <QCoreApplication>
#include <QCommandLineParser>
#include <diskpart.h>

class ConsoleInterface : public QCoreApplication
{
public:
    ConsoleInterface(int argc, char** argv);
    int parse_parameters();

private:
    QCommandLineParser command_line_parser;

    QCommandLineOption disk_init;
    diskpart *disk;
//    QCommandLineOption spaceWire_Recieve;
//    QCommandLineOption spaceWire_Read;
//    QCommandLineOption spaceWire_Reset;
//    QCommandLineOption spaceWire_Config;
//    QCommandLineOption spaceWire_ConfigPort;
};

#endif // CONSOLEINTERFACE_H
