//
//  AirPortOpenBSDWLANLegacyEthernetInterface.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//

#include "AirPortOpenBSDWLANLegacyEthernetInterface.hpp"

#define super IO80211Interface
OSDefineMetaClassAndStructors(AirPortOpenBSDWLANEthernetInterface, IO80211Interface);

bool AirPortOpenBSDWLANEthernetInterface::init(IO80211Controller *controller)
{
    if (!super::init(controller)) {
        return false;
    }

    return true;
}

UInt32 AirPortOpenBSDWLANEthernetInterface::inputPacket(mbuf_t packet, UInt32 length, IOOptionBits options, void *param)
{
    uint16_t ether_type;
    size_t len = mbuf_len(packet);
    if (len >= 14 && mbuf_copydata(packet, 12, 2, &ether_type) == 0 && ether_type == _OSSwapInt16(ETHERTYPE_PAE)) { // EAPOL packet
        return IO80211Interface::inputPacket(packet, (UInt32)len, 0, param);
    }
    return IOEthernetInterface::inputPacket(packet, length, options, param);
}
