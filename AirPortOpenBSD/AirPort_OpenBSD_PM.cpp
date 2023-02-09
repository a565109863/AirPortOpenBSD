//
//  AirPort_OpenBSD_PM.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#include "AirPort_OpenBSD.hpp"

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
IOReturn AirPort_OpenBSD::registerWithPolicyMaker(IOService* policyMaker)
{
    return policyMaker->registerPowerDriver(this, powerStateArray, kPowerStateCount);
}

IOReturn AirPort_OpenBSD::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
    IOReturn result = IOPMAckImplied;
    
    this->fCommandGate->runAction(setPowerStateAction, &powerStateOrdinal);

done:
    return result;
}

IOReturn AirPort_OpenBSD::setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4)
{
    AirPort_OpenBSD* dev = OSDynamicCast(AirPort_OpenBSD, owner);
    if (dev == NULL)
        return kIOReturnError;
    u_int32_t *powerStateOrdinal = (u_int32_t *)arg1;
    
    struct ifnet *ifp = &dev->ic->ic_if;
    dev->changePowerState(ifp->iface, *powerStateOrdinal);
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD::changePowerState(OSObject *object, u_int32_t powerStateOrdinal)
{
    IOReturn ret = kIOReturnSuccess;
    
    if (powerState == powerStateOrdinal) {
        return ret;
    }
    
    powerState = powerStateOrdinal;
    
    switch (powerStateOrdinal) {
        case APPLE_POWER_ON:
            DebugLog("Setting power on");
            
            if (this->firstUp) {
                this->firstUp = false;
                
                this->configArr[0] = this->getName();
                this->configArr[1] = "up";
                this->configArr[2] = "debug";
                this->configArrCount = 3;
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
            
            this->scanFreeResults();
            
            ret = kIOReturnSuccess;
            break;
        default:
            ret = kIOReturnError;
            break;
    };
    
    return ret;
}
