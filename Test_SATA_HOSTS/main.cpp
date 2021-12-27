#include <QCoreApplication>
#include <diskpart.h>
#include <consoleinterface.h>
#include <getsystemdata.h>

int main(int argc, char *argv[])
{
    ConsoleInterface *ci = new ConsoleInterface(argc, argv);

    getSystemData data;
    sysInfo assInfo;

    data.getSysInfo();

    int err = ci->parse_parameters();
    return err;
}


