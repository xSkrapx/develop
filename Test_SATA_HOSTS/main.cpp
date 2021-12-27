#include <QCoreApplication>
#include <diskpart.h>
#include <consoleinterface.h>
#include <getsystemdata.h>

int main(int argc, char *argv[])
{
    getSystemData data;
    sysInfo assInfo;
    assInfo = data.getSysInfoWithCmd();
    data.getSysInfo();
    ConsoleInterface *ci = new ConsoleInterface(argc, argv);


    int err = ci->parse_parameters();
    return err;
}


