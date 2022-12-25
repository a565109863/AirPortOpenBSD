//
//  AirPort_OpenBSD_Util.cpp
//  AirPortOpenBSD
//
//  Created by User-PC on 2020/8/10.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPort_OpenBSD.hpp"

void AirPort_OpenBSD::firmwareLoadComplete( OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context) {
    AirPort_OpenBSD *dev = (typeof dev)context;
    if(result == kOSReturnSuccess) {
        dev->firmwareData = OSData::withBytes(resourceData, resourceDataLength);
    } else
        printf("firmwareLoadComplete FAILURE: %08x.\n", result);
    IOLockWakeup(dev->fwLoadLock, dev, true);
}

void AirPort_OpenBSD::firmwareLoadComplete(const char* name) {
    for (int i = 0; i < firmwares_total; i++) {
        if (strcmp(firmwares[i].name, name) == 0) {
            firmware fw = firmwares[i];
            this->firmwareData = OSData::withBytes(fw.data, fw.size);
            return;
        }
    }
}

int AirPort_OpenBSD::loadfirmware(const char *firmware_name, u_char **bufp, size_t *buflen)
{
//    IOLockLock(this->fwLoadLock);
//
//    OSReturn ret = OSKextRequestResource(OSKextGetCurrentIdentifier(),
//                                         firmware_name,
//                                         firmwareLoadComplete,
//                                         this,
//                                         NULL);
//    if(ret != kOSReturnSuccess) {
//        IOLog("%s Unable to load firmware file %08x\n", __FUNCTION__, ret);
//        IOLockUnlock(this->fwLoadLock);
//        return 1;
//    }
//    IOLockSleep(this->fwLoadLock, this, THREAD_INTERRUPTIBLE);
//    IOLockUnlock(this->fwLoadLock);
    
    firmwareLoadComplete(firmware_name);
    
    *buflen = this->firmwareData->getLength();
    *bufp = (u_char *)malloc(*buflen, M_DEVBUF, M_NOWAIT);
    memcpy(*bufp , (u_char*)this->firmwareData->getBytesNoCopy(), *buflen);
    
    this->firmwareData->release();
    this->firmwareData = NULL;
    
//    *bufp = (u_char *)this->firmwareData->getBytesNoCopy();
    
    return 0;
}

void AirPort_OpenBSD::if_input(struct ifnet* ifp, struct mbuf_list *ml)
{
    int packets = 0;
    mbuf_t m;
    while ((m = ml_dequeue(ml)) != NULL) {
        if (ifp->iface == NULL) {
            panic("%s ifq->iface == NULL!!!\n", __FUNCTION__);
            break;
        }
        ifp->iface->inputPacket(m, 0, IONetworkInterface::kInputOptionQueuePacket);
        packets ++;
        if (ifp->netStat != NULL) {
            ifp->if_ipackets++;
        }
    }
    
    if (packets)
        ifp->iface->flushInputQueue();
}


IOReturn AirPort_OpenBSD::getVirtIf(OSObject *object)
{
    if (object == NULL) {
        return kIOReturnError;
    }
    
    return (OSDynamicCast(IO80211P2PInterface, object) != NULL) ? kIOReturnSuccess : kIOReturnError ;
}

IOReturn AirPort_OpenBSD::postMessage(unsigned int msg, void* data, unsigned long dataLen)
{
    this->getNetworkInterface()->postMessage(msg, data, dataLen);
    return kIOReturnSuccess;
}



OSString *AirPort_OpenBSD::getNVRAMProperty(char *name)
{
    OSString *value = OSString::withCString("");
    char val[120];
    int ret = 0;
    IOLog("Start Get NARAM value %s", name);
    if (IORegistryEntry *nvram = OSDynamicCast(IORegistryEntry, IODTNVRAM::fromPath("/options", gIODTPlane)))
    {
        if (OSData *buf = OSDynamicCast(OSData, nvram->getProperty(name))) {
            buf->appendByte('\0', 1);
            
            ret = snprintf(val, buf->getLength(), "%s", buf->getBytesNoCopy());
            IOLog("Get NARAM value %s=%s successed ret = %d", name,val, ret);
            if (ret == -1)
                return value;
            value = OSString::withCString(val);
            
            IOLog("Get NARAM value %s=%s successed", name,val);
        }else
        {
             IOLog("Get NARAM value %s failed: Cna't getProperty %s", name, name);
        }
        nvram->release();
    } else {
        IOLog("Get NARAM value %s failed: Can't get /options", name);
    }
    return value;
}
