//
//  AirPortOpenBSD_Ioctl.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

//
// MARK: 1 - SSID
//

IOReturn AirPortOpenBSD::getSSID(IOInterface *interface, struct apple80211_ssid_data *sd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    
    struct ifreq ifr;
    struct ieee80211_nwid nwid;
    
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_data = (caddr_t)&nwid;
    
    if (_ifp->if_ioctl(_ifp, SIOCG80211NWID, (caddr_t)&ifr) == -1)
        return kIOReturnError;
    
    memset(sd, 0, sizeof(*sd));
    sd->version = APPLE80211_VERSION;
    sd->ssid_len = nwid.i_len;
    memcpy(sd->ssid_bytes, nwid.i_nwid, sd->ssid_len);
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setSSID(IOInterface *interface, struct apple80211_ssid_data *sd)
{
    
    this->configArr[0] = "nwid";
    this->configArr[1] = (const char *)sd->ssid_bytes;
    this->configArrCount = 2;
    ifconfig(this->configArr, this->configArrCount);
    
//    _ifp->iface->postMessage(APPLE80211_M_SSID_CHANGED);

    return kIOReturnSuccess;
}

//
// MARK: 2 - AUTH_TYPE
//

IOReturn AirPortOpenBSD::getAUTH_TYPE(IOInterface *interface, struct apple80211_authtype_data *ad)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    *ad = this->authtype_data;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setAUTH_TYPE(IOInterface *interface, struct apple80211_authtype_data *ad)
{
    this->authtype_data = *ad;
    return kIOReturnSuccess;
}

//
// MARK: 3 - CIPHER_KEY
//

IOReturn AirPortOpenBSD::getCIPHER_KEY(IOInterface *interface, struct apple80211_key *key)
{
    *key = this->cipher_key;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setCIPHER_KEY(IOInterface *interface, struct apple80211_key *key)
{
    this->cipher_key = *key;
    
    DebugLog("--%s: line = %d, key_cipher_type = %d, key_len = %d, key_flags = %d", __FUNCTION__, __LINE__, key->key_cipher_type, key->key_len, key->key_flags);
    
    // 80211X认证 catalina    key_cipher_type = 6 key_len = 32
    // 80211X认证 bigsur      key_cipher_type = 9 key_len = 64
    
    struct ieee80211_keyavail keyavail;

//    if (this->cipher_key.key_len > IEEE80211_PMK_LEN)
//        return kIOReturnError;

    memset(&keyavail, 0, sizeof(keyavail));
    strlcpy(keyavail.i_name, this->pa->dev.dv_xname, sizeof(keyavail.i_name));

    struct apple80211_bssid_data bssid;
    if (this->getBSSID(interface, &bssid) == kIOReturnError)
        return kIOReturnError;

    bcopy(bssid.bssid.octet, keyavail.i_macaddr, APPLE80211_ADDR_LEN);
    bcopy(this->cipher_key.key, keyavail.i_key, APPLE80211_KEY_BUFF_LEN);

    if (_ifp->if_ioctl(_ifp, SIOCS80211KEYAVAIL, (caddr_t)&keyavail) < 0)
        return kIOReturnError;

//    _ifp->iface->postMessage(APPLE80211_M_RSN_HANDSHAKE_DONE);
    return kIOReturnSuccess;
}

//
// MARK: 4 - CHANNEL
//

IOReturn AirPortOpenBSD::getCHANNEL(IOInterface *interface, struct apple80211_channel_data *cd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    memset(cd, 0, sizeof(apple80211_channel_data));
    cd->version = APPLE80211_VERSION;
    cd->channel.version = APPLE80211_VERSION;
    cd->channel.channel = ieee80211_chan2ieee(ic, ic->ic_bss->ni_chan);
    cd->channel.flags = chanspec2applechannel(ic->ic_bss->ni_chan->ic_flags, ic->ic_bss->ni_chan->ic_xflags);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setCHANNEL(IOInterface *interface, struct apple80211_channel_data *data)
{
    DebugLog("%s channel=%d\n", __FUNCTION__, data->channel.channel);
    return kIOReturnSuccess;
}

//
// MARK: 5 - POWERSAVE
//

IOReturn AirPortOpenBSD::getPOWERSAVE(IOInterface* interface, struct apple80211_powersave_data* pd)
{
    pd->version = APPLE80211_VERSION;
    pd->powersave_level = APPLE80211_POWERSAVE_MODE_DISABLED;
    return kIOReturnSuccess;
}


//
// MARK: 6 - PROTMODE
//

IOReturn AirPortOpenBSD::getPROTMODE(IOInterface* interface, struct apple80211_protmode_data* pd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN)
        return kIOReturnError;
    memset(pd, 0, sizeof(*pd));
    pd->version = APPLE80211_VERSION;
    pd->protmode = APPLE80211_PROTMODE_AUTO;
    pd->threshold = ic->ic_rtsthreshold;
    return kIOReturnSuccess;
}


//
// MARK: 7 - TXPOWER
//

IOReturn AirPortOpenBSD::getTXPOWER(IOInterface *interface, struct apple80211_txpower_data *txd)
{
    struct ieee80211_txpower power;
    if (_ifp->if_ioctl(_ifp, SIOCG80211TXPOWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    txd->version = APPLE80211_VERSION;
    txd->txpower = power.i_val;
    txd->txpower_unit = APPLE80211_UNIT_PERCENT;
    return kIOReturnSuccess;
}


//IOReturn AirPortOpenBSD::setTXPOWER(IOInterface *interface,
//                                       struct apple80211_txpower_data *txd) {
//
//    struct ieee80211_txpower power = {NULL, 0, txd->txpower};
//    if (_ifp->if_ioctl(_ifp, SIOCS80211TXPOWER, (caddr_t)&power) != 0)
//        return kIOReturnError;
//
//    return kIOReturnSuccess;
//}

//
// MARK: 8 - RATE
//

IOReturn AirPortOpenBSD::getRATE(IOInterface *interface, struct apple80211_rate_data *rate_data)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    
    if (_ifp->tx_tap == NULL)
        return kIOReturnError;
    
    struct tx_radiotap_header *tap =(struct tx_radiotap_header *)_ifp->tx_tap;
    
    if (tap->wt_rate == 0) {
        if (this->noise_data.version == APPLE80211_VERSION) {
            *rate_data = this->rate_data;
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->rate_data.version = APPLE80211_VERSION;
    this->rate_data.num_radios = 1;
    this->rate_data.rate[0] = tap->wt_rate;
    
    *rate_data = this->rate_data;
    
    return kIOReturnSuccess;
}

//
// MARK: 9 - BSSID
//

IOReturn AirPortOpenBSD::getBSSID(IOInterface *interface, struct apple80211_bssid_data *bd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    memset(bd, 0, sizeof(*bd));
    bd->version = APPLE80211_VERSION;
    bcopy(ic->ic_bss->ni_bssid, bd->bssid.octet, APPLE80211_ADDR_LEN);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setBSSID(IOInterface *interface, struct apple80211_bssid_data *bd)
{
    DebugLog("%s bssid=%s\n", __FUNCTION__, ether_sprintf(bd->bssid.octet));
    return kIOReturnSuccess;
}

//
// MARK: 10 - SCAN_REQ
//

IOReturn AirPortOpenBSD::setSCAN_REQ(IOInterface *interface, struct apple80211_scan_data *sd)
{
//    IOLog("--%s: line = %d Scan requested. Type: %u\n"
//          "BSS Type: %u\n"
//          "PHY Mode: %u\n"
//          "Dwell time: %u\n"
//          "Rest time: %u\n"
//          "Num channels: %u\n"
//          "BSSID: %s\n"
//          "SSID: %s\n",
//          __FUNCTION__,
//          __LINE__,
//          sd->scan_type,
//          sd->bss_type,
//          sd->phy_mode,
//          sd->dwell_time,
//          sd->rest_time,
//          sd->num_channels,
//          ether_sprintf(sd->bssid.octet),
//          sd->ssid);
//
    
    if (!(_ifp->if_flags & IFF_RUNNING)) {
        return 0x16;
    }
    
    if (this->scanFlag) {
        return 0x16;
    }
    this->scanFlag = true;
    
    bcopy(sd, &this->scanRequest, sizeof(struct apple80211_scan_data));
    
    if (this->fTimerEventSource) {
        this->fTimerEventSource->setAction(&AirPortOpenBSD::scanDone);
        this->fTimerEventSource->setTimeoutMS(200);
    }

    return kIOReturnSuccess;
}

//
// MARK: 86 - SCAN_REQ_MULTIPLE
//

IOReturn AirPortOpenBSD::setSCAN_REQ_MULTIPLE(IOInterface *interface, struct apple80211_scan_multiple_data *smd)
{
    if (!(_ifp->if_flags & IFF_RUNNING)) {
        return 0x16;
    }
    
    if (this->scanFlag) {
        return 0x16;
    }
    this->scanFlag = true;
    
    bcopy(smd, &this->scanMultiRequest, sizeof(struct apple80211_scan_multiple_data));
    
    if (this->fTimerEventSource) {
        this->fTimerEventSource->setAction(&AirPortOpenBSD::scanDone);
        this->fTimerEventSource->setTimeoutMS(200);
    }

    return kIOReturnSuccess;
}

//
// MARK: 11 - SCAN_RESULT
//

IOReturn AirPortOpenBSD::getSCAN_RESULT(IOInterface *interface, struct apple80211_scan_result **sr)
{
    if (scanResults->getCount() == 0) {
        this->scanFreeResults();
        this->scanFlag = false;
        return 0x0C;
    }
    while (this->scanIndex < scanResults->getCount()) {
        OSObject* scanObj = scanResults->getObject(this->scanIndex++);
        if (scanObj == NULL) {
            continue;
        }
        
        OSData* scanresult = OSDynamicCast(OSData, scanObj);
        struct ieee80211_nodereq *na_node = (struct ieee80211_nodereq *)scanresult->getBytesNoCopy();
        
        struct apple80211_scan_result* oneResult =
            (struct apple80211_scan_result*)IOMalloc(sizeof(struct apple80211_scan_result));
        scanConvertResult(na_node, oneResult);

        *sr = oneResult;
        return kIOReturnSuccess;
    }
    
    this->scanFreeResults();
    this->scanFlag = false;
    
    return 0x05;
}

//
// MARK: 12 - CARD_CAPABILITIES
//

IOReturn AirPortOpenBSD::getCARD_CAPABILITIES(IOInterface *interface, struct apple80211_capability_data *cd)
{
    cd->version = APPLE80211_VERSION;

    struct ieee80211com *ic = (struct ieee80211com *)_ifp;

    u_int32_t caps = 0;

    caps |=  1 << APPLE80211_CAP_AES;
    caps |=  1 << APPLE80211_CAP_AES_CCM;

    if (ic->ic_caps & IEEE80211_C_WEP)              caps |= 1 << APPLE80211_CAP_WEP;
    if (ic->ic_caps & IEEE80211_C_RSN) {
        caps |=  1 << APPLE80211_CAP_TKIP;
        caps |=  1 << APPLE80211_CAP_TKIPMIC;
        caps |=  1 << APPLE80211_CAP_WPA;
        caps |=  1 << APPLE80211_CAP_WPA1;
        caps |=  1 << APPLE80211_CAP_WPA2;
    }
    if (ic->ic_caps & IEEE80211_C_MONITOR)          caps |= 1 << APPLE80211_CAP_MONITOR;
    if (ic->ic_caps & IEEE80211_C_SHSLOT)           caps |= 1 << APPLE80211_CAP_SHSLOT;
    if (ic->ic_caps & IEEE80211_C_SHPREAMBLE)       caps |= 1 << APPLE80211_CAP_SHPREAMBLE;
//    if (ic->ic_caps & IEEE80211_C_AHDEMO)           caps |= 1 << APPLE80211_CAP_IBSS;
    if (ic->ic_caps & IEEE80211_C_PMGT)             caps |= 1 << APPLE80211_CAP_PMGT;
    if (ic->ic_caps & IEEE80211_C_TXPMGT)           caps |= 1 << APPLE80211_CAP_TXPMGT;
//    if (ic->ic_caps & IEEE80211_C_QOS)              caps |= 1 << APPLE80211_CAP_WME;

    cd->capabilities[0] = (caps & 0xff);
    cd->capabilities[1] = (caps >> 8) & 0xff;
    
//    cd->capabilities[2] |= 0x13;    // 无线网络唤醒;
//    cd->capabilities[2] |= 0xc0;    // 批量扫描;
//    cd->capabilities[4] |= 0x1;     // 隔空投送
    
//    cd->capabilities[0] |= 0xab;
//    cd->capabilities[1] |= 0x7e;
    cd->capabilities[2] |= 0x3 | 0x4 | 0x13 | 0x20 | 0x28| 0x80 | 0xc0;
    cd->capabilities[3] |= 0x2 | 0x23;
//    cd->capabilities[4] |= 0x1;
    cd->capabilities[5] |= 0x80;
    cd->capabilities[6] |= 0x8D;
    cd->capabilities[7] = 0x84;

    return kIOReturnSuccess;
    
}

//
// MARK: 13 - STATE
//

IOReturn AirPortOpenBSD::getSTATE(IOInterface *interface,
                                     struct apple80211_state_data *ret)
{

    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    DebugLog("--%s: line = %d ic->ic_state = %d", __FUNCTION__, __LINE__, ic->ic_state);
    ret->version = APPLE80211_VERSION;
    ret->state = ic->ic_state;
    return kIOReturnSuccess;
}

//
// MARK: 14 - PHY_MODE
//

IOReturn AirPortOpenBSD::getPHY_MODE(IOInterface *interface,
                                        struct apple80211_phymode_data *pd) {
    pd->version = APPLE80211_VERSION;
    
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    u_int32_t ic_modecaps= ic->ic_modecaps;
    u_int32_t phy_mode  = APPLE80211_MODE_UNKNOWN;
    
    if (ic_modecaps & (1<<IEEE80211_MODE_AUTO))       phy_mode |= APPLE80211_MODE_AUTO;
    if (ic_modecaps & (1<<IEEE80211_MODE_11A))        phy_mode |= APPLE80211_MODE_11A;
    if (ic_modecaps & (1<<IEEE80211_MODE_11B))        phy_mode |= APPLE80211_MODE_11B;
    if (ic_modecaps & (1<<IEEE80211_MODE_11G))        phy_mode |= APPLE80211_MODE_11G;
    if (ic_modecaps & (1<<IEEE80211_MODE_11N))        phy_mode |= APPLE80211_MODE_11N;
    if (ic_modecaps & (1<<IEEE80211_MODE_11AC))       phy_mode |= APPLE80211_MODE_11AC;
    
    u_int32_t ic_curmode= ic->ic_curmode;
    u_int32_t active_phy_mode  = APPLE80211_MODE_UNKNOWN;
    
    switch (ic_curmode) {
        case IEEE80211_MODE_11A:
            active_phy_mode = APPLE80211_MODE_11A;
            break;
        case IEEE80211_MODE_11B:
            active_phy_mode = APPLE80211_MODE_11B;
            break;
        case IEEE80211_MODE_11G:
            active_phy_mode = APPLE80211_MODE_11G;
            break;
        case IEEE80211_MODE_11N:
            active_phy_mode = APPLE80211_MODE_11N;
            break;
        case IEEE80211_MODE_11AC:
            active_phy_mode = APPLE80211_MODE_11AC;
            break;
        default:
            active_phy_mode = APPLE80211_MODE_AUTO;
            break;
    }
    
    pd->phy_mode = phy_mode;
    pd->active_phy_mode = active_phy_mode;
    return kIOReturnSuccess;
}

//
// MARK: 15 - OP_MODE
//

IOReturn AirPortOpenBSD::getOP_MODE(IOInterface *interface,
                                       struct apple80211_opmode_data *od) {
    uint32_t op_mode = 0;
    
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    switch (ic->ic_opmode) {
        case IEEE80211_M_STA:
            op_mode = APPLE80211_M_STA;
        case IEEE80211_M_IBSS:
            op_mode =  APPLE80211_M_IBSS;
        case IEEE80211_M_AHDEMO:
            op_mode =  APPLE80211_M_AHDEMO;
        case IEEE80211_M_HOSTAP:
            op_mode =  APPLE80211_M_HOSTAP;
        case IEEE80211_M_MONITOR:
            op_mode =  APPLE80211_M_MONITOR;
        default:
            op_mode =  APPLE80211_M_NONE;
    }
    
    od->version = APPLE80211_VERSION;
    od->op_mode = op_mode;
    return kIOReturnSuccess;
}

//
// MARK: 16 - RSSI
//

IOReturn AirPortOpenBSD::getRSSI(IOInterface *interface,
                                    struct apple80211_rssi_data *rd_data)
{
    if (_ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)_ifp->rx_tap;
    
    if (tap->wr_dbm_antsignal == 0) {
        if (this->rssi_data.version == APPLE80211_VERSION) {
            *rd_data = this->rssi_data;
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->rssi_data.version = APPLE80211_VERSION;
    this->rssi_data.num_radios = 1;
    this->rssi_data.rssi_unit = APPLE80211_UNIT_DBM;
    this->rssi_data.rssi[0]
    = this->rssi_data.aggregate_rssi
    = this->rssi_data.rssi_ext[0]
    = this->rssi_data.aggregate_rssi_ext
    = tap->wr_dbm_antsignal - 100;
    
    *rd_data = this->rssi_data;
    
    return kIOReturnSuccess;
}

//
// MARK: 17 - NOISE
//

IOReturn AirPortOpenBSD::getNOISE(IOInterface *interface,
                                     struct apple80211_noise_data *nd_data)
{
    if (_ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)_ifp->rx_tap;
    
    if (tap->wr_dbm_antnoise == 0) {
        if (this->noise_data.version == APPLE80211_VERSION) {
            *nd_data = this->noise_data;
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->noise_data.version = APPLE80211_VERSION;
    this->noise_data.noise_unit = APPLE80211_UNIT_DBM;
    this->noise_data.num_radios = 1;
    this->noise_data.noise[0]
    = this->noise_data.aggregate_noise
    = this->noise_data.noise_ext[0]
    = this->noise_data.aggregate_noise_ext
    = tap->wr_dbm_antnoise;
    
    *nd_data = this->noise_data;
    return kIOReturnSuccess;
}

//
// MARK: 18 - INT_MIT
//

IOReturn AirPortOpenBSD::getINT_MIT(IOInterface* interface,
                                       struct apple80211_intmit_data* imd) {
    imd->version = APPLE80211_VERSION;
    imd->int_mit = APPLE80211_INT_MIT_AUTO;
    return kIOReturnSuccess;
}


//
// MARK: 19 - POWER
//

IOReturn AirPortOpenBSD::getPOWER(IOInterface *interface,
                                     struct apple80211_power_data *ret) {

    ret->version = APPLE80211_VERSION;
    ret->num_radios = 1;
    ret->power_state[0] = _ifp->if_power_state;

    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setPOWER(IOInterface *interface,
                                     struct apple80211_power_data *pd) {
    IOReturn ret = kIOReturnSuccess;

    if (pd->num_radios > 0) {
        _ifp->if_power_state = pd->power_state[0];
    }
    
    ret = this->changePowerState(interface, _ifp->if_power_state);

    return ret;
}


//
// MARK: 20 - ASSOCIATE
//

IOReturn AirPortOpenBSD::setASSOCIATE(IOInterface *interface, struct apple80211_assoc_data *ad)
{
    
    DebugLog("--%s: line = %d ssid = %s, bssid = %s", __FUNCTION__, __LINE__, ad->ad_ssid,
    ether_sprintf(ad->ad_bssid.octet));
    
    DebugLog("--%s: line = %d, ad_mode = %d, ad_rsn_ie_len = %d", __FUNCTION__, __LINE__, ad->ad_mode, ad->ad_rsn_ie_len);
    
    DebugLog("--%s: line = %d ad_auth_lower = %d, ad_auth_upper = %d, key_cipher_type = %d", __FUNCTION__, __LINE__, ad->ad_auth_lower, ad->ad_auth_upper, ad->ad_key.key_cipher_type);
    
    if (ad->ad_mode != 1) {
        if (!(ad->ad_ssid_len == this->assoc_data.ad_ssid_len && memcmp(ad->ad_ssid, this->assoc_data.ad_ssid, ad->ad_ssid_len) == 0 && ad->ad_key.key_len == this->assoc_data.ad_key.key_len && memcmp(ad->ad_key.key, this->assoc_data.ad_key.key, ad->ad_key.key_len) == 0)) {
            this->assoc_data = *ad;

            apple80211_authtype_data _authtype_data;
            _authtype_data.version = APPLE80211_VERSION;
            _authtype_data.authtype_lower = ad->ad_auth_lower;
            _authtype_data.authtype_upper = ad->ad_auth_upper;
            this->setAUTH_TYPE(interface, &_authtype_data);
            
            // OPEN认证           ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 0
            // WEP认证，开放系统    ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 1
            // WEP认证，共享密钥    ad_auth_lower = 2, ad_auth_upper = 0, key_cipher_type = 2
            // WPA认证            ad_auth_lower = 1, ad_auth_upper = 2, key_cipher_type = 6
            // WPA2认证           ad_auth_lower = 1, ad_auth_upper = 8, key_cipher_type = 6
            // 80211X认证         ad_auth_lower = 1, ad_auth_upper = 4, key_cipher_type = 0
            
            this->configArrCount = 0;
            this->configArr[0] = "nwid";
            this->configArr[1] = (const char *)ad->ad_ssid;
            this->configArrCount += 2;
            
            get_hexstring(ad->ad_key.key, i_psk, ad->ad_key.key_len);
            
            switch (ad->ad_auth_lower) {
                case APPLE80211_AUTHTYPE_OPEN:
                    
                    switch (ad->ad_auth_upper) {
                        case APPLE80211_AUTHTYPE_NONE:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_NONE:
                                    // OPEN认证
                                    break;
                                case APPLE80211_CIPHER_WEP_40:
                                case APPLE80211_CIPHER_WEP_104:
                                    // WEP认证，开放系统
                                    this->configArr[2] = "nwkey";
                                    this->configArr[3] = i_psk;
                                    this->configArrCount += 2;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA:
                        case APPLE80211_AUTHTYPE_WPA2:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_NONE:
                                    // 企业认证
                                    this->configArr[2] = "wpa";
                                    this->configArr[3] = "wpaakms";
                                    this->configArr[4] = "802.1x";
                                    this->configArr[5] = "wpaprotos";
                                    this->configArr[6] = "wpa1,wpa2";
                                    this->configArrCount += 5;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA_PSK:
                        case APPLE80211_AUTHTYPE_WPA2_PSK:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_TKIP:
                                case APPLE80211_CIPHER_PMK:
                                    // WPA/WPA2认证
                                    snprintf(key_tmp, sizeof(key_tmp), "0x%s", i_psk);
                                    this->configArr[2] = "wpakey";
                                    this->configArr[3] = key_tmp;
                                    this->configArr[4] = "wpaprotos";
                                    this->configArr[5] = "wpa1,wpa2";
                                    this->configArrCount += 4;
                                    break;
                                default:
                                    break;
                            }
                            break;

                            break;
                        default:
                            DebugLog("--%s: line = %d ad_auth_upper = %d", __FUNCTION__, __LINE__, ad->ad_auth_upper);
                            break;
                    }
                    
                    break;
                case APPLE80211_AUTHTYPE_SHARED:
                    
                    switch (ad->ad_auth_upper) {
                        case APPLE80211_AUTHTYPE_NONE:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_WEP_40:
                                case APPLE80211_CIPHER_WEP_104:
                                    // WEP认证，共享密钥
                                    this->configArr[2] = "nwkey";
                                    this->configArr[3] = i_psk;
                                    this->configArrCount += 2;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    
                    break;
                default:
                    break;
            }
            
            ifconfig(this->configArr, this->configArrCount);
            
            DebugLog("--%s: line = %d ad_auth_lower = %d, ad_auth_upper = %d, key_cipher_type = %d", __FUNCTION__, __LINE__, ad->ad_auth_lower, ad->ad_auth_upper, ad->ad_key.key_cipher_type);
        }
    
        try_times = 7;
        this->disassoc_times = false;

        struct ieee80211com *ic = (struct ieee80211com *)_ifp;
        while (!(this->isConnected() || this->isRun80211X()) && this->try_times-- > 0) {
            tsleep_nsec(&ic->ic_state, PCATCH, "getASSOCIATION_STATUS",
                SEC_TO_NSEC(1));
        };
    }
    
    return kIOReturnSuccess;
}

//
// MARK: 21 - ASSOCIATE_RESULT
//

IOReturn AirPortOpenBSD::getASSOCIATE_RESULT(IOInterface* interface, struct apple80211_assoc_result_data *ad)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    memset(ad, 0, sizeof(struct apple80211_assoc_result_data));
    ad->version = APPLE80211_VERSION;
    ad->result = APPLE80211_RESULT_SUCCESS;
    return kIOReturnSuccess;
}

//
// MARK: 22 - DISASSOCIATE
//

IOReturn AirPortOpenBSD::setDISASSOCIATE(IOInterface* interface)
{
    if (this->disassoc_times) {
        DebugLog("--%s: line = %d", __FUNCTION__, __LINE__);
        
        this->configArr[0] = "-nwid";
        this->configArr[1] = (const char *)this->assoc_data.ad_ssid;
        this->configArrCount = 2;
        ifconfig(this->configArr, this->configArrCount);
        
        bzero(&this->assoc_data, sizeof(this->assoc_data));
        this->disassoc_times = false;
        
        return kIOReturnSuccess;
    }
    
    this->disassoc_times = true;
    
//    _ifp->iface->postMessage(APPLE80211_M_SSID_CHANGED);
    
    return kIOReturnSuccess;
}

//
// MARK: 27 - SUPPORTED_CHANNELS
//

IOReturn AirPortOpenBSD::getSUPPORTED_CHANNELS(IOInterface *interface,
                                                  struct apple80211_sup_channel_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->num_channels = 0;
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    for (int i = 0; i < IEEE80211_CHAN_MAX && ad->num_channels < APPLE80211_MAX_CHANNELS; i++) {
        if (ic->ic_channels[i].ic_freq != 0) {
            ad->supported_channels[ad->num_channels].channel = ieee80211_chan2ieee(ic, &ic->ic_channels[i]);
            ad->supported_channels[ad->num_channels].flags   = chanspec2applechannel(ic->ic_channels[i].ic_flags, ic->ic_channels[i].ic_xflags);
            ad->supported_channels[ad->num_channels].version = APPLE80211_VERSION;
            ad->num_channels++;
        }
    }
    return kIOReturnSuccess;
}

//
// MARK: 28 - LOCALE
//

IOReturn AirPortOpenBSD::getLOCALE(IOInterface *interface, struct apple80211_locale_data *ld)
{
    ld->version = APPLE80211_VERSION;
    ld->locale  = APPLE80211_LOCALE_ROW;
    return kIOReturnSuccess;
}

//
// MARK: 29 - DEAUTH
//

IOReturn AirPortOpenBSD::getDEAUTH(IOInterface *interface, struct apple80211_deauth_data *dd)
{
    DebugLog("--%s: line = %d", __FUNCTION__, __LINE__);
    dd->version = APPLE80211_VERSION;
    dd->deauth_reason = APPLE80211_REASON_UNSPECIFIED;
    return kIOReturnSuccess;
}

//
// MARK: 32 - RATE_SET
//

IOReturn AirPortOpenBSD::getRATE_SET(IOInterface *interface, struct apple80211_rate_set_data *ad)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state == IEEE80211_S_RUN) {
        memset(ad, 0, sizeof(*ad));
        ad->version = APPLE80211_VERSION;
        ad->num_rates = ic->ic_bss->ni_rates.rs_nrates;
        size_t size = min(ic->ic_bss->ni_rates.rs_nrates, nitems(ad->rates));
        for (int i=0; i < size; i++) {
            struct apple80211_rate apple_rate = ad->rates[i];
            apple_rate.version = APPLE80211_VERSION;
            apple_rate.rate = ic->ic_bss->ni_rates.rs_rates[i];
            apple_rate.flags = 0;
        }
        return kIOReturnSuccess;
    }
    return kIOReturnError;
}

//
// MARK: 37 - TX_ANTENNA
//
IOReturn AirPortOpenBSD::getTX_ANTENNA(IOInterface *interface, apple80211_antenna_data *ad)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    ad->version = APPLE80211_VERSION;
    ad->num_radios = 1;
    ad->antenna_index[0] = 1;
    return kIOReturnSuccess;
}

//
// MARK: 39 - ANTENNA_DIVERSITY
//

IOReturn AirPortOpenBSD::getANTENNA_DIVERSITY(IOInterface *interface,
                                                 apple80211_antenna_data *ad)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    ad->version = APPLE80211_VERSION;
    ad->num_radios = 1;
    ad->antenna_index[0] = 1;
    return kIOReturnSuccess;
}


//
// MARK: 43 - DRIVER_VERSION
//

IOReturn AirPortOpenBSD::getDRIVER_VERSION(IOInterface *interface, struct apple80211_version_data *hv)
{
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, _ifp->fwver, sizeof(hv->string));
    hv->string_len = strlen(_ifp->fwver);
    return kIOReturnSuccess;
}

//
// MARK: 44 - HARDWARE_VERSION
//

IOReturn AirPortOpenBSD::getHARDWARE_VERSION(IOInterface *interface, struct apple80211_version_data *hv)
{
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, "Hardware 1.0", sizeof(hv->string));
    hv->string_len = strlen("Hardware 1.0");
    return kIOReturnSuccess;
}

//
// MARK: 46 - RSN_IE
//

IOReturn AirPortOpenBSD::getRSN_IE(IOInterface *interface, struct apple80211_rsn_ie_data *rid)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_bss == NULL || ic->ic_bss->ni_rsnie == NULL)
        return kIOReturnError;
    rid->version = APPLE80211_VERSION;
    rid->len = 2 + ic->ic_bss->ni_rsnie[1];
    memcpy(rid->ie, ic->ic_bss->ni_rsnie, rid->len);
    return kIOReturnSuccess;
}

//
// MARK: 48 - AP_IE_LIST
//

IOReturn AirPortOpenBSD::getAP_IE_LIST(IOInterface *interface, struct apple80211_ap_ie_data *data)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_bss == NULL || ic->ic_bss->ni_ie == NULL) {
        return kIOReturnError;
    }
    data->version = APPLE80211_VERSION;
    data->len = ic->ic_bss->ni_ie_len;
    memcpy(data->ie_data, ic->ic_bss->ni_ie, data->len);
    return kIOReturnSuccess;
}

//
// MARK: 50 - ASSOCIATION_STATUS
//

IOReturn AirPortOpenBSD::getASSOCIATION_STATUS(IOInterface* interface, struct apple80211_assoc_status_data* ret)
{
    ret->status = APPLE80211_STATUS_UNSPECIFIED_FAILURE;
    
    if (this->isRun80211X()) {
        struct ieee80211com *ic = (struct ieee80211com *)_ifp;
        ieee80211_set_link_state(ic, LINK_STATE_UP);
    
        ret->status = APPLE80211_STATUS_SUCCESS;
        this->disassoc_times = true;
    } else if (this->isConnected()) {
        ret->status = APPLE80211_STATUS_SUCCESS;
        this->disassoc_times = true;
    }

    DebugLog("--%s: line = %d asd.status = %d", __FUNCTION__, __LINE__, ret->status);

    ret->version = APPLE80211_VERSION;

    return kIOReturnSuccess;
}

//
// MARK: 51 - COUNTRY_CODE
//

IOReturn AirPortOpenBSD::getCOUNTRY_CODE(IOInterface *interface, struct apple80211_country_code_data *ccd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL || ic->ic_bss->ni_countryie == NULL) {
        *ccd = this->ccd;
        return kIOReturnSuccess;
    }
    
    ccd->version = APPLE80211_VERSION;
    bcopy(&ic->ic_bss->ni_countryie[2], ccd->cc, APPLE80211_MAX_CC_LEN);
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setCOUNTRY_CODE(IOInterface *interface, struct apple80211_country_code_data *ccd)
{
    DebugLog("--%s: line = %d, COUNTRY_CODE = %s", __FUNCTION__, __LINE__, ccd->cc);
    this->ccd = *ccd;
    return kIOReturnSuccess;
}

//
// MARK: 54 - RADIO_INFO
//

IOReturn AirPortOpenBSD::getRADIO_INFO(IOInterface* interface, struct apple80211_radio_info_data* md)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    md->version = APPLE80211_VERSION;
    md->count = 1;
    return kIOReturnSuccess;
}

//
// MARK: 57 - MCS
//
IOReturn AirPortOpenBSD::getMCS(IOInterface* interface, struct apple80211_mcs_data* md)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    md->version = APPLE80211_VERSION;
    md->index = ic->ic_bss->ni_txmcs;
    return kIOReturnSuccess;
}

//
// MARK: 66 - MCS_INDEX_SET
//
IOReturn AirPortOpenBSD::getMCS_INDEX_SET(IOInterface* interface, struct apple80211_mcs_index_set_data* md)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    memset(md, 0, sizeof(*md));
    md->version = APPLE80211_VERSION;
    size_t size = min(nitems(ic->ic_bss->ni_rxmcs), nitems(md->mcs_set_map));
    for (int i = 0; i < size; i++) {
        md->mcs_set_map[i] = ic->ic_bss->ni_rxmcs[i];
    }
    return kIOReturnSuccess;
}


//
// MARK: 80 - ROAM_THRESH
//

IOReturn AirPortOpenBSD::getROAM_THRESH(IOInterface* interface, struct apple80211_roam_threshold_data* rtd)
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    if (ic->ic_state != IEEE80211_S_RUN || ic->ic_bss == NULL)
        return kIOReturnError;
    rtd->threshold = 1000;
    rtd->count = 0;
    return kIOReturnSuccess;
}


//
// MARK: 90 - SCANCACHE_CLEAR
//

IOReturn AirPortOpenBSD::setSCANCACHE_CLEAR(IOInterface *interface, struct device *dev)
{
//    DebugLog("--%s: line = %d", __FUNCTION__, __LINE__);
//    scanFreeResults();
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    //if doing background or active scan, don't free nodes.
    if ((ic->ic_flags & IEEE80211_F_BGSCAN) || (ic->ic_flags & IEEE80211_F_ASCAN)) {
        return kIOReturnSuccess;
    }
    ieee80211_free_allnodes(ic, 0);
    return kIOReturnSuccess;
}


//
// MARK: 156 - LINK_CHANGED_EVENT_DATA
//

IOReturn AirPortOpenBSD::getLINK_CHANGED_EVENT_DATA(IOInterface *interface, struct apple80211_link_changed_event_data *ed) {
    if (ed == NULL)
        return 16;

    bzero(ed, sizeof(*ed));
    
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    
    ed->isLinkDown = ic->ic_state!= IEEE80211_S_RUN;
    struct apple80211_rssi_data rd_data;
    getRSSI(interface, &rd_data);
    
    ed->rssi = rd_data.rssi[0];
    if (ed->isLinkDown) {
        ed->voluntary = true;
        ed->reason = APPLE80211_LINK_DOWN_REASON_DEAUTH;
    }
    return kIOReturnSuccess;
}

//
// MARK: 196 - TX_NSS
//

IOReturn AirPortOpenBSD::getTX_NSS(IOInterface *interface, struct apple80211_tx_nss_data *data)
{
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->nss = 1;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setTX_NSS(IOInterface *interface, struct apple80211_tx_nss_data *data)
{
    return kIOReturnError;
}

//
// MARK: 353 - NSS
//

IOReturn AirPortOpenBSD::getNSS(IOInterface *interface, struct apple80211_nss_data *data)
{
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->nss = 1;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::apple80211Request(UInt32 request_type,
                                            int request_number,
                                            IOInterface* interface,
                                            void* data) {

    if (request_type != SIOCGA80211 && request_type != SIOCSA80211) {
        IOLog("AirPortOpenBSD: Invalid IOCTL request type: %u", request_type);
        return kIOReturnError;
    }
    
    IOReturn ret = kIOReturnUnsupported;
    
    bool isGet = (request_type == SIOCGA80211);
    
#define IOCTL(REQ_TYPE, REQ, DATA_TYPE) \
if (REQ_TYPE == SIOCGA80211) { \
ret = get##REQ(interface, (struct DATA_TYPE* )data); \
} else { \
ret = set##REQ(interface, (struct DATA_TYPE* )data); \
}
    
#define IOCTL_GET(REQ_TYPE, REQ, DATA_TYPE) \
if (REQ_TYPE == SIOCGA80211) { \
ret = get##REQ(interface, (struct DATA_TYPE* )data); \
}
#define IOCTL_SET(REQ_TYPE, REQ, DATA_TYPE) \
if (REQ_TYPE == SIOCSA80211) { \
ret = set##REQ(interface, (struct DATA_TYPE* )data); \
}
    
//    switch (request_number) {
//        case 1:
//        case 2:
//        case 4:
//        case 8:
//        case 9:
//        case 10:
//        case 11:
//        case 12:
//        case 14:
//        case 15:
//        case 16:
//        case 17:
//        case 19:
//        case 27:
//        case 28:
//        case 43:
//        case 44:
//        case 51:
//        case 57:
//        case 80:
//        case 254:
//        case 353:
//            break;
//        default:
//    IOLog("--%s: IOCTL %s(%d)", __FUNCTION__,
//          isGet ? "get" : "set",
//          request_number);
////    IOLog("%s: IOCTL %s(%d) %s", __FUNCTION__,
////          isGet ? "get" : "set",
////          request_number,
////          IOCTL_NAMES[request_number]);
//
//    }
    
    switch (request_number) {
        case APPLE80211_IOC_SSID: // 1
            IOCTL(request_type, SSID, apple80211_ssid_data);
            break;
        case APPLE80211_IOC_AUTH_TYPE: // 2
            IOCTL(request_type, AUTH_TYPE, apple80211_authtype_data);
            break;
        case APPLE80211_IOC_CIPHER_KEY: // 3
            IOCTL(request_type, CIPHER_KEY, apple80211_key);
            break;
        case APPLE80211_IOC_CHANNEL: // 4
            IOCTL(request_type, CHANNEL, apple80211_channel_data);
            break;
        case APPLE80211_IOC_POWERSAVE: // 5
            IOCTL_GET(request_type, POWERSAVE, apple80211_powersave_data);
            break;
        case APPLE80211_IOC_PROTMODE: // 6
            IOCTL_GET(request_type, PROTMODE, apple80211_protmode_data);
            break;
        case APPLE80211_IOC_TXPOWER: // 7
            IOCTL_GET(request_type, TXPOWER, apple80211_txpower_data);
            break;
        case APPLE80211_IOC_RATE: // 8
            IOCTL_GET(request_type, RATE, apple80211_rate_data);
            break;
        case APPLE80211_IOC_BSSID: // 9
            IOCTL(request_type, BSSID, apple80211_bssid_data);
            break;
        case APPLE80211_IOC_SCAN_REQ: // 10
            IOCTL_SET(request_type, SCAN_REQ, apple80211_scan_data);
            break;
        case APPLE80211_IOC_SCAN_RESULT: // 11
            IOCTL_GET(request_type, SCAN_RESULT, apple80211_scan_result*);
            break;
        case APPLE80211_IOC_CARD_CAPABILITIES: // 12
            IOCTL_GET(request_type, CARD_CAPABILITIES, apple80211_capability_data);
            break;
        case APPLE80211_IOC_STATE: // 13
            IOCTL_GET(request_type, STATE, apple80211_state_data);
            break;
        case APPLE80211_IOC_PHY_MODE: // 14
            IOCTL_GET(request_type, PHY_MODE, apple80211_phymode_data);
            break;
        case APPLE80211_IOC_OP_MODE: // 15
            IOCTL_GET(request_type, OP_MODE, apple80211_opmode_data);
            break;
        case APPLE80211_IOC_RSSI: // 16
            IOCTL_GET(request_type, RSSI, apple80211_rssi_data);
            break;
        case APPLE80211_IOC_NOISE: // 17
            IOCTL_GET(request_type, NOISE, apple80211_noise_data);
            break;
        case APPLE80211_IOC_INT_MIT: // 18
            IOCTL_GET(request_type, INT_MIT, apple80211_intmit_data);
            break;
        case APPLE80211_IOC_POWER: // 19
            IOCTL(request_type, POWER, apple80211_power_data);
            break;
        case APPLE80211_IOC_ASSOCIATE: // 20
            IOCTL_SET(request_type, ASSOCIATE, apple80211_assoc_data);
            break;
        case APPLE80211_IOC_ASSOCIATE_RESULT: // 21
            IOCTL_GET(request_type, ASSOCIATE_RESULT, apple80211_assoc_result_data);
            break;
        case APPLE80211_IOC_DISASSOCIATE: // 22
            ret = setDISASSOCIATE(interface);
            break;
//        case APPLE80211_IOC_STATUS_DEV_NAME: // 23
//            IOCTL_GET(request_type, STATUS_DEV_NAME, apple80211_status_dev_data);
//            break;
        case APPLE80211_IOC_SUPPORTED_CHANNELS: // 27
        case APPLE80211_IOC_HW_SUPPORTED_CHANNELS: // 254
            IOCTL_GET(request_type, SUPPORTED_CHANNELS, apple80211_sup_channel_data);
            break;
        case APPLE80211_IOC_LOCALE: // 28
            IOCTL_GET(request_type, LOCALE, apple80211_locale_data);
            break;
        case APPLE80211_IOC_DEAUTH: // 29
            IOCTL_GET(request_type, DEAUTH, apple80211_deauth_data);
            break;
//        case APPLE80211_IOC_FRAG_THRESHOLD: // 31
//            IOCTL_GET(request_type, FRAG_THRESHOLD, apple80211_frag_threshold_data);
//            break;
        case APPLE80211_IOC_RATE_SET: // 32
            IOCTL_GET(request_type, RATE_SET, apple80211_rate_set_data);
            break;
        case APPLE80211_IOC_TX_ANTENNA: // 37
            IOCTL_GET(request_type, TX_ANTENNA, apple80211_antenna_data);
            break;
        case APPLE80211_IOC_ANTENNA_DIVERSITY: // 39
            IOCTL_GET(request_type, ANTENNA_DIVERSITY, apple80211_antenna_data);
            break;
//        case APPLE80211_IOC_ROM:
//            IOCTL_GET(request_type, ROM, apple80211_rom_data);
//            break;
        case APPLE80211_IOC_DRIVER_VERSION: // 43
            IOCTL_GET(request_type, DRIVER_VERSION, apple80211_version_data);
            break;
        case APPLE80211_IOC_HARDWARE_VERSION: // 44
            IOCTL_GET(request_type, HARDWARE_VERSION, apple80211_version_data);
            break;
        case APPLE80211_IOC_RSN_IE: // 46
            IOCTL_GET(request_type, RSN_IE, apple80211_rsn_ie_data);
            break;
        case APPLE80211_IOC_AP_IE_LIST: // 48
            IOCTL_GET(request_type, AP_IE_LIST, apple80211_ap_ie_data);
            break;
        case APPLE80211_IOC_ASSOCIATION_STATUS: // 50
            IOCTL_GET(request_type, ASSOCIATION_STATUS, apple80211_assoc_status_data);
            break;
        case APPLE80211_IOC_COUNTRY_CODE: // 51
            IOCTL(request_type, COUNTRY_CODE, apple80211_country_code_data);
            break;
//        case APPLE80211_IOC_LAST_RX_PKT_DATA: // 53
//            IOCTL_GET(request_type, LAST_RX_PKT_DATA, apple80211_last_rx_pkt_data);
//            break;
        case APPLE80211_IOC_RADIO_INFO: // 54
            IOCTL_GET(request_type, RADIO_INFO, apple80211_radio_info_data);
            break;
        case APPLE80211_IOC_MCS: // 57
            IOCTL_GET(request_type, MCS, apple80211_mcs_data);
            break;
        case APPLE80211_IOC_MCS_INDEX_SET: // 66
            IOCTL_GET(request_type, MCS_INDEX_SET, apple80211_mcs_index_set_data);
            break;
//        case APPLE80211_IOC_WOW_PARAMETERS: // 69
//            break;
        case APPLE80211_IOC_ROAM_THRESH: // 80
            IOCTL_GET(request_type, ROAM_THRESH, apple80211_roam_threshold_data);
            break;
//        case APPLE80211_IOC_IE: // 85
//            IOCTL_GET(request_type, IE, apple80211_rsn_ie_data);
//            break;
        case APPLE80211_IOC_SCAN_REQ_MULTIPLE: // 86
            IOCTL_SET(request_type, SCAN_REQ_MULTIPLE, apple80211_scan_multiple_data);
            break;
        case APPLE80211_IOC_SCANCACHE_CLEAR: // 90
            IOCTL_SET(request_type, SCANCACHE_CLEAR, device);
            break;
//        case APPLE80211_IOC_TX_CHAIN_POWER: // 108
//            break;
//        case APPLE80211_IOC_THERMAL_THROTTLING: // 111
//            break;
//        case APPLE80211_IOC_FACTORY_MODE: // 112
//            IOCTL_GET(request_type, FACTORY_MODE, apple80211_factory_mode_data);
//            break;
        case APPLE80211_IOC_LINK_CHANGED_EVENT_DATA: // 156
            IOCTL_GET(request_type, LINK_CHANGED_EVENT_DATA, apple80211_link_changed_event_data);
            break;
        case APPLE80211_IOC_TX_NSS: // 196
            IOCTL(request_type, TX_NSS, apple80211_tx_nss_data);
            break;
        case APPLE80211_IOC_NSS: // 353
            IOCTL_GET(request_type, NSS, apple80211_nss_data);
            break;
        default:
//            DPRINTF(("Unhandled Airport GET request %u\n", request_number));
            ret = kIOReturnUnsupported;
    }
#undef IOCTL
    
    return ret;
}
