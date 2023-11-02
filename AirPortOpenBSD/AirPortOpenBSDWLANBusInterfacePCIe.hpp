//
//  AirPortOpenBSDWLANBusInterfacePCIe.hpp
//  AirPortOpenBSD
//
//  Created by TAL on 2023/10/22.
//

#ifndef AirPortOpenBSDWLANBusInterfacePCIe_hpp
#define AirPortOpenBSDWLANBusInterfacePCIe_hpp


#include <Availability.h>

#include "apple80211.h"

#include "compat.h"
#include "iwlwifi.h"

#include "AirPortOpenBSDFirmwareData.hpp"

extern struct ifnet *_ifp;

class AirPortOpenBSDWLANBusInterfacePCIe : public IOService {
    OSDeclareDefaultStructors(AirPortOpenBSDWLANBusInterfacePCIe)
    
public:
    virtual IOService* probe(IOService* provider, SInt32* score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual IOWorkLoop* getWorkLoop() const override;
    
public:
    IOPCIDevice *fPciDevice;
    IOWorkLoop *fWorkloop;
    struct pci_attach_args *pa;
    struct cfdriver *cd;
    struct cfattach *ca;
    
    
    // firmware data
    AirPortOpenBSDFirmwareData* getFirmwareStore();
    AirPortOpenBSDFirmwareData *_mFirmware;
    
//    OSData *firmwareLoadComplete(const char* name);
    int loadfirmware(const char *name, u_char **bufp, size_t *buflen);
    static void firmwareLoadComplete(OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context);
    IOLock *fwLoadLock;
    OSData *firmwareData;
    
};


#endif /* AirPortOpenBSDWLANBusInterfacePCIe_hpp */
