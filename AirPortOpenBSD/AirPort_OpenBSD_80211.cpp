//
//  AirPort_OpenBSD_80211.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/7/17.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPort_OpenBSD.hpp"

int AirPort_OpenBSD::bpfOutputPacket(OSObject *object, UInt dltType, mbuf_t m)
{
    IOLog("%s len=%zu\n", __FUNCTION__, mbuf_len(m));
    if (dltType == DLT_IEEE802_11_RADIO || dltType == DLT_IEEE802_11) {
        return bpfOutput80211Radio(object, m);
    }
    if (dltType == DLT_RAW) {
        return outputActionFrame(object, m);
    }
    mbuf_freem(m);
    return 1;
}

int AirPort_OpenBSD::outputRaw80211Packet(IO80211Interface* interface, mbuf_t m)
{
    IOLog("%s len=%zu\n", __FUNCTION__, mbuf_len(m));
    IOReturn ret = interface->outputPacket(m, NULL);
    return ret;
}

int AirPort_OpenBSD::outputActionFrame(OSObject *object, mbuf_t m)
{
    IOReturn ret = kIOReturnOutputDropped;
    IOLog("%s len=%zu\n", __FUNCTION__, mbuf_len(m));
    if (OSDynamicCast(IO80211Interface, object) != NULL) {
        IOLog("%s: line = %d IO80211Interface", __FUNCTION__, __LINE__);
        ret = OSDynamicCast(IO80211Interface, object)->outputPacket(m, NULL);
    } else if (OSDynamicCast(IO80211VirtualInterface, object) != NULL) {
        IOLog("%s: line = %d IO80211VirtualInterface", __FUNCTION__, __LINE__);
        ret = OSDynamicCast(IO80211VirtualInterface, object)->outputPacket(m, NULL);
    } else {
        IOLog("%s: line = %d unknown", __FUNCTION__, __LINE__);
    }
    
    return ret;
}

int AirPort_OpenBSD::bpfOutput80211Radio(OSObject *object, mbuf_t m)
{
    IOReturn ret = kIOReturnOutputDropped;
    IOLog("%s len=%zu\n", __FUNCTION__, mbuf_len(m));
    if (OSDynamicCast(IO80211Interface, object) != NULL) {
        IOLog("%s: line = %d IO80211Interface", __FUNCTION__, __LINE__);
        ret = OSDynamicCast(IO80211Interface, object)->outputPacket(m, NULL);
    } else if (OSDynamicCast(IO80211VirtualInterface, object) != NULL) {
        IOLog("%s: line = %d IO80211VirtualInterface", __FUNCTION__, __LINE__);
        ret = OSDynamicCast(IO80211VirtualInterface, object)->outputPacket(m, NULL);
    } else {
        IOLog("%s: line = %d unknown", __FUNCTION__, __LINE__);
    }
    
    return ret;
}

bool AirPort_OpenBSD::useAppleRSNSupplicant(IO80211Interface *interface)
{
    return true;
}

bool AirPort_OpenBSD::useAppleRSNSupplicant(IO80211VirtualInterface *interface)
{
    return true;
}

SInt32 AirPort_OpenBSD::monitorModeSetEnabled(IO80211Interface *interface, bool enabled, UInt32 dlt)
{
    return kIOReturnSuccess;
}


SInt32 AirPort_OpenBSD::enableFeature(IO80211FeatureCode code, void *data)
{
    if (code == kIO80211Feature80211n) {
        return 0;
    }
    return 0x66;
}



UInt32 AirPort_OpenBSD::hardwareOutputQueueDepth(IO80211Interface *interface)
{
    return 0;
}

SInt32 AirPort_OpenBSD::performCountryCodeOperation(IO80211Interface *interface, IO80211CountryCodeOp op)
{
    return 0;
}


SInt32 AirPort_OpenBSD::stopDMA()
{
    if (_ifp->iface) {
        this->disable(_ifp->iface);
    }
    return 0;
}
