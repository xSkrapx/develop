#include "consoleinterface.h"

ConsoleInterface::ConsoleInterface(int argc, char** argv):
    QCoreApplication(argc, argv),
    disk_init({ "di", "diskInit" }, "diskInit", "diskInit")
{
    disk = new diskpart;
    command_line_parser.setApplicationDescription("SpaceWire console interface");
    command_line_parser.addHelpOption();
    command_line_parser.addVersionOption();

    command_line_parser.addOption(disk_init);

    command_line_parser.process(*this);
}

int ConsoleInterface::parse_parameters()
{
    if (command_line_parser.isSet(disk_init))
    {
        auto args = command_line_parser.values(disk_init);
        auto firstArg = args.at(0);

        QString image;
        QString journal;
        firstArg.replace("\\", "/");
        firstArg.replace("{", "");
        firstArg.replace("}", "");
        args = firstArg.split(",", QString::SkipEmptyParts);

        char diskNumber = args.at(0).toInt();

        if (args.count() > 1){
            quint64 size = args.at(1).toInt();
            disk->createPart(diskNumber, size);
        } else{
            disk->createPart(diskNumber, 0, true);
        }
    }
    return 1;
}
