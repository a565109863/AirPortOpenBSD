//
//  AirPortOpenBSDWLANBusInterfacePCIe.cpp
//  AirPortOpenBSD
//
//  Created by TAL on 2023/10/22.
//

#include "AirPortOpenBSDWLANBusInterfacePCIe.hpp"

#define super IOService
OSDefineMetaClassAndStructors(AirPortOpenBSDWLANBusInterfacePCIe, IOService);

IOService* AirPortOpenBSDWLANBusInterfacePCIe::probe(IOService *provider, SInt32 *score)
{
    if (MAC_VERSION_MAJOR != version_major) {
        return NULL;
    }
    
    super::probe(provider, score);
    IOPCIDevice* device = OSDynamicCast(IOPCIDevice, provider);
    if (!device) {
        return NULL;
    }
    
    struct pci_attach_args *_pa = IONew(struct pci_attach_args, 1);
//    _pa->dev.dev = this;
    _pa->pa_tag = device;
    
    _pa->vendor = _pa->pa_tag->configRead16(kIOPCIConfigVendorID);
    _pa->device = _pa->pa_tag->configRead16(kIOPCIConfigDeviceID);
    _pa->pa_id = (_pa->device << 16) + _pa->vendor;
    _pa->subsystem_device = _pa->pa_tag->configRead16(kIOPCIConfigSubSystemID);
    _pa->revision = _pa->pa_tag->configRead8(kIOPCIConfigRevisionID);
    
    for (int i = 0; i < calist.size; i++) {
        struct cfattach *_ca = &calist.ca[i];
        if (_ca->ca_match((struct device *)provider, this, _pa)) {
            this->pa = _pa;
            this->ca = _ca;
            this->cd = &cdlist.cd[i];
            
            pci_disable_msi(pa->pa_pc, pa->pa_tag);
            pci_disable_msix(pa->pa_pc, pa->pa_tag);
            return this;
        }
    }
    IODelete(_pa, struct pci_attach_args, 1);
    return NULL;
}

bool AirPortOpenBSDWLANBusInterfacePCIe::start(IOService *provider)
{
    
#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
    this->fWorkloop = IO80211WorkQueue::workQueue();
#endif

    if (!super::start(provider)) {
        return false;
    }
    
    // 是否开启打印日志
    if (PE_parse_boot_argn("debug", &debug_log, sizeof(debug_log)) == false) {
        debug_log = 0;
    }
    
    this->fPciDevice = OSDynamicCast(IOPCIDevice, provider);
    if (!this->fPciDevice) {
        IOLog("AirPortOpenBSD: Failed to cast provider to IOPCIDevice!");
        return false;
    }
    
    this->setName("WLANPCIeDriver");
    
    this->fwLoadLock = IOLockAlloc();
    
    DebugLog("%s::super start succeed\n", getName());
    
    UInt8 builtIn = 0;
    setProperty("built-in", OSData::withBytes(&builtIn, sizeof(builtIn)));
    registerService();
    return true;
}

void AirPortOpenBSDWLANBusInterfacePCIe::stop(IOService *provider)
{
    if (this->pa) {
        IODelete(this->pa, struct pci_attach_args, 1);
    }
    
    if (this->fwLoadLock != NULL) {
        IOLockFree(this->fwLoadLock);
    }
    super::stop(provider);
}

IOWorkLoop *AirPortOpenBSDWLANBusInterfacePCIe::getWorkLoop() const
{
    return this->fWorkloop;
}
