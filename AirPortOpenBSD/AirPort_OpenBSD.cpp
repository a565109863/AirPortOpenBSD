/* add your code here*/

#include "AirPort_OpenBSD.hpp"

OSDefineMetaClassAndStructors(AirPort_OpenBSD, IOController);
OSDefineMetaClassAndStructors(IOTimeout, OSObject);

struct ifnet *_ifp;
IOWorkLoop *_fWorkloop;
IOCommandGate *_fCommandGate;

int logStr_i = 0;
int debug_log = 0;

bool AirPort_OpenBSD::init(OSDictionary* parameters) {
    IOLog("AirPort_OpenBSD: Init");
    
    if (!super::init(parameters)) {
        IOLog("AirPort_OpenBSD: Failed to call super::init!");
        return false;
    }
    
    fwLoadLock = IOLockAlloc();
    
    this->powerState = APPLE_POWER_OFF;
    
    return true;
}

IOService* AirPort_OpenBSD::probe(IOService* provider, SInt32 *score)
{
    // 是否启用WIFI
    int AirPortOpenBSD = 0;
    if (PE_parse_boot_argn("AirPortOpenBSD", &AirPortOpenBSD, sizeof(AirPortOpenBSD)) == true) {
        if (AirPortOpenBSD == 0) {
            return NULL;
        }
    }
    
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

bool AirPort_OpenBSD::start(IOService* provider) {
    IOLog("AirPort_OpenBSD: Start");
    
    struct device *dev;
    IOReturn ret = false;
    
    if (!super::start(provider)) {
        IOLog("AirPort_OpenBSD: Failed to call super::start!");
        goto fail0;
    }
    
    fPciDevice = OSDynamicCast(IOPCIDevice, provider);
    if (!fPciDevice) {
        IOLog("AirPort_OpenBSD: Failed to cast provider to IOPCIDevice!");
        goto fail0;
    }
    
//
//    if (!fPciDevice->open(this)) {
//        IOLog("AirPort_OpenBSD: Failed to open provider.\n");
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
        IOLog("AirPort_OpenBSD: Failed to get workloop!");
        goto fail0;
    }
    
    fCommandGate = getCommandGate();
    if (!fCommandGate) {
        IOLog("AirPort_OpenBSD: Failed to create command gate!");
        goto fail1;
    }
    
    if (fWorkloop->addEventSource(fCommandGate) != kIOReturnSuccess) {
        IOLog("AirPort_OpenBSD: Failed to register command gate event source!");
        goto fail2;
    }
    
    fWatchdogTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AirPort_OpenBSD::if_watchdog));
    
    if (!fWatchdogTimer) {
        IOLog("AirPort_OpenBSD: Failed to create IOTimerEventSource.\n");
        goto fail3;
    }
    
    if (fWorkloop->addEventSource(fWatchdogTimer) != kIOReturnSuccess) {
        IOLog("AirPort_OpenBSD: Failed to register fWatchdogTimer event source!");
        goto fail4;
    }
    
    fScanSource = IOTimerEventSource::timerEventSource(this, &AirPort_OpenBSD::apple80211_scan_done);
    if (!fScanSource) {
        IOLog("AirPort_OpenBSD: Failed to create timer event source!\n");
        goto fail5;
    }

    if (fWorkloop->addEventSource(fScanSource) != kIOReturnSuccess) {
        IOLog("AirPort_OpenBSD: Failed to register fScanSource event source!");
        goto fail6;
    }
    
    // 是否开启打印日志
    if (PE_parse_boot_argn("debug", &debug_log, sizeof(debug_log)) == false) {
        debug_log = 0;
    }
    
    // 是否开启批量扫描
    if (PE_parse_boot_argn("scanmultiple", &this->scanReqMultiple, sizeof(this->scanReqMultiple)) == false) {
//        IOReturn pmret = this->getAggressiveness(kPMPowerSource, &this->currentPMPowerLevel);
//        if (pmret == kIOReturnSuccess && this->currentPMPowerLevel == kIOPMInternalPower) {
//            // Ventura使用内部电池时，启用批量扫描，提高自动连接速度
//            this->scanReqMultiple = 1;
//        }
        this->scanReqMultiple = 1;
    }
    
    if_softc = malloc(this->ca->ca_devsize, M_DEVBUF, M_NOWAIT);
    dev = (struct device *)if_softc;
    dev->dev = this;
    
    this->ic = (struct ieee80211com *)((char *)if_softc + sizeof(struct device));
    
    _ifp = &this->ic->ic_if;
    _fWorkloop = fWorkloop;
    _fCommandGate = fCommandGate;
    _ifp->if_link_state = LINK_STATE_DOWN;
    
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
    
//    fPciDevice->close(this);
    ret = true;
    
done:
    return ret;
    
fail7:
    if (_ifp->iface) {
        struct ieee80211com *ic = (struct ieee80211com *)_ifp;
        ieee80211_ifdetach(&this->ic->ic_ac.ac_if);
        _ifp->iface = NULL;
    }
    dev = (struct device *)_ifp->if_softc;
    if (dev->ih_count) {
        pci_intr_disestablish(dev, dev->ih);
        pci_intr_unmap(dev->dev->pa, &dev->ih_count);
    }
    IOFree(this->pa->pa_dmat, sizeof(bus_dma_tag_t));
    IOFree(if_softc, this->ca->ca_devsize);
    
    fWorkloop->removeEventSource(fScanSource);
fail6:
    RELEASE(fScanSource);
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

bool AirPort_OpenBSD::addMediumType(UInt32 type, UInt32 speed, UInt32 code, char* name) {
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


void AirPort_OpenBSD::stop(IOService* provider) {
    IOLog("AirPort_OpenBSD: stop");
    
    RELEASE(mediumDict);
    RELEASE(firmwareData);
    
    if (this->pa) {
        IODelete(this->pa, struct pci_attach_args, 1);
    }
    
    super::stop(provider);
}

void AirPort_OpenBSD::free() {
    IOLog("AirPort_OpenBSD: Free");
    
    IOLockFree(fwLoadLock);
    super::free();
}

IOReturn AirPort_OpenBSD::getHardwareAddress(IOEthernetAddress* addr) {
    bcopy(this->ic->ic_myaddr, addr->bytes, kIOEthernetAddressSize);
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD::setHardwareAddress(const IOEthernetAddress *addrP)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if_setlladdr(ifp, addrP->bytes);
    if (this->ic->ic_state > IEEE80211_S_INIT) {
        this->disable(ifp->iface);
        this->enable(ifp->iface);
    }
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD::getHardwareAddressForInterface(IO80211Interface *netif, IOEthernetAddress *addr)
{
    return getHardwareAddress(addr);
}

IOReturn AirPort_OpenBSD::outputStart(IONetworkInterface *interface, IOOptionBits options)
{
    IOReturn ret = kIOReturnNoResources;
    mbuf_t m;
    while ((interface->dequeueOutputPackets(1, &m, NULL, NULL, NULL) == kIOReturnSuccess)) {
        ret = this->outputPacket(m, NULL);
        if (ret != kIOReturnSuccess) {
            return ret;
        }
    }

    return ret;
}

UInt32 AirPort_OpenBSD::outputPacket(mbuf_t m, void* param) {
    
    int error;
    struct ifnet *ifp = &this->ic->ic_if;
    
    if (this->ic->ic_state != IEEE80211_S_RUN) {
        if (m && mbuf_type(m) != MBUF_TYPE_FREE) {
            ifp->if_oerrors++;
            freePacket(m);
        }
        return kIOReturnOutputDropped;
    }
    
    if (m == NULL) {
        ifp->if_oerrors++;
        return kIOReturnOutputDropped;
    }
    if (!(mbuf_flags(m) & MBUF_PKTHDR) ) {
        mbuf_freem(m);
        ifp->if_oerrors++;
        return kIOReturnOutputDropped;
    }
    if (mbuf_type(m) == MBUF_TYPE_FREE) {
        ifp->if_oerrors++;
        return kIOReturnOutputDropped;
    }
    
    IFQ_ENQUEUE(&ifp->if_snd, m, error);
    if (error) {
        ifp->if_oerrors++;
        return kIOReturnOutputDropped;
    }
    
    if_start(ifp);
    
    ifp->if_opackets++;
    
    return kIOReturnSuccess;
}

IONetworkInterface *AirPort_OpenBSD::createInterface()
{
    AirPort_OpenBSD_Interface *netif = new AirPort_OpenBSD_Interface;
    if (!netif) {
        return NULL;
    }
    if (!netif->init(this)) {
        netif->release();
        return NULL;
    }
    return netif;
}

bool AirPort_OpenBSD::configureInterface(IONetworkInterface * netif)
 {
     struct ifnet *ifp = &this->ic->ic_if;
     IONetworkData *data;
     if (super::configureInterface(netif) == false)
         return false;
      
     // Get the generic network statistics structure.
     data = netif->getParameter(kIONetworkStatsKey);
     if (!data || !(netStats = (IONetworkStats *)data->getBuffer())) {
         return false;
     }
     // Get the Ethernet statistics structure.
     data = netif->getParameter(kIOEthernetStatsKey);
     if (!data || !(etherStats = (IOEthernetStats *)data->getBuffer())) {
         return false;
     }
     
     netStats->collisions = 0;
     ifp->netStat = netStats;
    
     netif->configureOutputPullModel(IFQ_MAXLEN, kIONetworkWorkLoopSynchronous);
     
     return true;
}

IOReturn AirPort_OpenBSD::enable(IONetworkInterface *netif) {
    IOReturn result = kIOReturnError;
    
    kprintf("enable() ===>\n");
    
    struct ifnet *ifp = &this->ic->ic_if;
    setLinkStatus((kIONetworkLinkValid | kIONetworkLinkActive), mediumTable[MEDIUM_TYPE_AUTO], IF_Mbps(ifp->if_baudrate), NULL);
    
    if (this->powerState == APPLE_POWER_OFF) {
        unsigned long powerStateOrdinal = APPLE_POWER_ON;
        this->fCommandGate->runAction(setPowerStateAction, &powerStateOrdinal);
    }
    
    result = kIOReturnSuccess;
    
    kprintf("enable() <===\n");
    
done:
    return result;
}

IOReturn AirPort_OpenBSD::disable(IONetworkInterface *netif) {
    IOReturn result = kIOReturnSuccess;
    
    kprintf("disable() ===>\n");
    
    kprintf("disable() <===\n");
    return result;
}

IOReturn AirPort_OpenBSD::getMaxPacketSize( UInt32* maxSize ) const {
    return super::getMaxPacketSize(maxSize);
}

IOReturn AirPort_OpenBSD::setPromiscuousMode(IOEnetPromiscuousMode mode) {
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD::setMulticastMode(IOEnetMulticastMode mode) {
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD::setMulticastList(IOEthernetAddress* addr, UInt32 len) {
    return kIOReturnSuccess;
}

const OSString* AirPort_OpenBSD::newVendorString() const {
    return OSString::withCString("Apple");
}

const OSString* AirPort_OpenBSD::newModelString() const {
    struct ifnet *ifp = &this->ic->ic_if;
    return OSString::withCString(ifp->fwname);
}

UInt32 AirPort_OpenBSD::getFeatures() const {
    return 0x8;
}

