////
////  AirPort_OpenBSD_Virtual.cpp
////  AirPortOpenBSD
////
////  Created by Mac-PC on 2023/1/2.
////
//
//#include "AirPort_OpenBSD.hpp"
//
//IO80211VirtualInterface *AirPort_OpenBSD::createVirtualInterface(ether_addr *ether, UInt role)
//{
//    if (role - 1 > 3) {
//        return super::createVirtualInterface(ether, role);
//    }
//    IO80211VirtualInterface *inf = new IO80211VirtualInterface;
//    if (inf) {
//        if (inf->init(this, ether, role, role == APPLE80211_VIF_AWDL ? "awdl" : "p2p")) {
//            IOLog("%s role=%d succeed\n", __FUNCTION__, role);
//        } else {
//            inf->release();
//            return NULL;
//        }
//    }
//    return inf;
//}
//
//SInt32 AirPort_OpenBSD::enableVirtualInterface(IO80211VirtualInterface *interface)
//{
//    IOLog("%s interface=%s role=%d\n", __FUNCTION__, interface->getBSDName(), interface->getInterfaceRole());
//    SInt32 ret = super::enableVirtualInterface(interface);
//    if (!ret) {
//#if MAC_TARGET >= __MAC_13_0
//        interface->setEnabledBySystem(true);
//#endif
//        interface->setLinkState(kIO80211NetworkLinkUp, 0);
//        interface->postMessage(APPLE80211_M_LINK_CHANGED);
//        return kIOReturnSuccess;
//    }
//    return ret;
//}
//
//SInt32 AirPort_OpenBSD::disableVirtualInterface(IO80211VirtualInterface *interface)
//{
//    IOLog("%s interface=%s role=%d\n", __FUNCTION__, interface->getBSDName(), interface->getInterfaceRole());
//    SInt32 ret = super::disableVirtualInterface(interface);
//    if (!ret) {
//        interface->setLinkState(kIO80211NetworkLinkDown, 0);
//        interface->postMessage(APPLE80211_M_LINK_CHANGED);
//        return kIOReturnSuccess;
//    }
//    return ret;
//}
