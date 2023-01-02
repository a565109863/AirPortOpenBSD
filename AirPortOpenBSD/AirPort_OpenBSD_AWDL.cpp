////
////  AirPort_OpenBSD_AWDL.cpp
////  AirPortOpenBSD
////
////  Created by Mac-PC on 2022/11/8.
////
//
//#include "AirPort_OpenBSD.hpp"
//
//#define INTERFACE_NAME(object) \
// OSDynamicCast(IO80211Interface, object) == nullptr ? (OSDynamicCast(IO80211P2PInterface, object) == nullptr ? "???" : OSDynamicCast(IO80211P2PInterface, object)->getBSDName()) : OSDynamicCast(IO80211Interface, object)->getBSDName()
//
//
////
//// MARK: 69 - WOW_PARAMETERS
////
//
//IOReturn AirPort_OpenBSD::getWOW_PARAMETERS(OSObject *object, struct apple80211_wow_parameter_data *data)
//{
//    return kIOReturnError;
//}
//
//IOReturn AirPort_OpenBSD::setWOW_PARAMETERS(OSObject *object, struct apple80211_wow_parameter_data *data)
//{
//    DebugLog("pattern_count=%d\n", data->pattern_count);
//    return kIOReturnError;
//}
//
//
////
//// MARK: 85 - IE
////
//
//IOReturn AirPort_OpenBSD::getIE(OSObject *object, struct apple80211_ie_data *data)
//{
//    DebugLog("%s %s Error\n", __FUNCTION__,  INTERFACE_NAME(object));
//    return kIOReturnError;
//}
//
//IOReturn AirPort_OpenBSD::setIE(OSObject *object, struct apple80211_ie_data *data)
//{
//    DebugLog("%s %s frame_type_flags %x add %d signature_len %d ie_len %d\n", __FUNCTION__, INTERFACE_NAME(object), data->frame_type_flags, data->add, data->signature_len, data->ie_len);
//    if (data->frame_type_flags == APPLE80211_IE_FLAG_ASSOC_REQ && data->add && data->ie_len && *(uint8_t*)data->ie == 68) {
//        DebugLog("%s setCustomAssocIE\n", __FUNCTION__);
//        return kIOReturnSuccess;
//    }
//
//    return kIOReturnSuccess;
//}
//
//
////
//// MARK: 93 - P2P_SCAN
////
//
//IOReturn AirPort_OpenBSD::setP2P_SCAN(OSObject *object, struct apple80211_scan_data *data)
//{
//    DebugLog("%s %s ssid=%s bssid=%s channel=%d phy_mode=%d scan_type=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->ssid, ether_sprintf(data->bssid.octet), data->num_channels, data->phy_mode, data->scan_type);
//
//    return kIOReturnSuccess;
//}
//
//
////
//// MARK: 92 - P2P_LISTEN
////
//
//IOReturn AirPort_OpenBSD::setP2P_LISTEN(OSObject *object, struct apple80211_p2p_listen_data *data)
//{
//    DebugLog("%s %s channel=%d pad1=%d flags=%d duration=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->channel, data->pad1, data->flags, data->duration);
//
//    return kIOReturnSuccess;
//}
//
//
////
//// MARK: 98 - P2P_GO_CONF
////
//
//IOReturn AirPort_OpenBSD::setP2P_GO_CONF(OSObject *object, struct apple80211_p2p_go_conf_data *data)
//{
//    DebugLog("%s %s auth_upper=%d auth_lower=%d channel=%d bcn_len=%d ssid=%s suppress_beacon=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->auth_upper, data->auth_lower, data->channel, data->bcn_len, data->ssid, data->suppress_beacon);
//
//    return kIOReturnSuccess;
//}
