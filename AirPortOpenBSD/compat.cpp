//
//  compat.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <compat.h>

OSDefineMetaClassAndStructors(pci_intr_handle, OSObject)

void interrupt_func(OSObject *ih, IOInterruptEventSource *src, int count)
{
    pci_intr_handle* _ih = OSDynamicCast(pci_intr_handle, ih);
    if (_ih == NULL)
        return;
    _ih->func(_ih->arg);
}

int tsleep_nsec(void *ident, int priority, const char *wmesg, int timo)
{
    if (_ifp->fCommandGate == NULL) {
        // no command gate so we just sleep
        IOSleep(timo / 1000000000ULL);
        return 1;
    }
    
    IOReturn ret;
    if (timo == 0) {
        ret = _ifp->fCommandGate->runAction(AirPortOpenBSD::tsleepHandler, ident);
    } else {
        ret = _ifp->fCommandGate->runAction(AirPortOpenBSD::tsleepHandler, ident, &timo);
    }
    
    if (ret == kIOReturnSuccess)
        return 0;
    else
        return 1;
}

void wakeup_sleep(void *ident, bool one)
{
    if (_ifp->fCommandGate == NULL)
        return;
    else
        _ifp->fCommandGate->commandWakeup(ident, one);
}

int ticks = INT_MAX - (15 * 60 * 1000);

int loadfirmware(const char *name, u_char **bufp, size_t *buflen)
{
    snprintf(_ifp->fwname, sizeof(_ifp->fwname), "%s", name);
    
    struct device *dev = (struct device *)_ifp->if_softc;
    return dev->dev->loadfirmware(name, bufp, buflen);
}

void if_link_state_change(struct ifnet * ifp)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->setLinkState(ifp->if_link_state);
}

void if_start(struct ifnet *ifp)
{
    (*ifp->if_start)(ifp);
}

void if_input(struct ifnet* ifp, struct mbuf_list *ml)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->if_input(ifp, ml);
}

void ether_ifattach(struct ifnet *ifp)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->ether_ifattach();
}

void ether_ifdetach(struct ifnet *ifp)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->ether_ifdetach();
}
