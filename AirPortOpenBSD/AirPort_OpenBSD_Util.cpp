//
//  AirPort_OpenBSD_Util.cpp
//  AirPortOpenBSD
//
//  Created by User-PC on 2020/8/10.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPort_OpenBSD.hpp"

int _stop(struct kmod_info*, void*) {
    IOLog("_stop(struct kmod_info*, void*) has been invoked\n");
    return 0;
};
int _start(struct kmod_info*, void*) {
    IOLog("_start(struct kmod_info*, void*) has been invoked\n");
    return 0;
};

void AirPort_OpenBSD_Class::firmwareLoadComplete( OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context) {
    AirPort_OpenBSD_Class *dev = (typeof dev)context;
    if(result == kOSReturnSuccess) {
        dev->firmwareData = OSData::withBytes(resourceData, resourceDataLength);
    } else
        printf("firmwareLoadComplete FAILURE: %08x.\n", result);
    IOLockWakeup(dev->fwLoadLock, dev, true);
}

//OSData *AirPort_OpenBSD_Class::firmwareLoadComplete(const char* name) {
//    for (int i = 0; i < firmwares_total; i++) {
//        if (strcmp(firmwares[i].name, name) == 0) {
//            struct firmware fw = firmwares[i];
//            return OSData::withBytes(fw.data, fw.size);
//        }
//    }
//
//    return NULL;
//}

AirPortOpenBSDFirmwareData* AirPort_OpenBSD_Class::getFirmwareStore()
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

int AirPort_OpenBSD_Class::loadfirmware(const char *firmware_name, u_char **bufp, size_t *buflen)
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

void AirPort_OpenBSD_Class::if_input(struct ifnet* ifp, struct mbuf_list *ml)
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


IOReturn AirPort_OpenBSD_Class::getVirtIf(OSObject *object)
{
    if (object == NULL) {
        return kIOReturnError;
    }
    
    return (OSDynamicCast(IO80211P2PInterface, object) != NULL) ? kIOReturnSuccess : kIOReturnError ;
}

IOReturn AirPort_OpenBSD_Class::postMessage(unsigned int msg, void* data, unsigned long dataLen)
{
    this->getNetworkInterface()->postMessage(msg, data, dataLen);
    return kIOReturnSuccess;
}

void
AirPort_OpenBSD_Class::ether_ifattach(struct ifnet *ifp)
{
    if (ifp->iface != NULL) {
        return;
    }
    
    this->setName("AirPortOpenBSD");
    
    if (!attachInterface((IONetworkInterface**)&ifp->iface, true)) {
        panic("AirPort_OpenBSD: Failed to attach interface!");
    }
    
}

void
AirPort_OpenBSD_Class::ether_ifdetach(struct ifnet *ifp)
{
    if (ifp->iface == NULL) {
        return;
    }
    detachInterface((IONetworkInterface*)ifp->iface, true);
    ifp->iface = NULL;
}

void AirPort_OpenBSD_Class::setLinkState(int linkState)
{
    DebugLog("ifp->if_link_state = %d, ic_state = %d", linkState, this->ic->ic_state);
    
    struct ifnet *ifp = &this->ic->ic_if;
    
    int reason = 0;
    if (linkState == LINK_STATE_UP) {
        this->setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, this->getCurrentMedium());
        ifp->iface->startOutputThread();
    }else {
//        this->scanFreeResults();
        this->setLinkStatus(kIONetworkLinkValid);
        ifp->iface->stopOutputThread();
        if (this->ic->ic_state == IEEE80211_S_INIT) {
            ifp->iface->flushOutputQueue();
        }
        reason = APPLE80211_REASON_UNSPECIFIED;
    }
    
    this->getNetworkInterface()->setLinkState((IO80211LinkState)linkState, reason);
    ifp->iface->setLinkQualityMetric(100);
    
}

IOReturn AirPort_OpenBSD_Class::tsleepHandler(OSObject* owner, void* arg0 = 0, void* arg1 = 0, void* arg2 = 0, void* arg3 = 0) {
    AirPort_OpenBSD_Class* dev = OSDynamicCast(AirPort_OpenBSD_Class, owner);
    if (dev == NULL)
        return kIOReturnError;
    
    if (arg1 == 0) {
        // no deadline
        if (dev->fCommandGate->commandSleep(arg0, THREAD_INTERRUPTIBLE) == THREAD_AWAKENED)
            return kIOReturnSuccess;
        else
            return kIOReturnTimeout;
    } else {
        AbsoluteTime deadline;
        clock_absolutetime_interval_to_deadline((*(uint64_t*)arg1), &deadline);
        if (dev->fCommandGate->commandSleep(arg0, deadline, THREAD_INTERRUPTIBLE) == THREAD_AWAKENED)
            return kIOReturnSuccess;
        else
            return kIOReturnTimeout;
    }
}

void AirPort_OpenBSD_Class::if_watchdog(IOTimerEventSource *timer)
{
//    if (this->powerState == APPLE_POWER_ON && this->ic->ic_state <= IEEE80211_S_INIT) {
//        DebugLog("");
////        this->ca->ca_activate((struct device *)if_softc, DVACT_WAKEUP);
//    }
    
    struct ifnet *ifp = &this->ic->ic_if;
    if (ifp->if_watchdog) {
        if (ifp->if_timer > 0 && --ifp->if_timer == 0)
                (*ifp->if_watchdog)(ifp);
        
        this->fWatchdogTimer->setTimeoutMS(kTimeoutMS);
    }
}



OSString *AirPort_OpenBSD_Class::getNVRAMProperty(char *name)
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
