//
//  compat.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <compat.h>

OSDefineMetaClassAndStructors(pci_intr_handle_class, OSObject)

void interrupt_func(OSObject *ih, IOInterruptEventSource *src, int count)
{
    pci_intr_handle_class* _ih = OSDynamicCast(pci_intr_handle_class, ih);
    if (_ih == NULL)
        return;
    _ih->func(_ih->arg);
}

int tsleep_nsec(void *ident, int priority, const char *wmesg, uint64_t nsecs)
{
    if (_fCommandGate == NULL) {
        // no command gate so we just sleep
        IOSleep(nsecs / 1000000000ULL);
        return 1;
    }
    
    IOReturn ret;
    if (nsecs == 0) {
        ret = _fCommandGate->runAction(AirPort_OpenBSD_Class::tsleepHandler, ident);
    } else {
        ret = _fCommandGate->runAction(AirPort_OpenBSD_Class::tsleepHandler, ident, &nsecs);
    }
    
    if (ret == kIOReturnSuccess)
        return 0;
    else
        return 1;
}

void wakeup_sleep(void *ident, bool one)
{
    if (_fCommandGate == NULL)
        return;
    else
        _fCommandGate->commandWakeup(ident, one);
}

int ticks = INT_MAX - (15 * 60 * 1000);

int loadfirmware(const char *name, u_char **bufp, size_t *buflen)
{
    snprintf(_ifp->fwname, sizeof(_ifp->fwname), "%s", name);
//    snprintf(_ifp->fwver, sizeof(_ifp->fwver), "%s", name);
    
    struct device *dev = (struct device *)_ifp->if_softc;
    return dev->dev->loadfirmware(name, bufp, buflen);
}

TAILQ_HEAD(,ifnet) ifnet_list = TAILQ_HEAD_INITIALIZER(ifnet_list);

void if_attach(struct ifnet *ifp)
{
    mq_init(&ifp->if_snd, IFQ_MAXLEN, IPL_NET);
    
    if (ifp->if_enqueue == NULL)
        ifp->if_enqueue = if_enqueue_ifq;
    
    TAILQ_INSERT_TAIL(&ifnet_list, ifp, if_list);
}

void if_detach(struct ifnet *ifp)
{
    
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

void post_message(struct ifnet *ifp, int msgCode)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->postMessage(msgCode);
}

void ether_ifattach(struct ifnet *ifp)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->ether_ifattach(ifp);
}

void ether_ifdetach(struct ifnet *ifp)
{
    struct device *dev = (struct device *)ifp->if_softc;
    dev->dev->ether_ifdetach(ifp);
}

struct ifnet *if_get(const char* if_xname)
{
    struct ifnet *ifp, *tmp;
    TAILQ_FOREACH_SAFE(ifp, &ifnet_list, if_list, tmp) {
//        if (ifp->if_index == sock) {
        if (strcmp(ifp->if_xname, if_xname) == 0) {
            return ifp;
        }
    }
    
    return NULL;
}

int ioctl(int sock, u_long type, void *add)
{
    int ret = 0;
    
    struct ifnet *ifp = if_get(ifname);
    if (ifp == NULL) {
        return 0;
    }
    
    ret = ifp->if_ioctl(ifp, type, (caddr_t)add);
    
    return ret;
}

int ifnet_sock(char* xname)
{
    int sock = -1;
    struct ifnet *ifp, *tmp;
    TAILQ_FOREACH_SAFE(ifp, &ifnet_list, if_list, tmp) {
        if (strcmp(ifp->if_xname, xname) == 0) {
            sock = ifp->if_index;
            break;
        }
    }
    
    return sock;
}
