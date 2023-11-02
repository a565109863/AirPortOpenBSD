//
//  AirPortOpenBSDCommon_Util.cpp
//  AirPortOpenBSD
//
//  Created by User-PC on 2020/8/10.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <Availability.h>

#include "apple80211.h"

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "AirPortOpenBSD.hpp"
#else
#include "AirPortOpenBSDLegacy.hpp"
#endif

int _stop(struct kmod_info*, void*) {
    IOLog("_stop(struct kmod_info*, void*) has been invoked\n");
    return 0;
};
int _start(struct kmod_info*, void*) {
    IOLog("_start(struct kmod_info*, void*) has been invoked\n");
    return 0;
};

int AirPortOpenBSD::loadfirmware(const char *firmware_name, u_char **bufp, size_t *buflen)
{
    this->pciNub->loadfirmware(firmware_name, bufp, buflen);
    
    return 0;
}

void AirPortOpenBSD::if_input(struct ifnet* ifp, struct mbuf_list *ml)
{
    int packets = 0;
    mbuf_t m;
    while ((m = ml_dequeue(ml)) != NULL) {
        if (ifp->iface == NULL) {
            DebugLog("%s ifq->iface == NULL!!!\n", __FUNCTION__);
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

IOReturn AirPortOpenBSD::tsleepHandler(OSObject* owner, void* arg0 = 0, void* arg1 = 0, void* arg2 = 0, void* arg3 = 0) 
{
    AirPortOpenBSD* dev = OSDynamicCast(AirPortOpenBSD, owner);
    if (dev == NULL)
        return kIOReturnError;
    
    if (arg1 == 0) {
        // no deadline
        if (dev->getCommandGate()->commandSleep(arg0, THREAD_INTERRUPTIBLE) == THREAD_AWAKENED)
            return kIOReturnSuccess;
        else
            return kIOReturnTimeout;
    } else {
        AbsoluteTime deadline;
        clock_absolutetime_interval_to_deadline((*(uint64_t*)arg1), &deadline);
        if (dev->getCommandGate()->commandSleep(arg0, deadline, THREAD_INTERRUPTIBLE) == THREAD_AWAKENED)
            return kIOReturnSuccess;
        else
            return kIOReturnTimeout;
    }
}

void AirPortOpenBSD::if_watchdog(IOTimerEventSource *timer)
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


OSString *AirPortOpenBSD::getNVRAMProperty(char *name)
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
