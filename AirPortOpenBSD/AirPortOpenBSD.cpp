/* add your code here*/

#include "AirPortOpenBSD.hpp"

OSDefineMetaClassAndStructors(AirPortOpenBSD, IOController);

struct ifnet *_ifp;
IOWorkLoop *_fWorkloop;
IOCommandGate *_fCommandGate;

int logStr_i = 0;
int debug_log = 0;

bool AirPortOpenBSD::init(OSDictionary* parameters) {
    DebugLog("AirPortOpenBSD: Init");
    
    if (MAC_VERSION_MAJOR != version_major) {
        return false;
    }
    
    if (!super::init(parameters)) {
        DebugLog("AirPortOpenBSD: Failed to call super::init!");
        return false;
    }
    
    this->powerState = APPLE_POWER_OFF;
    
    return true;
}

IOService* AirPortOpenBSD::probe(IOService *provider, SInt32 *score)
{
    if (MAC_VERSION_MAJOR != version_major) {
        return NULL;
    }
    
    AirPortOpenBSDWLANBusInterfacePCIe *pciNub = OSDynamicCast(AirPortOpenBSDWLANBusInterfacePCIe, provider);
    if (!pciNub) {
        DebugLog("%s Not a AirPortOpenBSDWLANBusInterfacePCIe instance\n", __FUNCTION__);
        return NULL;
    }
    
    this->fPciDevice = pciNub->fPciDevice;
    this->pa = pciNub->pa;
    if (!this->fPciDevice || !this->pa) {
        DebugLog("%s Not a valid AirPortOpenBSDWLANBusInterfacePCIe instance\n", __FUNCTION__);
        return NULL;
    }
    this->pciNub = pciNub;
    this->fWorkloop = this->pciNub->fWorkloop;
    _fWorkloop = this->fWorkloop;
    
    this->cd = this->pciNub->cd;
    this->ca = this->pciNub->ca;
    
    return super::probe(provider, score);
}

bool AirPortOpenBSD::start(IOService *provider) {
    DebugLog("AirPortOpenBSD: Start");
    
    struct device *dev;
    IOReturn ret = false;
    
    UInt8 builtIn = 0;
    setProperty("built-in", OSData::withBytes(&builtIn, sizeof(builtIn)));
    setProperty("DriverKitDriver", kOSBooleanFalse);
    if (!super::start(provider)) {
        return false;
    }
    
//    fPciDevice = OSDynamicCast(IOPCIDevice, provider);
//    if (!fPciDevice) {
//        DebugLog("AirPortOpenBSD: Failed to cast provider to IOPCIDevice!");
//        goto fail0;
//    }

//    if (!fPciDevice->open(this)) {
//        DebugLog("AirPortOpenBSD: Failed to open provider.\n");
//        return false;
//    }
    
//    if (fPciDevice->requestPowerDomainState(kIOPMPowerOn,
//                                            (IOPowerConnection *) getParentEntry(gIOPowerPlane),
//                                            IOPMLowestState ) != IOPMNoErr) {
//        DebugLog("%s Power thingi failed\n", getName());
//        return  false;
//    }
    
//    fWorkloop = IO80211WorkQueue::workQueue(); // OSDynamicCast(WorkLoop, getWorkLoop());
    if (!fWorkloop) {
        DebugLog("AirPortOpenBSD: Failed to get workloop!");
        goto fail0;
    }
    
    fCommandGate = IOCommandGate::commandGate(this);
    if (!fCommandGate) {
        DebugLog("AirPortOpenBSD: Failed to create command gate!");
        goto fail1;
    }
    
    if (fWorkloop->addEventSource(fCommandGate) != kIOReturnSuccess) {
        DebugLog("AirPortOpenBSD: Failed to register command gate event source!");
        goto fail2;
    }
    
    fWatchdogTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AirPortOpenBSD::if_watchdog));
    
    if (!fWatchdogTimer) {
        DebugLog("AirPortOpenBSD: Failed to create IOTimerEventSource.\n");
        goto fail3;
    }
    
    if (fWorkloop->addEventSource(fWatchdogTimer) != kIOReturnSuccess) {
        DebugLog("AirPortOpenBSD: Failed to register fWatchdogTimer event source!");
        goto fail4;
    }
    
    fScanSource = IOTimerEventSource::timerEventSource(this, &AirPortOpenBSD::apple80211_scan_done);
    if (!fScanSource) {
        DebugLog("AirPortOpenBSD: Failed to create timer event source!\n");
        goto fail5;
    }

    if (fWorkloop->addEventSource(fScanSource) != kIOReturnSuccess) {
        DebugLog("AirPortOpenBSD: Failed to register fScanSource event source!");
        goto fail6;
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
    SLIST_INIT(&this->pa->pa_dmat->bus_dmamap_list);
    
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


void AirPortOpenBSD::stop(IOService *provider) {
    DebugLog("AirPortOpenBSD: stop");
    
    RELEASE(mediumDict);
    RELEASE(firmwareData);
    
    super::stop(provider);
}

void AirPortOpenBSD::free() {
    DebugLog("AirPortOpenBSD: Free");
    
    if (MAC_VERSION_MAJOR != version_major) {
        return;
    }
    
    super::free();
}

IOReturn AirPortOpenBSD::getHardwareAddress(IOEthernetAddress* addr) {
    IEEE80211_ADDR_COPY(addr->bytes, this->ic->ic_myaddr);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setHardwareAddress(const void *addrP, UInt32 addrBytes)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if_setlladdr(ifp, (const UInt8 *)addrP);
    if (this->ic->ic_state > IEEE80211_S_INIT) {
        DebugLog("");
        this->disable(fNetIf);
        this->enable(fNetIf);
    }
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::outputStart(IONetworkInterface *interface, IOOptionBits options)
{
    IOReturn ret = kIOReturnNoResources;
    mbuf_t m;
    while ((interface->dequeueOutputPackets(1, &m) == kIOReturnSuccess)) {
        ret = this->outputPacket(m, NULL);
        if (ret != kIOReturnSuccess) {
            return ret;
        }
    }

    return ret;
}

IOReturn AirPortOpenBSD::networkInterfaceNotification(
                    IONetworkInterface * interface,
                    uint32_t              type,
                    void *                  argument )
{
    DebugLog("%d\n", type);
    return kIOReturnSuccess;
}

UInt32 AirPortOpenBSD::outputPacket(mbuf_t m, void* param) {
    
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
    
    size_t len = mbuf_len(m);
    ether_header_t *eh = (ether_header_t *)mbuf_data(m);
    if (len >= sizeof(ether_header_t) && eh->ether_type == htons(ETHERTYPE_PAE)) { // EAPOL packet
        const char* dump = hexdump((uint8_t*)mbuf_data(m), len);
        DebugLog("output EAPOL packet, len: %zu, data: %s\n", len, dump ? dump : "Failed to allocate memory");
        if (dump)
            IOFree((void*)dump, 3 * len + 1);
    }
    
    if_start(ifp);
    
    ifp->if_opackets++;
    
    return kIOReturnSuccess;
}

IONetworkInterface *AirPortOpenBSD::createInterface()
{
    AirPortOpenBSD_EthernetInterface *netif = new AirPortOpenBSD_EthernetInterface;
    if (!netif) {
        return NULL;
    }
    if (!netif->initWithSkywalkInterfaceAndProvider(this, this->fNetIf)) {
        netif->release();
        return NULL;
    }
    return netif;
}

bool AirPortOpenBSD::configureInterface(IONetworkInterface * netif)
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

    netif->configureOutputPullModel(IFQ_MAXLEN, 0, 0, IOEthernetInterface::kOutputPacketSchedulingModelNormal, 0);

    return true;
}

IOReturn AirPortOpenBSD::enable(IO80211SkywalkInterface *netif) {
    IOReturn result = kIOReturnError;
    
    DebugLog("enable() ===>\n");
    super::enable(netif);
    
    struct ifnet *ifp = &this->ic->ic_if;
    setLinkStatus((kIONetworkLinkValid | kIONetworkLinkActive), mediumTable[MEDIUM_TYPE_AUTO], IF_Mbps(ifp->if_baudrate), NULL);
    
    if (this->powerState == APPLE_POWER_OFF) {
        unsigned long powerStateOrdinal = APPLE_POWER_ON;
        this->getCommandGate()->runAction(setPowerStateAction, &powerStateOrdinal);
    }
    
    result = kIOReturnSuccess;
    
    DebugLog("enable() <===\n");
    
done:
    return result;
}

IOReturn AirPortOpenBSD::disable(IO80211SkywalkInterface *netif) {
    IOReturn result = kIOReturnSuccess;
    
    DebugLog("disable() ===>\n");
    super::disable(netif);
    setLinkStatus(kIONetworkLinkValid);
    
    DebugLog("disable() <===\n");
    return result;
}

//IOReturn AirPortOpenBSD::getMaxPacketSize( UInt32* maxSize ) const {
//    return super::getMaxPacketSize(maxSize);
//}

//IOReturn AirPortOpenBSD::setPromiscuousMode(IOEnetPromiscuousMode mode) {
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::setMulticastMode(IOEnetMulticastMode mode) {
//    return kIOReturnSuccess;
//}

IOReturn AirPortOpenBSD::setMulticastList(IOEthernetAddress* addr, UInt32 len) {
    return kIOReturnSuccess;
}

const OSString* AirPortOpenBSD::newVendorString() const {
    return OSString::withCString("Apple");
}

const OSString* AirPortOpenBSD::newModelString() const {
    struct ifnet *ifp = &this->ic->ic_if;
    return OSString::withCString(ifp->fwname);
}

UInt32 AirPortOpenBSD::getFeatures() const {
    return kIONetworkFeatureMultiPages;
}


IO80211WorkQueue *AirPortOpenBSD::getWorkQueue()
{
    return (IO80211WorkQueue *)this->fWorkloop;
}


SInt32 AirPortOpenBSD::apple80211_ioctl(IO80211SkywalkInterface *interface,unsigned long cmd,void *data, bool b1, bool b2)
{
    if (!ml_at_interrupt_context())
        DebugLog("%s cmd: %s b1: %d b2: %d\n", __FUNCTION__, convertApple80211IOCTLToString((unsigned int)cmd), b1, b2);
    return super::apple80211_ioctl(interface, cmd, data, b1, b2);
}

SInt32 AirPortOpenBSD::apple80211SkywalkRequest(UInt request,int cmd,IO80211SkywalkInterface *interface,void *data)
{
    if (!ml_at_interrupt_context())
        DebugLog("%s 1 cmd: %s request: %d\n", __FUNCTION__, convertApple80211IOCTLToString(cmd), request);
    return kIOReturnUnsupported;
}

SInt32 AirPortOpenBSD::apple80211SkywalkRequest(UInt request,int cmd,IO80211SkywalkInterface *interface,void *data,void *)
{
    if (!ml_at_interrupt_context())
        DebugLog("%s 2 cmd: %s request: %d\n", __FUNCTION__, convertApple80211IOCTLToString(cmd), request);
    return kIOReturnUnsupported;
}


IOReturn AirPortOpenBSD::getPacketFilters(const OSSymbol *group, UInt32 *filters) const
{
    IOReturn    rtn = kIOReturnSuccess;
    if (group == gIOEthernetWakeOnLANFilterGroup && magicPacketSupported)
        *filters = kIOEthernetWakeOnMagicPacket;
    else if (group == gIONetworkFilterGroup)
        *filters = kIOPacketFilterMulticast | kIOPacketFilterPromiscuous;
    else
        rtn = IOEthernetController::getPacketFilters(group, filters);
    return rtn;
}

IOReturn AirPortOpenBSD::setPromiscuousMode(IOEnetPromiscuousMode mode)
{
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setMulticastMode(IOEnetMulticastMode mode)
{
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::selectMedium(const IONetworkMedium *medium) {
    setSelectedMedium(medium);
    return kIOReturnSuccess;
}

bool AirPortOpenBSD::createWorkQueue()
{
    DebugLog("%s %d\n", __FUNCTION__, this->pciNub->fWorkloop != 0);
    return this->pciNub->fWorkloop != 0;
}

#define LOWER32(x)  ((uint64_t)(x) & 0xffffffff)
#define HIGHER32(x) ((uint64_t)(x) >> 32)

bool AirPortOpenBSD::
initCCLogs()
{
    CCPipeOptions driverLogOptions = { 0 };
    driverLogOptions.pipe_type = 0;
    driverLogOptions.log_data_type = 1;
    driverLogOptions.pipe_size = 0x200000;
    driverLogOptions.min_log_size_notify = 0xccccc;
    driverLogOptions.notify_threshold = 1000;
    strlcpy(driverLogOptions.file_name, "Itlwm_Logs", sizeof(driverLogOptions.file_name));
    snprintf(driverLogOptions.name, sizeof(driverLogOptions.name), "wlan%d", 0);
    strlcpy(driverLogOptions.directory_name, "WiFi", sizeof(driverLogOptions.directory_name));
    driverLogOptions.pad9 = 0x1000000;
    driverLogOptions.pad10 = 2;
    driverLogOptions.file_options = 0;
    driverLogOptions.log_policy = 0;
    driverLogPipe = CCPipe::withOwnerNameCapacity(this, "com.zhong.AirPortOpenBSD_Sonoma", "DriverLogs", &driverLogOptions);
    DebugLog("%s driverLogPipeRet %d\n", __FUNCTION__, driverLogPipe != NULL);
    
    memset(&driverLogOptions, 0, sizeof(driverLogOptions));
    driverLogOptions.pipe_type = 0;
    driverLogOptions.log_data_type = 0;
    driverLogOptions.pipe_size = 0x200000;
    driverLogOptions.min_log_size_notify = 0xccccc;
    driverLogOptions.notify_threshold = 1000;
    strlcpy(driverLogOptions.file_name, "AppleBCMWLAN_Datapath", sizeof(driverLogOptions.file_name));
    strlcpy(driverLogOptions.directory_name, "WiFi", sizeof(driverLogOptions.directory_name));
    driverLogOptions.pad9 = HIGHER32(0x202800000);
    driverLogOptions.pad10 = LOWER32(0x202800000);
    driverLogOptions.file_options = 0;
    driverLogOptions.log_policy = 0;
    driverDataPathPipe = CCPipe::withOwnerNameCapacity(this, "com.zhong.AirPortOpenBSD_Sonoma", "DatapathEvents", &driverLogOptions);
    DebugLog("%s driverDataPathPipeRet %d\n", __FUNCTION__, driverDataPathPipe != NULL);
    
    memset(&driverLogOptions, 0, sizeof(driverLogOptions));
    driverLogOptions.pipe_type = 0x200000001;
    driverLogOptions.log_data_type = 2;
    strlcpy(driverLogOptions.file_name, "StateSnapshots", sizeof(driverLogOptions.file_name));
    strlcpy(driverLogOptions.name, "0", sizeof(driverLogOptions.name));
    strlcpy(driverLogOptions.directory_name, "WiFi", sizeof(driverLogOptions.directory_name));
    driverLogOptions.pipe_size = 128;
    driverSnapshotsPipe = CCPipe::withOwnerNameCapacity(this, "com.zhong.AirPortOpenBSD_Sonoma", "StateSnapshots", &driverLogOptions);
    DebugLog("%s driverSnapshotsPipeRet %d\n", __FUNCTION__, driverSnapshotsPipe != NULL);
    
    CCStreamOptions faultReportOptions = { 0 };
    faultReportOptions.stream_type = 1;
    faultReportOptions.console_level = 0xFFFFFFFFFFFFFFFF;
    driverFaultReporter = CCStream::withPipeAndName(driverSnapshotsPipe, "FaultReporter", &faultReportOptions);
    DebugLog("%s driverFaultReporterRet %d\n", __FUNCTION__, driverFaultReporter != NULL);
    return driverLogPipe && driverDataPathPipe && driverSnapshotsPipe && driverFaultReporter;
}

//IOReturn AirPortOpenBSD::setWakeOnMagicPacket(bool active)
//{
//    magicPacketEnabled = active;
//    return kIOReturnSuccess;
//}

bool AirPortOpenBSD::getLogPipes(CCPipe**logPipe, CCPipe**eventPipe, CCPipe**snapshotsPipe)
{
    bool ret = false;
    if (logPipe) {
        *logPipe = driverLogPipe;
        ret = true;
    }
    if (eventPipe) {
        *eventPipe = driverDataPathPipe;
        ret = true;
    }
    if (snapshotsPipe) {
        *snapshotsPipe = driverSnapshotsPipe;
        ret = true;
    }
    return ret;
}

void *AirPortOpenBSD::getFaultReporterFromDriver()
{
    return driverFaultReporter;
}

AirPortOpenBSD_SkywalkInterface* AirPortOpenBSD::getNetworkInterface()
{
    return this->fNetIf;
}

void AirPortOpenBSD::ether_ifattach(struct ifnet *ifp)
{
    if (ifp->iface != NULL) {
        return;
    }
    
    this->setName("AirPortOpenBSD");
    
    struct IOSkywalkEthernetInterface::RegistrationInfo registInfo;
    
    this->fNetIf = new AirPortOpenBSD_SkywalkInterface;
    if (!this->fNetIf->init(this)) {
        panic("Skywalk interface init fail\n");
    }
    this->fNetIf->setInterfaceRole(1);
    this->fNetIf->setInterfaceId(1);
    
    if (!initCCLogs()) {
        panic("CCLog init fail\n");
    }
    if (!this->fNetIf->attach(this)) {
        panic("attach to service fail\n");
    }
    if (!attachInterface(this->fNetIf, this)) {
        panic("attach to interface fail\n");
    }
    if (!IONetworkController::attachInterface((IONetworkInterface**)&ifp->iface, true)) {
        panic("AirPortOpenBSD: Failed to attach interface!");
    }
    memset(&registInfo, 0, sizeof(registInfo));
    if (!this->fNetIf->initRegistrationInfo(&registInfo, 1, sizeof(registInfo))) {
        panic("initRegistrationInfo fail\n");
    }
    this->fNetIf->mExpansionData->fRegistrationInfo = (struct IOSkywalkNetworkInterface::RegistrationInfo *)IOMalloc(sizeof(struct IOSkywalkNetworkInterface::RegistrationInfo));
    this->fNetIf->mExpansionData2->fRegistrationInfo = (struct IOSkywalkEthernetInterface::RegistrationInfo *)IOMalloc(sizeof(struct IOSkywalkEthernetInterface::RegistrationInfo));
    memcpy(this->fNetIf->mExpansionData->fRegistrationInfo, &registInfo, sizeof(registInfo));
    memcpy(this->fNetIf->mExpansionData2->fRegistrationInfo, &registInfo, sizeof(registInfo));
    if (this->fNetIf->getInterfaceRole() == 1)
        this->fNetIf->deferBSDAttach(true);
    this->fNetIf->start(this);
    
}

void AirPortOpenBSD::ether_ifdetach(struct ifnet *ifp)
{
    if (ifp->iface == NULL) {
        return;
    }
    detachInterface((IONetworkInterface*)ifp->iface, true);
    ifp->iface = NULL;
}


void AirPortOpenBSD::setLinkState(int linkState)
{
    DebugLog("ifp->if_link_state = %d, ic_state = %d", linkState, this->ic->ic_state);
    
    struct ifnet *ifp = &this->ic->ic_if;
    
    int reason = 0;
    if (linkState == LINK_STATE_UP) {
        this->setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, this->getCurrentMedium());
        ifp->iface->startOutputThread();
    }else {
//        this->scanFreeResults(0);
        this->setLinkStatus(kIONetworkLinkValid);
        ifp->iface->stopOutputThread();
        ifp->iface->flushOutputQueue();
        
        reason = this->ic->ic_deauth_reason;
    }
    
    this->getNetworkInterface()->setLinkState((IO80211LinkState)linkState, reason);
//    ifp->iface->setLinkQualityMetric(100);
    this->getNetworkInterface()->setRunningState((IO80211LinkState)linkState == kIO80211NetworkLinkUp);
    this->postMessage(APPLE80211_M_LINK_CHANGED, NULL, 0);
    this->postMessage(APPLE80211_M_BSSID_CHANGED, NULL, 0);
    this->postMessage(APPLE80211_M_SSID_CHANGED, NULL, 0);
    if ((IO80211LinkState)linkState == LINK_STATE_UP) {
        this->getNetworkInterface()->reportLinkStatus(3, 0x80);
    } else {
        this->getNetworkInterface()->reportLinkStatus(1, 0);
    }
    ifp->iface->setLinkState((IO80211LinkState)linkState);
    
}

IOReturn AirPortOpenBSD::postMessage(unsigned int msg, void* data, unsigned long dataLen)
{
    this->getNetworkInterface()->postMessage(msg, data, dataLen, false);
    return kIOReturnSuccess;
}
