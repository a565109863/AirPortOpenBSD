//
//  AirPortOpenBSDLegacy_EthernetInterface.hpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#ifndef AirPortOpenBSDLegacy_EthernetInterface_hpp
#define AirPortOpenBSDLegacy_EthernetInterface_hpp

#include "apple80211.h"
#include <IOKit/IOLib.h>
#include <libkern/OSKextLib.h>
#include <sys/kernel_types.h>


class AirPortOpenBSD_EthernetInterface : public IO80211Interface {
    OSDeclareDefaultStructors(AirPortOpenBSD_EthernetInterface)
    
public:
    virtual UInt32   inputPacket(
                                 mbuf_t          packet,
                                 UInt32          length  = 0,
                                 IOOptionBits    options = 0,
                                 void *          param   = 0 ) override;

    bool init(IO80211Controller *controller);

};

#endif /* AirPortOpenBSDLegacy_EthernetInterface_hpp */
