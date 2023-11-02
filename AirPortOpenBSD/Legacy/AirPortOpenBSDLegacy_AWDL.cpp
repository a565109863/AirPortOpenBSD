////
////  AirPortOpenBSDLegacy_AWDL.cpp
////  AirPortOpenBSD
////
////  Created by Mac-PC on 2022/11/8.
////
//
//#include "AirPortOpenBSDLegacy.hpp"
//
//#define INTERFACE_NAME(object) \
// OSDynamicCast(IO80211Interface, object) == nullptr ? (OSDynamicCast(IO80211P2PInterface, object) == nullptr ? "???" : OSDynamicCast(IO80211P2PInterface, object)->getBSDName()) : OSDynamicCast(IO80211Interface, object)->getBSDName()
//
//
////
//// MARK: 69 - WOW_PARAMETERS
////
//
//IOReturn AirPortOpenBSD::getWOW_PARAMETERS(OSObject *object, struct apple80211_wow_parameter_data *data)
//{
//    return kIOReturnError;
//}
//
//IOReturn AirPortOpenBSD::setWOW_PARAMETERS(OSObject *object, struct apple80211_wow_parameter_data *data)
//{
//    DebugLog("pattern_count=%d\n", data->pattern_count);
//    return kIOReturnError;
//}
//
////
//// MARK: 85 - IE
////
//
//IOReturn AirPortOpenBSD::getIE(OSObject *object, struct apple80211_ie_data *data)
//{
//    DebugLog("%s %s Error\n", __FUNCTION__,  INTERFACE_NAME(object));
//    return kIOReturnError;
//}
//
//IOReturn AirPortOpenBSD::setIE(OSObject *object, struct apple80211_ie_data *data)
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
////
//// MARK: 87 - BTCOEX_MODE
////
//
//IOReturn AirPortOpenBSD::getBTCOEX_MODE(OSObject *object, struct apple80211_btc_mode_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    data->version = APPLE80211_VERSION;
//    data->btc_mode = btcMode;
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::setBTCOEX_MODE(OSObject *object, struct apple80211_btc_mode_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    btcMode = data->btc_mode;
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 93 - P2P_SCAN
////
//
//IOReturn AirPortOpenBSD::setP2P_SCAN(OSObject *object, struct apple80211_scan_data *data)
//{
//    DebugLog("%s %s ssid=%s bssid=%s channel=%d phy_mode=%d scan_type=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->ssid, ether_sprintf(data->bssid.octet), data->num_channels, data->phy_mode, data->scan_type);
//
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 92 - P2P_LISTEN
////
//
//IOReturn AirPortOpenBSD::setP2P_LISTEN(OSObject *object, struct apple80211_p2p_listen_data *data)
//{
//    DebugLog("%s %s channel=%d pad1=%d flags=%d duration=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->channel, data->pad1, data->flags, data->duration);
//
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 98 - P2P_GO_CONF
////
//
//IOReturn AirPortOpenBSD::setP2P_GO_CONF(OSObject *object, struct apple80211_p2p_go_conf_data *data)
//{
//    DebugLog("%s %s auth_upper=%d auth_lower=%d channel=%d bcn_len=%d ssid=%s suppress_beacon=%d\n", __FUNCTION__, INTERFACE_NAME(object), data->auth_upper, data->auth_lower, data->channel, data->bcn_len, data->ssid, data->suppress_beacon);
//
//    return kIOReturnSuccess;
//}
//
//
//
//// MARK: 216 - ROAM_PROFILE
////
//
//IOReturn AirPortOpenBSD::getROAM_PROFILE(OSObject *object, struct apple80211_roam_profile_band_data *data)
//{
//    if (roamProfile == NULL) {
//        DebugLog("no roam profile, return error\n");
//        return kIOReturnError;
//    }
//    memcpy(data, roamProfile, sizeof(struct apple80211_roam_profile_band_data));
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::setROAM_PROFILE(OSObject *object, struct apple80211_roam_profile_band_data *data)
//{
//    DebugLog("cnt=%d flags=%d\n", data->profile_cnt, data->flags);
//
//    if (roamProfile != NULL) {
//        IOFree(roamProfile, sizeof(struct apple80211_roam_profile_band_data));
//    }
//    roamProfile = (uint8_t *)IOMalloc(sizeof(struct apple80211_roam_profile_band_data));
//    memcpy(roamProfile, data, sizeof(struct apple80211_roam_profile_band_data));
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 221 - BTCOEX_PROFILES
////
//
//IOReturn AirPortOpenBSD::getBTCOEX_PROFILES(OSObject *object, struct apple80211_btc_profiles_data *data)
//{
//    if (!data || !btcProfile)
//        return kIOReturnError;
//    memcpy(data, btcProfile, sizeof(struct apple80211_btc_profiles_data));
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::setBTCOEX_PROFILES(OSObject *object, struct apple80211_btc_profiles_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    if (btcProfile)
//        IOFree(btcProfile, sizeof(struct apple80211_btc_profiles_data));
//    btcProfile = (struct apple80211_btc_profiles_data *)IOMalloc(sizeof(struct apple80211_btc_profiles_data));
//    memcpy(btcProfile, data, sizeof(struct apple80211_btc_profiles_data));
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 222 - BTCOEX_CONFIG
////
//
//IOReturn AirPortOpenBSD::getBTCOEX_CONFIG(OSObject *object, struct apple80211_btc_config_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    memcpy(data, &btcConfig, sizeof(struct apple80211_btc_config_data));
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::setBTCOEX_CONFIG(OSObject *object, struct apple80211_btc_config_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    memcpy(&btcConfig, data, sizeof(struct apple80211_btc_config_data));
//    return kIOReturnSuccess;
//}
//
////
//// MARK: 235 - BTCOEX_OPTIONS
////
//
//IOReturn AirPortOpenBSD::
//getBTCOEX_OPTIONS(OSObject *object, struct apple80211_btc_options_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    data->version = APPLE80211_VERSION;
//    data->btc_options = btcOptions;
//    return kIOReturnSuccess;
//}
//
//IOReturn AirPortOpenBSD::
//setBTCOEX_OPTIONS(OSObject *object, struct apple80211_btc_options_data *data)
//{
//    if (!data)
//        return kIOReturnError;
//    btcOptions = data->btc_options;
//    return kIOReturnSuccess;
//}
