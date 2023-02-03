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
            struct firmware fw = firmwares[i];
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

void
AirPort_OpenBSD::ether_ifattach(struct ifnet *ifp)
{
    if (ifp->iface != NULL) {
        return;
    }
    
    this->setName(ifp->if_xname);
    
    if (!attachInterface((IONetworkInterface**)&ifp->iface, true)) {
        panic("AirPort_OpenBSD: Failed to attach interface!");
    }
    
}

void
AirPort_OpenBSD::ether_ifdetach(struct ifnet *ifp)
{
    if (ifp->iface == NULL) {
        return;
    }
    detachInterface((IONetworkInterface*)ifp->iface, true);
    ifp->iface = NULL;
}

void AirPort_OpenBSD::setLinkState(int linkState)
{
    DebugLog("ifp->if_link_state = %d", linkState);
    int reason = 0;
    if (linkState == LINK_STATE_UP) {
        setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, this->getCurrentMedium());
        _ifp->iface->startOutputThread();
    }else {
        this->scanFreeResults();
        
        setLinkStatus(kIONetworkLinkValid);
        _ifp->iface->stopOutputThread();
        _ifp->iface->flushOutputQueue();
        reason = APPLE80211_REASON_UNSPECIFIED;
    }
    
    this->getNetworkInterface()->setLinkState((IO80211LinkState)linkState, reason);
    _ifp->iface->setLinkQualityMetric(100);
    
}

IOReturn AirPort_OpenBSD::tsleepHandler(OSObject* owner, void* arg0 = 0, void* arg1 = 0, void* arg2 = 0, void* arg3 = 0) {
    AirPort_OpenBSD* dev = OSDynamicCast(AirPort_OpenBSD, owner);
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
        clock_interval_to_deadline((*(int*)arg1), kNanosecondScale, reinterpret_cast<uint64_t*> (&deadline));
        if (dev->fCommandGate->commandSleep(arg0, deadline, THREAD_INTERRUPTIBLE) == THREAD_AWAKENED)
            return kIOReturnSuccess;
        else
            return kIOReturnTimeout;
    }
}

void AirPort_OpenBSD::if_watchdog(IOTimerEventSource *timer)
{
    if (_ifp->if_watchdog) {
        if (_ifp->if_timer > 0 && --_ifp->if_timer == 0)
                (*_ifp->if_watchdog)(_ifp);
        
        this->fWatchdogTimer->setTimeoutMS(kTimeoutMS);
    }
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
