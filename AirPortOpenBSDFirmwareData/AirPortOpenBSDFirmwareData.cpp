/* add your code here */

#include "AirPortOpenBSDFirmwareData.hpp"

OSDefineMetaClassAndStructors(AirPortOpenBSDFirmwareData, IOService)

bool AirPortOpenBSDFirmwareData::start(IOService *provider)
{
    IOLog("Firmware store start\n");
    
    if (!super::start(provider))
        return false;

    registerService();

    return true;
}

void AirPortOpenBSDFirmwareData::stop(IOService *provider)
{
    IOLog("Firmware store stop\n");
    
    super::stop(provider);
}

OSData* AirPortOpenBSDFirmwareData::firmwareLoadComplete(const char* name) {
    OSData* firmwareData = NULL;
    for (int i = 0; i < firmwares_total; i++) {
        if (strcmp(firmwares[i].name, name) == 0) {
            struct firmware fw = firmwares[i];
            firmwareData = OSData::withBytes(fw.data, fw.size);
            break;
        }
    }
    return firmwareData;
}

