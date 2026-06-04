//
//  AirPortOpenBSDWLANEthernetInterface.hpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#ifndef AirPortOpenBSDWLANEthernetInterface_hpp
#define AirPortOpenBSDWLANEthernetInterface_hpp

#include "apple80211.h"
#include <IOKit/IOLib.h>
#include <libkern/OSKextLib.h>
#include <sys/kernel_types.h>

class AirPortOpenBSDWLANEthernetInterface : public IOEthernetInterface {
    OSDeclareDefaultStructors(AirPortOpenBSDWLANEthernetInterface)
    
public:
    virtual IOReturn attachToDataLinkLayer( IOOptionBits options,
                                            void *       parameter ) override;
    
    virtual bool initWithSkywalkInterfaceAndProvider(IONetworkController *controller, IO80211SkywalkInterface *interface);
    
    virtual bool setLinkState(IO80211LinkState state);
    
    static errno_t bpfOutputPacket(ifnet_t interface, u_int32_t data_link_type,
                                  mbuf_t packet);
    
    static errno_t bpfTap(ifnet_t interface, u_int32_t data_link_type,
                          bpf_tap_mode direction);
    
    virtual UInt32   inputPacket(
                                 mbuf_t          packet,
                                 UInt32          length  = 0,
                                 IOOptionBits    options = 0,
                                 void *          param   = 0 ) override;
    
private:
    IO80211SkywalkInterface *interface;
};

#endif /* AirPortOpenBSDWLANEthernetInterface_hpp */
