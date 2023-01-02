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
//            DebugLog("%s role=%d succeed\n", __FUNCTION__, role);
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
//    DebugLog("%s interface=%s role=%d\n", __FUNCTION__, interface->getBSDName(), interface->getInterfaceRole());
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
//    DebugLog("%s interface=%s role=%d\n", __FUNCTION__, interface->getBSDName(), interface->getInterfaceRole());
//    SInt32 ret = super::disableVirtualInterface(interface);
//    if (!ret) {
//        interface->setLinkState(kIO80211NetworkLinkDown, 0);
//        interface->postMessage(APPLE80211_M_LINK_CHANGED);
//        return kIOReturnSuccess;
//    }
//    return ret;
//}
//
//
//IOReturn AirPort_OpenBSD::setVIRTUAL_IF_CREATE(OSObject *object, struct apple80211_virt_if_create_data* data)
//{
//    struct ether_addr addr;
//    struct apple80211_channel chann;
//    DebugLog("%s role=%d, bsd_name=%s, mac=%s, unk1=%d\n", __FUNCTION__, data->role, data->bsd_name,
//          ether_sprintf(data->mac), data->unk1);
//    if (data->role == APPLE80211_VIF_P2P_DEVICE) {
//        IO80211P2PInterface *inf = new IO80211P2PInterface;
//        if (inf == NULL)
//            return kIOReturnError;
//        memcpy(addr.octet, data->mac, 6);
//        inf->init(this, &addr, data->role, "p2p");
//        fP2PDISCInterface = inf;
//    } else if (data->role == APPLE80211_VIF_P2P_GO) {
//        IO80211P2PInterface *inf = new IO80211P2PInterface;
//        if (inf == NULL)
//            return kIOReturnError;
//        memcpy(addr.octet, data->mac, 6);
//        inf->init(this, &addr, data->role, "p2p");
//        fP2PGOInterface = inf;
//    } else if (data->role == APPLE80211_VIF_AWDL) {
//        if (fAWDLInterface != NULL && strncmp((const char *)data->bsd_name, "awdl", 4) == 0) {
//            DebugLog("%s awdl interface already exists!\n", __FUNCTION__);
//            return kIOReturnSuccess;
//        }
//        IO80211P2PInterface *inf = new IO80211P2PInterface;
//        if (inf == NULL)
//            return kIOReturnError;
//        memcpy(addr.octet, data->mac, 6);
//        inf->init(this, &addr, data->role, "awdl");
//        chann.channel = 149;
//        chann.version = 1;
//        chann.flags = APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE | APPLE80211_C_FLAG_80MHZ;
//        setInfraChannel(&chann);
//        fAWDLInterface = inf;
//    } else {
//        DebugLog("%s unhandled virtual interface role type: %d\n", __FUNCTION__, data->role);
//        return kIOReturnError;
//    }
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPort_OpenBSD::setVIRTUAL_IF_DELETE(OSObject *object, struct apple80211_virt_if_delete_data *data)
//{
//    DebugLog("%s bsd_name=%s\n", __FUNCTION__, data->bsd_name);
//    //TODO find vif according to the bsd_name
//    IO80211VirtualInterface *vif = OSDynamicCast(IO80211VirtualInterface, object);
//    if (vif == NULL)
//        return kIOReturnError;
//    detachVirtualInterface(vif, false);
//    vif->release();
//    return kIOReturnSuccess;
//}
