#include "getsystemdata.h"

getSystemData::getSystemData()
{

}

sysInfo getSystemData::getSysInfo()
{
    sysInfo *info = new sysInfo;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION test;
    DWORD test2 = 100;
    SYSTEM_INFO testInfo;

    HKEY aReg;
    RegConnectRegistryA(NULL, HKEY_LOCAL_MACHINE, &aReg);
    PHKEY aKey;
    RegOpenKeyA(aReg, "HARDWARE/DESCRIPTION/System/CentralProcessor/0", aKey);
    auto name = RegQueryValueExA(*aKey, "ProcessorNameString", NULL, NULL, NULL, NULL);

    GetSystemInfo(&testInfo);

    GetLogicalProcessorInformation(&test, &test2);
    info->PCName = getNamePC();
    return *info;
}

sysInfo getSystemData::getSysInfoWithCmd()
{
    QThread::sleep(1);
    QProcess process;
    process.setWorkingDirectory("C:/Windows/System32");
    QString PROGRAM_NAME = "winsat features -xml test.xml";
    process.start(PROGRAM_NAME);

    if (!process.waitForStarted(-1))
    {
        return {};
    }

    if (!process.waitForFinished(-1))
    {
        return {};
    }

    auto data = process.readAllStandardOutput();
    data.append(process.readAllStandardError());
    QString decodeData(QTextCodec::codecForName("IBM866")->toUnicode(data));
    return {0};
}



QString getSystemData::getNamePC()
{
    CHAR compName[100];
    DWORD size = sizeof (compName);
    GetComputerNameA(compName, &size);
    return QString(compName);
}
