/* add your code here*/

#include "AirPortOpenBSD.hpp"

OSDefineMetaClassAndStructors(AirPortOpenBSD, IOController);
OSDefineMetaClassAndStructors(IOTimeout, OSObject)
#define super IOController

int _stop(struct kmod_info*, void*) {
    IOLog("_stop(struct kmod_info*, void*) has been invoked\n");
    return 0;
};
int _start(struct kmod_info*, void*) {
    IOLog("_start(struct kmod_info*, void*) has been invoked\n");
    return 0;
};

struct ifnet *_ifp;
int logStr_i = 0;

bool AirPortOpenBSD::init(OSDictionary* parameters) {
    IOLog("AirPortOpenBSD: Init");
    
    if (!super::init(parameters)) {
        IOLog("AirPortOpenBSD: Failed to call super::init!");
        return false;
    }
    
    fwLoadLock = IOLockAlloc();
    
    powerState = APPLE_POWER_OFF;
    
    return true;
}

IOService* AirPortOpenBSD::probe(IOService* provider, SInt32 *score)
{
    super::probe(provider, score);
    IOPCIDevice* device = OSDynamicCast(IOPCIDevice, provider);
    if (!device) {
        return NULL;
    }
    
    struct pci_attach_args *_pa = IONew(struct pci_attach_args, 1);
    _pa->dev.dev = this;
    _pa->pa_tag = device;
    
    _pa->vendor = _pa->pa_tag->configRead16(kIOPCIConfigVendorID);
    _pa->device = _pa->pa_tag->configRead16(kIOPCIConfigDeviceID);
    _pa->pa_id = (_pa->device << 16) + _pa->vendor;
    _pa->subsystem_device = _pa->pa_tag->configRead16(kIOPCIConfigSubSystemID);
    _pa->revision = _pa->pa_tag->configRead8(kIOPCIConfigRevisionID);
    
    for (int i = 0; i < calist.size; i++) {
        this->ca = &calist.ca[i];
        this->cd = &cdlist.cd[i];
        if (this->ca->ca_match((struct device *)provider, this, _pa)) {
            this->pa = _pa;
            pci_disable_msi(pa->pa_pc, pa->pa_tag);
            pci_disable_msix(pa->pa_pc, pa->pa_tag);
            return this;
        }
    }
    IODelete(_pa, struct pci_attach_args, 1);
    return NULL;
}

bool AirPortOpenBSD::start(IOService* provider) {
    IOLog("AirPortOpenBSD: Start");
    
    struct ieee80211com *ic;
    struct device *dev;
    IOReturn ret = false;
    
#ifdef Ethernet
    OSArray *IFConfig;
    OSString *NWID;
    OSString *WPAKEY;
    OSBoolean *kSecurityType;
    OSDictionary *wifi;
    
    bool security;
    const char *nwid;
    const char *wpakey;
#endif
    
    if (!super::start(provider)) {
        IOLog("AirPortOpenBSD: Failed to call super::start!");
        goto fail0;
    }
    
    fPciDevice = OSDynamicCast(IOPCIDevice, provider);
    if (!fPciDevice) {
        IOLog("AirPortOpenBSD: Failed to cast provider to IOPCIDevice!");
        goto fail0;
    }
    
//
//    if (!fPciDevice->open(this)) {
//        IOLog("AirPortOpenBSD: Failed to open provider.\n");
//        return false;
//    }
    
//    if (fPciDevice->requestPowerDomainState(kIOPMPowerOn,
//                                            (IOPowerConnection *) getParentEntry(gIOPowerPlane),
//                                            IOPMLowestState ) != IOPMNoErr) {
//        IOLog("%s Power thingi failed\n", getName());
//        return  false;
//    }
    
    fWorkloop = OSDynamicCast(WorkLoop, getWorkLoop());
    if (!fWorkloop) {
        IOLog("AirPortOpenBSD: Failed to get workloop!");
        goto fail0;
    }
    
    fCommandGate = getCommandGate();
    if (!fCommandGate) {
        IOLog("AirPortOpenBSD: Failed to create command gate!");
        goto fail1;
    }
    
    if (fWorkloop->addEventSource(fCommandGate) != kIOReturnSuccess) {
        IOLog("AirPortOpenBSD: Failed to register command gate event source!");
        goto fail2;
    }
    
    fWatchdogTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AirPortOpenBSD::if_watchdog));
    
    if (!fWatchdogTimer) {
        IOLog("AirPortOpenBSD: Failed to create IOTimerEventSource.\n");
        goto fail3;
    }
    
    if (fWorkloop->addEventSource(fWatchdogTimer) != kIOReturnSuccess) {
        IOLog("AirPortOpenBSD: Failed to register fWatchdogTimer event source!");
        goto fail4;
    }
    
    fTimerEventSource = IOTimerEventSource::timerEventSource(this);
    if (!fTimerEventSource) {
        IOLog("AirPortOpenBSD: Failed to create timer event source!\n");
        goto fail5;
    }

    if (fWorkloop->addEventSource(fTimerEventSource) != kIOReturnSuccess) {
        IOLog("AirPortOpenBSD: Failed to register fTimerEventSource event source!");
        goto fail6;
    }
    
    if_softc = malloc(this->ca->ca_devsize, M_DEVBUF, M_NOWAIT);
    dev = (struct device *)if_softc;
    dev->dev = this;
    
    ic = (struct ieee80211com *)((char *)if_softc + sizeof(struct device));
    
    _ifp = &ic->ic_if;
    _ifp->fWorkloop = fWorkloop;
    _ifp->fCommandGate = fCommandGate;
    _ifp->if_link_state = LINK_STATE_DOWN;
    
    this->scanResults = OSArray::withCapacity(512); // by default, but it autoexpands
    
    this->pa->dev.dev = this;
    if (this->cd) {
        bcopy(this->cd->cd_name, this->pa->dev.dv_xname, sizeof(this->pa->dev.dv_xname));
    } else {
        bcopy("AirPortOpenBSD", this->pa->dev.dv_xname, sizeof(this->pa->dev.dv_xname));
    }
    this->pa->workloop = fWorkloop;
    this->pa->pa_tag = fPciDevice;
    this->pa->pa_dmat = (bus_dma_tag_t)malloc(sizeof(bus_dma_tag_t), M_DEVBUF, M_NOWAIT);
    
    fPciDevice->setMemoryEnable(true);
    fPciDevice->setIOEnable(true);
    fPciDevice->setBusMasterEnable(true);
    
    this->ca->ca_attach((struct device *)provider, (struct device *)if_softc, this->pa);
    if (_ifp->err)
    {
        goto fail7;
    }
    
//    this->mediumDict = OSDictionary::withCapacity(MEDIUM_TYPE_INVALID + 1);
//    this->addMediumType(kIOMediumIEEE80211None, 0,  MEDIUM_TYPE_NONE);
//    this->addMediumType(kIOMediumIEEE80211Auto, IF_Mbps(_ifp->if_baudrate),  MEDIUM_TYPE_AUTO);
//    this->addMediumType(kIOMediumIEEE80211DS1, IF_Mbps(1),  MEDIUM_TYPE_1MBIT);
//    this->addMediumType(kIOMediumIEEE80211DS2, IF_Mbps(2),  MEDIUM_TYPE_2MBIT);
//    this->addMediumType(kIOMediumIEEE80211DS5, IF_Mbps(5),  MEDIUM_TYPE_5MBIT);
//    this->addMediumType(kIOMediumIEEE80211DS11, IF_Mbps(11),  MEDIUM_TYPE_11MBIT);
//    this->addMediumType(kIOMediumIEEE80211, IF_Mbps(54),  MEDIUM_TYPE_54MBIT);
//    this->addMediumType(kIOMediumIEEE80211OptionAdhoc, IF_Mbps(300),  MEDIUM_TYPE_300MBIT);

    this->mediumDict = OSDictionary::withCapacity(1);
    this->addMediumType(kIOMediumIEEE80211Auto, IF_Mbps(_ifp->if_baudrate),  MEDIUM_TYPE_AUTO);
    this->publishMediumDictionary(this->mediumDict);
    this->setCurrentMedium(this->mediumTable[MEDIUM_TYPE_AUTO]);
    this->setSelectedMedium(this->mediumTable[MEDIUM_TYPE_AUTO]);
    
    registerService();
    _ifp->iface->registerService();
    

#ifdef Ethernet
    #define kIFConfigName       "IFConfig"
    #define kSecurityTypeName   "WPA/WPA2"
    #define kNwidName           "NWID"
    #define kWpaKeyName         "WPAKEY"

    assoc_data_Arr = OSArray::withCapacity(512);
    IFConfig = OSDynamicCast(OSArray, getProperty(kIFConfigName));
    NWID = OSString::withCString("");
    WPAKEY = OSString::withCString("");
    kSecurityType = OSBoolean::withBoolean(false);
    if (IFConfig->getCount() > 0) {
        for(int i = 0; i < IFConfig->getCount(); i++) {
            wifi = OSDynamicCast(OSDictionary, IFConfig->getObject(i));
            kSecurityType = OSDynamicCast(OSBoolean, wifi->getObject(kSecurityTypeName));
            NWID = OSDynamicCast(OSString, wifi->getObject(kNwidName));
            WPAKEY = OSDynamicCast(OSString, wifi->getObject(kWpaKeyName));
            
            security = kSecurityType->getValue();
            nwid = NWID->getCStringNoCopy();
            wpakey = WPAKEY->getCStringNoCopy();

            struct apple80211_assoc_data ad;
            bzero(&ad, sizeof(apple80211_assoc_data));
            bcopy(nwid, ad.ad_ssid, sizeof(ad.ad_ssid));
            ad.ad_ssid_len = strlen((char *)ad.ad_ssid);
            
            if (security) {
                ad.ad_key.key_cipher_type = APPLE80211_CIPHER_TKIP;
                ad.ad_key.key_len = APPLE80211_KEY_BUFF_LEN;
                u_int32_t passlen = strlen(wpakey);
                if (pkcs5_pbkdf2(wpakey, passlen, (const uint8_t *)nwid, strlen(nwid),
                                 ad.ad_key.key, sizeof(ad.ad_key.key), 4096) != 0)
                    errx(1, "wpakey: passphrase hashing failed");
                
            }else {
                ad.ad_key.key_cipher_type = APPLE80211_CIPHER_NONE;
            }

            OSData* scanresult = OSData::withBytes(&ad, sizeof(ad));
            if (!scanresult) {
                continue;
            }
            assoc_data_Arr->setObject(scanresult);

        }
    }
#endif
    
//    fPciDevice->close(this);
    ret = true;
    
done:
    return ret;
    
fail7:
    if (_ifp->iface){
        struct ieee80211com *ic = (struct ieee80211com *)_ifp;
        ieee80211_ifdetach(&ic->ic_ac.ac_if);
        _ifp->iface = NULL;
    }
    dev = (struct device *)_ifp->if_softc;
    if (dev->ih_count) {
        pci_intr_disestablish(dev, dev->ih);
        pci_intr_unmap(dev->dev->pa, &dev->ih_count);
    }
    IOFree(this->pa->pa_dmat, sizeof(bus_dma_tag_t));
    RELEASE(this->scanResults);
    IOFree(if_softc, this->ca->ca_devsize);
    
    fWorkloop->removeEventSource(fTimerEventSource);
fail6:
    RELEASE(fTimerEventSource);
fail5:
    fWorkloop->removeEventSource(fWatchdogTimer);
fail4:
    RELEASE(fWatchdogTimer);
fail3:
    fWorkloop->removeEventSource(fCommandGate);
fail2:
    RELEASE(fCommandGate);
fail1:
    RELEASE(fWorkloop);
fail0:
    ret = false;
    goto done;
}

bool AirPortOpenBSD::addMediumType(UInt32 type, UInt32 speed, UInt32 code, char* name) {
    bool ret = false;
    
    IONetworkMedium* medium = IONetworkMedium::medium(type, speed, 0, code, name);
    if (medium) {
        ret = IONetworkMedium::addMedium(mediumDict, medium);
        if (ret)
            mediumTable[code] = medium;
        medium->release();
    }
    return ret;
}


void AirPortOpenBSD::stop(IOService* provider) {
    IOLog("AirPortOpenBSD: stop");
    
    RELEASE(mediumDict);
    RELEASE(firmwareData);
    
    if (this->pa) {
        IODelete(this->pa, struct pci_attach_args, 1);
    }
    
    super::stop(provider);
}

void AirPortOpenBSD::free() {
    IOLog("AirPortOpenBSD: Free");
    
    IOLockFree(fwLoadLock);
    super::free();
}

IOReturn AirPortOpenBSD::getHardwareAddress(IOEthernetAddress* addr) {
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    bcopy(ic->ic_myaddr, addr->bytes, kIOEthernetAddressSize);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::outputStart(IONetworkInterface *interface, IOOptionBits options)
{
    mbuf_t m;
    while ((interface->dequeueOutputPackets(1, &m, NULL, NULL, NULL) == kIOReturnSuccess)) {
        IOReturn ret = this->outputPacket(m, NULL);
        if (ret != kIOReturnSuccess) {
            _ifp->if_oerrors++;
            return ret;
        }
    }

    return kIOReturnNoResources;
}

UInt32 AirPortOpenBSD::outputPacket(mbuf_t m, void* param) {
    
    int error;
    
    if (m == NULL) {
        return kIOReturnOutputDropped;
    }
    if (!(mbuf_flags(m) & MBUF_PKTHDR) ){
        mbuf_freem(m);
        return kIOReturnOutputDropped;
    }
    if (mbuf_type(m) == MBUF_TYPE_FREE) {
        return kIOReturnOutputDropped;
    }
    
    IFQ_ENQUEUE(&_ifp->if_snd, m, error);
    if (error) {
        return kIOReturnOutputDropped;
    }
    
    if_start(_ifp);
    
    return kIOReturnSuccess;
}

int AirPortOpenBSD::enqueueInputPacket2(mbuf_t m)
{
    if (!(mbuf_flags(m) & MBUF_PKTHDR) ){
        mbuf_freem(m);
        return kIOReturnError;
    }
    
    return _ifp->iface->enqueueInputPacket(m, 0, IONetworkInterface::kInputOptionQueuePacket);
}

void AirPortOpenBSD::flushInputQueue2()
{
    _ifp->iface->flushInputQueue();
}

#define APPLE_POWER_COUNT 2

/* Power Management Support */
static IOPMPowerState powerStateArray[APPLE_POWER_COUNT] =
{
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};


// Power Management
IOReturn AirPortOpenBSD::registerWithPolicyMaker(IOService* policyMaker)
{
    return policyMaker->registerPowerDriver(this, powerStateArray, APPLE_POWER_COUNT);
}

IOReturn AirPortOpenBSD::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
    IOReturn result = IOPMAckImplied;
    
    this->fCommandGate->runAction(setPowerStateAction, &powerStateOrdinal);

done:
    return result;
}

bool AirPortOpenBSD::configureInterface(IONetworkInterface * netif)
 {
     IONetworkData *data;
     if (super::configureInterface(netif) == false)
         return false;
      
     // Get the generic network statistics structure.
     data = netif->getParameter(kIONetworkStatsKey);
     if (!data || !(netStats = (IONetworkStats *)data->getBuffer())) {
         return false;
     }
     netStats->collisions = 0;
     _ifp->netStat = netStats;

     // Get the Ethernet statistics structure.
     data = netif->getParameter(kIOEthernetStatsKey);
     if (!data || !(etherStats = (IOEthernetStats *)data->getBuffer())) {
         return false;
     }
     
     netif->configureOutputPullModel(IFQ_MAXLEN, kIONetworkWorkLoopSynchronous);
//     netif->configureInputPacketPolling(IFQ_MAXLEN, kIONetworkWorkLoopSynchronous);
       
     return true;
}

IOReturn AirPortOpenBSD::enable(IONetworkInterface *netif) {
    IOReturn result = kIOReturnError;
    
    kprintf("enable() ===>\n");
    
    setLinkStatus((kIONetworkLinkValid | kIONetworkLinkActive), mediumTable[MEDIUM_TYPE_AUTO], IF_Mbps(_ifp->if_baudrate), NULL);
    
    if (powerState == APPLE_POWER_OFF) {
        unsigned long powerStateOrdinal = APPLE_POWER_ON;
        this->fCommandGate->runAction(setPowerStateAction, &powerStateOrdinal);
    }
    
    result = kIOReturnSuccess;
    
    kprintf("enable() <===\n");
    
done:
    return result;
}

IOReturn AirPortOpenBSD::disable(IONetworkInterface *netif) {
    IOReturn result = kIOReturnSuccess;
    
    kprintf("disable() ===>\n");
    
    kprintf("disable() <===\n");
    return result;
}

IOReturn AirPortOpenBSD::getMaxPacketSize( UInt32* maxSize ) const {
    return super::getMaxPacketSize(maxSize);
}

IOReturn AirPortOpenBSD::setPromiscuousMode(IOEnetPromiscuousMode mode) {
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setMulticastMode(IOEnetMulticastMode mode) {
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setMulticastList(IOEthernetAddress* addr, UInt32 len) {
    return kIOReturnSuccess;
}

const OSString* AirPortOpenBSD::newVendorString() const {
    return OSString::withCString("Apple");
}

const OSString* AirPortOpenBSD::newModelString() const {
    return OSString::withCString(_ifp->fwname);
}





IOReturn AirPortOpenBSD::setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4)
{
    AirPortOpenBSD* dev = OSDynamicCast(AirPortOpenBSD, owner);
    if (dev == NULL)
        return kIOReturnError;
    unsigned long *powerStateOrdinal = (unsigned long *)arg1;
    
    dev->changePowerState(_ifp->iface, *powerStateOrdinal);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::changePowerState(IOInterface *interface, int powerStateOrdinal)
{
    IOReturn ret = kIOReturnSuccess;
    
    if (powerState == powerStateOrdinal) {
        return ret;
    }
    
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    
    powerState = powerStateOrdinal;
    
    switch (powerStateOrdinal) {
        case APPLE_POWER_ON:
            DPRINTF(("Setting power on\n"));
            
            if (this->firstUp) {
                this->firstUp = false;
                
                const char *configArr[] = {"up", "debug"};
                ifconfig(configArr, nitems(configArr));
            }else {
                this->ca->ca_activate((struct device *)if_softc, DVACT_WAKEUP);
            }
            
            this->fWatchdogTimer->cancelTimeout();
            this->fWatchdogTimer->setTimeoutMS(kTimeoutMS);

#ifdef Ethernet
            this->fTimerEventSource->setAction(&AirPortOpenBSD::autoASSOC);
            this->fTimerEventSource->setTimeoutUS(1);
#endif
            
            ret =  kIOReturnSuccess;
            break;
        case APPLE_POWER_OFF:
            DPRINTF(("Setting power off\n"));
            this->fWatchdogTimer->cancelTimeout();
            
            this->configArr[0] = "-nwid";
            this->configArr[1] = (const char *)this->assoc_data.ad_ssid;
            this->configArrCount = 2;
            ifconfig(this->configArr, this->configArrCount);
            
            this->ca->ca_activate((struct device *)if_softc, DVACT_QUIESCE);
            this->scanFreeResults();
            
            ieee80211_free_allnodes(ic, 1);
            
            ret = kIOReturnSuccess;
            break;
        default:
            ret = kIOReturnError;
            break;
    };
    
    return ret;
}

void
AirPortOpenBSD::ether_ifattach()
{
    if (_ifp->iface != NULL) {
        return;
    }
    if (!attachInterface((IONetworkInterface**)&_ifp->iface, true)) {
        panic("AirPortOpenBSD: Failed to attach interface!");
    }
}

void
AirPortOpenBSD::ether_ifdetach()
{
    if (_ifp->iface == NULL) {
        return;
    }
    detachInterface((IONetworkInterface*)_ifp->iface, true);
    _ifp->iface = NULL;
}

void AirPortOpenBSD::setLinkState(int linkState)
{
    DebugLog("---%s: line = %d ifp->if_link_state = %d", __FUNCTION__, __LINE__, linkState);
    int reason = 0;
    if (linkState == LINK_STATE_UP) {
        setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, this->getCurrentMedium());
        _ifp->iface->startOutputThread();
    }else {
        setLinkStatus(kIONetworkLinkValid);
        _ifp->iface->stopOutputThread();
        _ifp->iface->flushOutputQueue();
        reason = APPLE80211_REASON_UNSPECIFIED;
    }
    
#ifndef Ethernet
    _ifp->iface->setLinkState((IO80211LinkState)linkState, reason);
    _ifp->iface->setLinkQualityMetric(100);
    _ifp->iface->postMessage(APPLE80211_M_LINK_CHANGED);
#endif
    
}

IOReturn AirPortOpenBSD::tsleepHandler(OSObject* owner, void* arg0 = 0, void* arg1 = 0, void* arg2 = 0, void* arg3 = 0) {
    AirPortOpenBSD* dev = OSDynamicCast(AirPortOpenBSD, owner);
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

void AirPortOpenBSD::if_watchdog(IOTimerEventSource *timer)
{
    if (_ifp->if_watchdog) {
        if (_ifp->if_timer > 0 && --_ifp->if_timer == 0)
                (*_ifp->if_watchdog)(_ifp);
        
        this->fWatchdogTimer->setTimeoutMS(kTimeoutMS);
    }
}
