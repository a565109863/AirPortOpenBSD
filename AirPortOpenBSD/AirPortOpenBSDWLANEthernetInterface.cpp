//
//  AirPortOpenBSDWLANEthernetInterface.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/5.
//
#include "AirPortOpenBSD.hpp"
#include "AirPortOpenBSDWLANEthernetInterface.hpp"

#define super IOEthernetInterface
OSDefineMetaClassAndStructors(AirPortOpenBSDWLANEthernetInterface, IOEthernetInterface);

bool AirPortOpenBSDWLANEthernetInterface::
initWithSkywalkInterfaceAndProvider(IONetworkController *controller, IO80211SkywalkInterface *interface)
{
    bool ret = super::init(controller);
    if (ret)
        this->interface = interface;
    return ret;
}

IOReturn AirPortOpenBSDWLANEthernetInterface::
attachToDataLinkLayer( IOOptionBits options, void *parameter )
{
    char infName[IFNAMSIZ];
    IOReturn ret = super::attachToDataLinkLayer(options, parameter);
    if (ret == kIOReturnSuccess && interface) {
        UInt8 builtIn = 0;
        IOEthernetAddress addr;
        interface->setProperty("built-in", OSData::withBytes(&builtIn, sizeof(builtIn)));
        snprintf(infName, sizeof(infName), "%s%u", ifnet_name(getIfnet()), ifnet_unit(getIfnet()));
        interface->setProperty("IOInterfaceName", OSString::withCString(infName));
        interface->setProperty(kIOInterfaceUnit, OSNumber::withNumber(ifnet_unit(getIfnet()), 8));
        interface->setProperty(kIOInterfaceNamePrefix, OSString::withCString(ifnet_name(getIfnet())));
        if (OSDynamicCast(IOEthernetController, getController())->getHardwareAddress(&addr) == kIOReturnSuccess)
            setProperty(kIOMACAddress,  (void *) &addr,
                        kIOEthernetAddressSize);
        interface->registerService();
        interface->prepareBSDInterface(getIfnet(), 0);
//        ret = bpf_attach(getIfnet(), DLT_RAW, 0x48, &AirPortOpenBSDWLANEthernetInterface::bpfOutputPacket, &AirPortOpenBSDWLANEthernetInterface::bpfTap);
    }
    return ret;
}

errno_t AirPortOpenBSDWLANEthernetInterface::
bpfOutputPacket(ifnet_t interface, u_int32_t data_link_type, mbuf_t packet)
{
    DebugLog("%s data_link_type: %d\n", __FUNCTION__, data_link_type);
    AirPortOpenBSDWLANEthernetInterface *networkInterface = (AirPortOpenBSDWLANEthernetInterface *)ifnet_softc(interface);
    return networkInterface->enqueueOutputPacket(packet);
}

errno_t AirPortOpenBSDWLANEthernetInterface::
bpfTap(ifnet_t interface, u_int32_t data_link_type, bpf_tap_mode direction)
{
    DebugLog("%s data_link_type: %d direction: %d\n", __FUNCTION__, data_link_type, direction);
    return 0;
}

bool AirPortOpenBSDWLANEthernetInterface::
setLinkState(IO80211LinkState state)
{
    if (state == kIO80211NetworkLinkUp) {
        ifnet_set_flags(getIfnet(), ifnet_flags(getIfnet()) | (IFF_UP | IFF_RUNNING), (IFF_UP | IFF_RUNNING));
    } else {
        ifnet_set_flags(getIfnet(), ifnet_flags(getIfnet()) & ~(IFF_UP | IFF_RUNNING), 0);
    }
    return true;
}

UInt32 AirPortOpenBSDWLANEthernetInterface::
inputPacket(mbuf_t packet, UInt32 length, IOOptionBits options, void *param)
{
    ether_header_t *eh;
    size_t len = mbuf_len(packet);
    
    eh = (ether_header_t *)mbuf_data(packet);
    if (len >= sizeof(ether_header_t) && eh->ether_type == htons(ETHERTYPE_PAE)) { // EAPOL packet
        const char* dump = hexdump((uint8_t*)mbuf_data(packet), len);
        DebugLog("input EAPOL packet, len: %zu, pktlen: %zu, data: %s\n", len, mbuf_pkthdr_len(packet), dump ? dump : "Failed to allocate memory");
        if (dump)
            IOFree((void*)dump, 3 * len + 1);
    }
    return IOEthernetInterface::inputPacket(packet, length, options, param);
}
