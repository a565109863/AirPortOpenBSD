//
//  AirPortOpenBSDComm_PM.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#include <Availability.h>

#include "apple80211.h"

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "AirPortOpenBSD.hpp"
#else
#include "AirPortOpenBSDLegacy.hpp"
#endif

enum
{
    kPowerStateOff = 0,
    kPowerStateOn,
    kPowerStateCount
};

/* Power Management Support */
static IOPMPowerState powerStateArray[kPowerStateCount] =
{
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};


// Power Management
IOReturn AirPortOpenBSD::registerWithPolicyMaker(IOService* policyMaker)
{
    return policyMaker->registerPowerDriver(this, powerStateArray, kPowerStateCount);
}

IOReturn AirPortOpenBSD::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
    IOReturn result = IOPMAckImplied;
    
    this->getCommandGate()->runAction(setPowerStateAction, &powerStateOrdinal);

done:
    return result;
}

IOReturn AirPortOpenBSD::setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4)
{
    AirPortOpenBSD* dev = OSDynamicCast(AirPortOpenBSD, owner);
    if (dev == NULL)
        return kIOReturnError;
    u_int32_t *powerStateOrdinal = (u_int32_t *)arg1;
    
    struct ifnet *ifp = &dev->ic->ic_if;
    dev->changePowerState(ifp->iface, *powerStateOrdinal);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::changePowerState(OSObject *object, u_int32_t powerStateOrdinal)
{
    IOReturn ret = kIOReturnSuccess;
    
    if (this->powerState == powerStateOrdinal) {
        return ret;
    }
    
    this->powerState = powerStateOrdinal;
    
    switch (powerStateOrdinal) {
        case APPLE_POWER_ON:
            DebugLog("Setting power on");
            
            if (this->firstUp) {
                this->firstUp = false;
                
                struct ifnet *ifp = &this->ic->ic_if;
                this->configArrCount = 0;
                this->configArr[this->configArrCount++] = ifp->if_xname;
                this->configArr[this->configArrCount++] = "up";
                this->configArr[this->configArrCount++] = "debug";
                ifconfig(this->configArr, this->configArrCount);
            }else {
                this->ca->ca_activate((struct device *)if_softc, DVACT_WAKEUP);
            }
            
            this->fWatchdogTimer->cancelTimeout();
            this->fWatchdogTimer->setTimeoutMS(kTimeoutMS);
            
            ret =  kIOReturnSuccess;
            break;
        case APPLE_POWER_OFF:
            DebugLogClean();
            DebugLog("Setting power off");
            this->fWatchdogTimer->cancelTimeout();
            
            this->ca->ca_activate((struct device *)if_softc, DVACT_QUIESCE);
            
            ieee80211_free_allnodes(this->ic, 1);
            
            this->scanFreeResults(0);
            
            ret = kIOReturnSuccess;
            break;
        default:
            ret = kIOReturnError;
            break;
    };
    
    return ret;
}
