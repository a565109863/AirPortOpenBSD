//
//  AirPortOpenBSD_80211.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/7/17.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

int AirPortOpenBSD::bpfOutputPacket(IOInterface* interface, UInt dltType, mbuf_t m)
{
    DebugLog("--%s: 80211 line = %d", __FUNCTION__, __LINE__);
    if (dltType == DLT_IEEE802_11_RADIO || dltType == DLT_IEEE802_11) {
        return bpfOutput80211Radio(interface, m);
    }
    if (dltType == DLT_RAW) {
        return outputActionFrame(interface, m);
    }
    mbuf_freem(m);
    return 1;
}

int AirPortOpenBSD::outputRaw80211Packet(IOInterface* interface, mbuf_t m)
{
    DebugLog("--%s: 80211 line = %d", __FUNCTION__, __LINE__);
    mbuf_freem(m);
    return kIOReturnOutputDropped;
}

int AirPortOpenBSD::outputActionFrame(IOInterface *interface, mbuf_t m)
{
    DebugLog("--%s: 80211 line = %d", __FUNCTION__, __LINE__);
    mbuf_freem(m);
    return kIOReturnOutputDropped;
}

int AirPortOpenBSD::bpfOutput80211Radio(IOInterface *interface, mbuf_t m)
{
    DebugLog("--%s: 80211 line = %d", __FUNCTION__, __LINE__);
    mbuf_freem(m);
    return 0;
}

bool AirPortOpenBSD::useAppleRSNSupplicant(IOInterface *interface)
{
    return false;
}

//bool AirPortOpenBSD::useAppleRSNSupplicant(IO80211VirtualInterface *interface)
//{
//    return false;
//}
