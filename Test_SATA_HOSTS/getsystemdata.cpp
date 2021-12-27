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

QString getSystemData::getNamePC()
{
    CHAR compName[100];
    DWORD size = sizeof (compName);
    GetComputerNameA(compName, &size);
    return QString(compName);
}
