//
//  AirPortOpenBSDWLANBusInterfacePCIe_Firmware.cpp
//  AirPortOpenBSD
//
//  Created by TAL on 2023/10/24.
//

#include "AirPortOpenBSDWLANBusInterfacePCIe.hpp"

void AirPortOpenBSDWLANBusInterfacePCIe::firmwareLoadComplete( OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context) {
    AirPortOpenBSDWLANBusInterfacePCIe *pciNub = (typeof pciNub)context;
    if(result == kOSReturnSuccess) {
        pciNub->firmwareData = OSData::withBytes(resourceData, resourceDataLength);
    } else
        DebugLog("firmwareLoadComplete FAILURE: %08x.\n", result);
    IOLockWakeup(pciNub->fwLoadLock, pciNub, true);
}

//OSData *AirPortOpenBSDWLANBusInterfacePCIe::firmwareLoadComplete(const char* name) {
//    for (int i = 0; i < firmwares_total; i++) {
//        if (strcmp(firmwares[i].name, name) == 0) {
//            struct firmware fw = firmwares[i];
//            return OSData::withBytes(fw.data, fw.size);
//        }
//    }
//
//    return NULL;
//}

AirPortOpenBSDFirmwareData* AirPortOpenBSDWLANBusInterfacePCIe::getFirmwareStore()
{
    if (!_mFirmware)
    {
        // check to see if it already loaded
        IOService* tmpStore = waitForMatchingService(serviceMatching(kAirPortOpenBSDFirmwareDataService), 0);
        _mFirmware = OSDynamicCast(AirPortOpenBSDFirmwareData, tmpStore);
        if (!_mFirmware)
        {
            if (tmpStore)
                tmpStore->release();
        }

    }
    
    if (!_mFirmware)
        DebugLog("FirmwareData does not appear to be available.\n");

    return _mFirmware;
}

int AirPortOpenBSDWLANBusInterfacePCIe::loadfirmware(const char *firmware_name, u_char **bufp, size_t *buflen)
{
//    IOLockLock(this->fwLoadLock);
//
//    OSReturn ret = OSKextRequestResource(OSKextGetCurrentIdentifier(),
//                                         firmware_name,
//                                         firmwareLoadComplete,
//                                         this,
//                                         NULL);
//    if(ret != kOSReturnSuccess) {
//        DebugLog("%s Unable to load firmware file %08x\n", __FUNCTION__, ret);
//        IOLockUnlock(this->fwLoadLock);
//        return 1;
//    }
//    IOLockSleep(this->fwLoadLock, this, THREAD_INTERRUPTIBLE);
//    IOLockUnlock(this->fwLoadLock);
    
//    this->firmwareData = this->firmwareLoadComplete(firmware_name);
    
    AirPortOpenBSDFirmwareData* mFirmware = this->getFirmwareStore();
    if (mFirmware == NULL) {
        DebugLog("AirPortOpenBSDFirmwareData is unload");
        return 1;
    }
    this->firmwareData = mFirmware->firmwareLoadComplete(firmware_name);
    if (this->firmwareData == NULL) {
        DebugLog("%s Unable to load firmware file", __FUNCTION__);
        return 1;
    }
    
    
    *buflen = this->firmwareData->getLength();
    *bufp = (u_char *)malloc(*buflen, M_DEVBUF, M_NOWAIT);
    memcpy(*bufp , (u_char*)this->firmwareData->getBytesNoCopy(), *buflen);
    
    this->firmwareData->release();
    this->firmwareData = NULL;
    
//    *bufp = (u_char *)this->firmwareData->getBytesNoCopy();
    
    return 0;
}
