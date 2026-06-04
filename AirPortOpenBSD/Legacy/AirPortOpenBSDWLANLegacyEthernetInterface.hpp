//
//  AirPortOpenBSDWLANLegacyEthernetInterface.hpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#ifndef AirPortOpenBSDWLANLegacyEthernetInterface_hpp
#define AirPortOpenBSDWLANLegacyEthernetInterface_hpp

#include "apple80211.h"
#include <IOKit/IOLib.h>
#include <libkern/OSKextLib.h>
#include <sys/kernel_types.h>


class AirPortOpenBSDWLANEthernetInterface : public IO80211Interface {
    OSDeclareDefaultStructors(AirPortOpenBSDWLANEthernetInterface)
    
public:
    virtual UInt32   inputPacket(
                                 mbuf_t          packet,
                                 UInt32          length  = 0,
                                 IOOptionBits    options = 0,
                                 void *          param   = 0 ) override;

    bool init(IO80211Controller *controller);

};

#endif /* AirPortOpenBSDWLANLegacyEthernetInterface_hpp */
