//
//  AirPortOpenBSD_SkywalkInterface.cpp
//  AirPortOpenBSD
//
//  Created by qcwap on 2023/6/27.
//  Copyright © 2023 钟先耀. All rights reserved.
//
#include "AirPortOpenBSD.hpp"
#include "AirPortOpenBSD_SkywalkInterface.hpp"
#include <crypto/sha1.h>
#include <net80211/ieee80211_priv.h>
#include <net80211/ieee80211_var.h>

#define super IO80211InfraProtocol
OSDefineMetaClassAndStructors(AirPortOpenBSD_SkywalkInterface, IO80211InfraProtocol);

bool AirPortOpenBSD_SkywalkInterface::
init(IOService *provider)
{
    bool ret = IO80211InfraInterface::init();
    if (!ret) {
        return false;
    }
    instance = OSDynamicCast(AirPortOpenBSD, provider);
    if (!instance)
        return false;
    
    return ret;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getSSID(struct apple80211_ssid_data *sd)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    
    struct ifnet *ifp = &this->instance->ic->ic_if;
    struct ifreq ifr;
    struct ieee80211_nwid nwid;
    
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_data = (caddr_t)&nwid;
    
    if (ifp->if_ioctl(ifp, SIOCG80211NWID, (caddr_t)&ifr) == -1)
        return kIOReturnError;
    
    memset(sd, 0, sizeof(*sd));
    sd->version = APPLE80211_VERSION;
    sd->ssid_len = nwid.i_len;
    memcpy(sd->ssid, nwid.i_nwid, sd->ssid_len);
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getAUTH_TYPE(struct apple80211_authtype_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->authtype_lower = this->instance->current_authtype_lower;
    ad->authtype_upper = this->instance->current_authtype_upper;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setAUTH_TYPE(struct apple80211_authtype_data *ad)
{
    this->instance->current_authtype_lower = ad->authtype_lower;
    this->instance->current_authtype_upper = ad->authtype_upper;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::setCIPHER_KEY(struct apple80211_key *key)
{
    DebugLog("%s\n", __FUNCTION__);
    const char* keydump = hexdump(key->key, key->key_len);
    const char* rscdump = hexdump(key->key_rsc, key->key_rsc_len);
    const char* eadump = hexdump(key->key_ea.octet, APPLE80211_ADDR_LEN);
    static_assert(__offsetof(struct apple80211_key, key_ea) == 92, "struct corrupted");
    static_assert(__offsetof(struct apple80211_key, key_rsc_len) == 80, "struct corrupted");
    static_assert(__offsetof(struct apple80211_key, wowl_kck_len) == 100, "struct corrupted");
    static_assert(__offsetof(struct apple80211_key, wowl_kek_len) == 120, "struct corrupted");
    static_assert(__offsetof(struct apple80211_key, wowl_kck_key) == 104, "struct corrupted");
    if (keydump && rscdump && eadump) {
        DebugLog("Set key request: len=%d cipher_type=%d flags=%d index=%d key=%s rsc_len=%d rsc=%s ea=%s\n",
                 key->key_len, key->key_cipher_type, key->key_flags, key->key_index, keydump, key->key_rsc_len, rscdump, eadump);
    } else {
        DebugLog("Set key request, but failed to allocate memory for hexdump\n");
    }
    
    if (keydump)
        IOFree((void*)keydump, 3 * key->key_len + 1);
    if (rscdump)
        IOFree((void*)rscdump, 3 * key->key_rsc_len + 1);
    if (eadump)
        IOFree((void*)eadump, 3 * APPLE80211_ADDR_LEN + 1);
    
    switch (key->key_cipher_type) {
        case APPLE80211_CIPHER_NONE:
            // clear existing key
//            DebugLog("Setting NONE key is not supported\n");
            break;
        case APPLE80211_CIPHER_WEP_40:
        case APPLE80211_CIPHER_WEP_104:
            DebugLog("Setting WEP key %d is not supported\n", key->key_index);
            break;
        case APPLE80211_CIPHER_TKIP:
        case APPLE80211_CIPHER_AES_OCB:
        case APPLE80211_CIPHER_AES_CCM:
            switch (key->key_flags) {
                case 4: // PTK
                    this->instance->setPTK(key->key, key->key_len);
                    this->instance->getNetworkInterface()->postMessage(APPLE80211_M_RSN_HANDSHAKE_DONE, NULL, 0, false);
                    break;
                case 0: // GTK
                    this->instance->setGTK(key->key, key->key_len, key->key_index, key->key_rsc);
                    this->instance->getNetworkInterface()->postMessage(APPLE80211_M_RSN_HANDSHAKE_DONE, NULL, 0, false);
                    break;
            }
            break;
        case APPLE80211_CIPHER_PMK:
            DebugLog("Setting WPA PMK is not supported\n");
            break;
        case APPLE80211_CIPHER_MSK:
            DebugLog("Setting MSK\n");
            ieee80211_pmksa_add(this->instance->ic, IEEE80211_AKM_8021X,
                                this->instance->ic->ic_bss->ni_macaddr, key->key, 0);
            break;
        case APPLE80211_CIPHER_PMKSA:
            DebugLog("Setting WPA PMKSA\n");
            ieee80211_pmksa_add(this->instance->ic, IEEE80211_AKM_8021X,
                                this->instance->ic->ic_bss->ni_macaddr, key->key, 0);
            break;
    }
    //fInterface->postMessage(APPLE80211_M_CIPHER_KEY_CHANGED);
    return kIOReturnSuccess;
}


IOReturn AirPortOpenBSD_SkywalkInterface::getPHY_MODE(struct apple80211_phymode_data *pd)
{
    pd->version = APPLE80211_VERSION;
    
    u_int32_t ic_modecaps= this->instance->ic->ic_modecaps;
    u_int32_t phy_mode  = APPLE80211_MODE_UNKNOWN;
    
    if (ic_modecaps & (1<<IEEE80211_MODE_AUTO))       phy_mode |= APPLE80211_MODE_AUTO;
    if (ic_modecaps & (1<<IEEE80211_MODE_11A))        phy_mode |= APPLE80211_MODE_11A;
    if (ic_modecaps & (1<<IEEE80211_MODE_11B))        phy_mode |= APPLE80211_MODE_11B;
    if (ic_modecaps & (1<<IEEE80211_MODE_11G))        phy_mode |= APPLE80211_MODE_11G;
    if (ic_modecaps & (1<<IEEE80211_MODE_11N))        phy_mode |= APPLE80211_MODE_11N;
    if (ic_modecaps & (1<<IEEE80211_MODE_11AC))       phy_mode |= APPLE80211_MODE_11AC;
    
    u_int32_t ic_curmode= this->instance->ic->ic_curmode;
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

IOReturn AirPortOpenBSD_SkywalkInterface::
getCHANNEL(struct apple80211_channel_data *cd)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    memset(cd, 0, sizeof(*cd));
    cd->version = APPLE80211_VERSION;
    cd->channel.version = APPLE80211_VERSION;
    cd->channel.channel = ieee80211_chan2ieee(this->instance->ic, this->instance->ic->ic_bss->ni_chan);
    cd->channel.flags = this->instance->chanspec2applechannel(this->instance->ic->ic_bss->ni_chan->ic_flags, this->instance->ic->ic_bss->ni_chan->ic_xflags);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getSTATE(struct apple80211_state_data *sd)
{
    memset(sd, 0, sizeof(*sd));
    sd->version = APPLE80211_VERSION;
    sd->state = this->instance->ic->ic_state;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getMCS_INDEX_SET(struct apple80211_mcs_index_set_data *md)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    memset(md, 0, sizeof(*md));
    md->version = APPLE80211_VERSION;
    size_t size = min(nitems(this->instance->ic->ic_bss->ni_rxmcs), nitems(md->mcs_set_map));
    for (int i = 0; i < size; i++) {
        md->mcs_set_map[i] = this->instance->ic->ic_bss->ni_rxmcs[i];
    }
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getVHT_MCS_INDEX_SET(struct apple80211_vht_mcs_index_set_data *data)
{
    if (this->instance->ic->ic_bss == NULL || this->instance->ic->ic_curmode < IEEE80211_MODE_11AC) {
        return kIOReturnError;
    }
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->mcs_map = this->instance->ic->ic_bss->ni_vht_txmcs;
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getMCS_VHT(struct apple80211_mcs_vht_data *data)
{
    if (this->instance->ic->ic_bss == NULL || this->instance->ic->ic_curmode < IEEE80211_MODE_11AC) {
        return kIOReturnError;
    }
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->guard_interval = (ieee80211_node_supports_vht_sgi80(this->instance->ic->ic_bss) || ieee80211_node_supports_vht_sgi160(this->instance->ic->ic_bss)) ? APPLE80211_GI_SHORT : APPLE80211_GI_LONG;
    data->index = this->instance->ic->ic_bss->ni_txmcs;
    data->nss = this->instance->ic->ic_bss->ni_vht_ss;
    
    u_int32_t flags = this->instance->chanspec2applechannel(this->instance->ic->ic_bss->ni_chan->ic_flags, this->instance->ic->ic_bss->ni_chan->ic_xflags);
    
    if(flags & APPLE80211_C_FLAG_160MHZ) {
        data->bw = 160;
    } else if(flags & APPLE80211_C_FLAG_80MHZ) {
        data->bw = 80;
    } else if(flags & APPLE80211_C_FLAG_40MHZ) {
        data->bw = 40;
    } else{
        data->bw = 20;
    }
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getRATE_SET(struct apple80211_rate_set_data *ad)
{
    if (this->instance->ic->ic_state == IEEE80211_S_RUN) {
        memset(ad, 0, sizeof(*ad));
        ad->version = APPLE80211_VERSION;
        ad->num_rates = this->instance->ic->ic_bss->ni_rates.rs_nrates;
        size_t size = min(this->instance->ic->ic_bss->ni_rates.rs_nrates, nitems(ad->rates));
        for (int i=0; i < size; i++) {
            struct apple80211_rate apple_rate = ad->rates[i];
            apple_rate.version = APPLE80211_VERSION;
            apple_rate.rate = this->instance->ic->ic_bss->ni_rates.rs_rates[i];
            apple_rate.flags = 0;
        }
        return kIOReturnSuccess;
    }
    return kIOReturnError;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getOP_MODE(struct apple80211_opmode_data *od)
{
    od->version = APPLE80211_VERSION;
    od->op_mode = APPLE80211_M_STA;
    return kIOReturnSuccess;
    
    uint32_t op_mode = 0;
    
    switch (this->instance->ic->ic_opmode) {
        case IEEE80211_M_STA:
            op_mode = APPLE80211_M_STA;
            break;
        case IEEE80211_M_IBSS:
            op_mode =  APPLE80211_M_IBSS;
            break;
        case IEEE80211_M_AHDEMO:
            op_mode =  APPLE80211_M_AHDEMO;
            break;
        case IEEE80211_M_HOSTAP:
            op_mode =  APPLE80211_M_HOSTAP;
            break;
        case IEEE80211_M_MONITOR:
            op_mode =  APPLE80211_M_MONITOR;
            break;
        default:
            op_mode =  APPLE80211_M_NONE;
            break;
    }
    
    od->version = APPLE80211_VERSION;
    od->op_mode = op_mode;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getTXPOWER(struct apple80211_txpower_data *txd)
{
    struct ifnet *ifp = &this->instance->ic->ic_if;
    struct ieee80211_txpower power;
    if (ifp->if_ioctl(ifp, SIOCG80211TXPOWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    txd->version = APPLE80211_VERSION;
    txd->txpower = power.i_val;
    txd->txpower_unit = APPLE80211_UNIT_PERCENT;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getRATE(struct apple80211_rate_data *rate_data)
{
    struct ifnet *ifp = &this->instance->ic->ic_if;
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    
    if (ifp->tx_tap == NULL)
        return kIOReturnError;
    
    struct tx_radiotap_header *tap =(struct tx_radiotap_header *)ifp->tx_tap;
    
    if (tap->wt_rate == 0) {
        if (this->instance->noise_data.version == APPLE80211_VERSION) {
            memcpy(rate_data, &this->instance->rate_data, sizeof(*rate_data));
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->instance->rate_data.version = APPLE80211_VERSION;
    this->instance->rate_data.num_radios = 1;
    this->instance->rate_data.rate[0] = tap->wt_rate;
    
    memcpy(rate_data, &this->instance->rate_data, sizeof(*rate_data));
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getBSSID(struct apple80211_bssid_data *bd)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return 0x16;
    
    memset(bd, 0, sizeof(*bd));
    bd->version = APPLE80211_VERSION;
    IEEE80211_ADDR_COPY(bd->bssid.octet, this->instance->ic->ic_bss->ni_bssid);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getRSSI(struct apple80211_rssi_data *rd_data)
{
    struct ifnet *ifp = &this->instance->ic->ic_if;
    if (ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)ifp->rx_tap;
    
    if (tap->wr_dbm_antsignal == 0) {
        if (this->instance->rssi_data.version == APPLE80211_VERSION) {
            memcpy(rd_data, &this->instance->rssi_data, sizeof(*rd_data));
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->instance->rssi_data.version = APPLE80211_VERSION;
    this->instance->rssi_data.num_radios = 1;
    this->instance->rssi_data.rssi_unit = APPLE80211_UNIT_DBM;
    this->instance->rssi_data.rssi[0]
    = this->instance->rssi_data.aggregate_rssi
    = this->instance->rssi_data.rssi_ext[0]
    = this->instance->rssi_data.aggregate_rssi_ext
    = tap->wr_dbm_antsignal - 100;
    
    memcpy(rd_data, &this->instance->rssi_data, sizeof(*rd_data));
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getRSN_IE(struct apple80211_rsn_ie_data *data)
{
    DebugLog("");
    if (this->instance->useAppleRSN) {
        if (this->instance->ic->ic_bss == NULL || this->instance->ic->ic_bss->ni_rsnie == NULL) {
            return kIOReturnError;
        }
        data->version = APPLE80211_VERSION;
        if (this->instance->ic->ic_rsnie[1] > 0) {
            data->len = 2 + this->instance->ic->ic_rsnie[1];
            memcpy(data->ie, this->instance->ic->ic_rsnie, data->len);
        } else {
            data->len = 2 + this->instance->ic->ic_bss->ni_rsnie[1];
            memcpy(data->ie, this->instance->ic->ic_bss->ni_rsnie, data->len);
        }
        return kIOReturnSuccess;
    } else {
        return kIOReturnUnsupported;
    }
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setRSN_IE(struct apple80211_rsn_ie_data *data)
{
    if (this->instance->useAppleRSN) {
        if (!data)
            return kIOReturnError;
        static_assert(sizeof(this->instance->ic->ic_rsnie) == APPLE80211_MAX_RSN_IE_LEN, "Max RSN IE length mismatch");
        memcpy(this->instance->ic->ic_rsnie, data->ie, APPLE80211_MAX_RSN_IE_LEN);
        if (this->instance->ic->ic_state == IEEE80211_S_RUN && this->instance->ic->ic_bss != nullptr)
            ieee80211_save_ie(data->ie, &this->instance->ic->ic_bss->ni_rsnie);
        return kIOReturnSuccess;
    } else {
        return kIOReturnUnsupported;
    }
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getAP_IE_LIST(struct apple80211_ap_ie_data *data)
{
    if (!data)
        return kIOReturnError;
    
    if (this->instance->ic->ic_bss == NULL || this->instance->ic->ic_bss->ni_ie == NULL) {
        return kIOReturnError;
    }
    data->version = APPLE80211_VERSION;
    data->len = this->instance->ic->ic_bss->ni_ie_len;
    memcpy(data->ie_data, this->instance->ic->ic_bss->ni_ie, data->len);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getNOISE(struct apple80211_noise_data *nd_data)
{
    struct ifnet *ifp = &this->instance->ic->ic_if;
    if (ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)ifp->rx_tap;
    
    if (tap->wr_dbm_antnoise == 0) {
        if (this->instance->noise_data.version == APPLE80211_VERSION) {
            memcpy(nd_data, &this->instance->noise_data, sizeof(*nd_data));
            return kIOReturnSuccess;
        } else {
            return kIOReturnError;
        }
    }
    
    this->instance->noise_data.version = APPLE80211_VERSION;
    this->instance->noise_data.noise_unit = APPLE80211_UNIT_DBM;
    this->instance->noise_data.num_radios = 1;
    this->instance->noise_data.noise[0]
    = this->instance->noise_data.aggregate_noise
    = this->instance->noise_data.noise_ext[0]
    = this->instance->noise_data.aggregate_noise_ext
    = tap->wr_dbm_antnoise;
    
    memcpy(nd_data, &this->instance->noise_data, sizeof(*nd_data));
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getPOWERSAVE(struct apple80211_powersave_data *pd)
{
    //    pd->version = APPLE80211_VERSION;
    //    pd->powersave_level = powersave_level;
    //    return kIOReturnSuccess;
    
    struct ifnet *ifp = &this->instance->ic->ic_if;
    struct ieee80211_power power;
    if (ifp->if_ioctl(ifp, SIOCG80211POWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    pd->version = APPLE80211_VERSION;
    pd->powersave_level = power.i_maxsleep;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getNSS(struct apple80211_nss_data *data)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->nss = this->instance->ic->ic_bss->ni_vht_ss;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setASSOCIATE(struct apple80211_assoc_data *ad)
{
    DebugLog("[%s] mode=%d ad_auth_lower=%d ad_auth_upper=%d rsn_ie_len=%d%s%s%s%s%s%s%s\n", ad->ad_ssid, ad->ad_mode, ad->ad_auth_lower, ad->ad_auth_upper, ad->ad_rsn_ie_len,
          (ad->ad_flags & 2) ? ", Instant Hotspot" : "",
          (ad->ad_flags & 4) ? ", Auto Instant Hotspot" : "",
          (ad->ad_rsn_ie[APPLE80211_MAX_RSN_IE_LEN] & 1) ? ", don't disassociate" : "",
          (ad->ad_rsn_ie[APPLE80211_MAX_RSN_IE_LEN] & 2) ? ", don't blacklist" : "",
          (ad->ad_rsn_ie[APPLE80211_MAX_RSN_IE_LEN] & 4) ? ", closed Network" : "",
          (ad->ad_rsn_ie[APPLE80211_MAX_RSN_IE_LEN] & 8) ? ", 802.1X" : "",
          (ad->ad_rsn_ie[APPLE80211_MAX_RSN_IE_LEN] & 0x20) ? ", force BSSID" : "");
    
    if (!ad)
        return kIOReturnError;
    
    struct ifnet *ifp = &this->instance->ic->ic_if;
    
    if (this->instance->ic->ic_state == IEEE80211_S_INIT) {
        return kIOReturnError;
    }
    
    if (this->instance->ic->ic_state == IEEE80211_S_ASSOC || this->instance->ic->ic_state == IEEE80211_S_AUTH) {
        return kIOReturnError;
    }
    
    if (ad->ad_mode != APPLE80211_AP_MODE_IBSS) {
        
        struct apple80211_authtype_data _authtype_data;
        _authtype_data.version = APPLE80211_VERSION;
        _authtype_data.authtype_lower = ad->ad_auth_lower;
        _authtype_data.authtype_upper = ad->ad_auth_upper;
        this->setAUTH_TYPE(&_authtype_data);
        
        struct apple80211_rsn_ie_data rsn_ie_data;
        rsn_ie_data.version = APPLE80211_VERSION;
        rsn_ie_data.len = ad->ad_rsn_ie[1] + 2;
        memcpy(rsn_ie_data.ie, ad->ad_rsn_ie, rsn_ie_data.len);
        this->setRSN_IE(&rsn_ie_data);
        
        // OPEN认证           ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 0
        // WEP认证，开放系统    ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 1
        // WEP认证，共享密钥    ad_auth_lower = 2, ad_auth_upper = 0, key_cipher_type = 2
        // WPA认证            ad_auth_lower = 1, ad_auth_upper = 2, key_cipher_type = 6
        // WPA2认证           ad_auth_lower = 1, ad_auth_upper = 8, key_cipher_type = 6
        // 80211X认证         ad_auth_lower = 1, ad_auth_upper = 4, key_cipher_type = 0
        
        this->instance->configArrCount = 0;
        this->instance->configArr[this->instance->configArrCount++] = ifp->if_xname;
        this->instance->configArr[this->instance->configArrCount++] = "nwid";
        this->instance->configArr[this->instance->configArrCount++] = (const char *)ad->ad_ssid;
        
        get_hexstring(ad->ad_key.key, this->instance->i_psk, ad->ad_key.key_len);
        
        if (ad->ad_auth_upper & (APPLE80211_AUTHTYPE_WPA_PSK | APPLE80211_AUTHTYPE_WPA2_PSK | APPLE80211_AUTHTYPE_SHA256_PSK) ) {
            // WPA/WPA2认证
            this->instance->configArr[this->instance->configArrCount++] = "wpaprotos";
            this->instance->configArr[this->instance->configArrCount++] = "wpa1,wpa2";
            
        }
        
        if (ad->ad_auth_upper & (APPLE80211_AUTHTYPE_WPA3_SAE | APPLE80211_AUTHTYPE_WPA3_FT_SAE) ) {
            // WPA2认证
            this->instance->configArr[this->instance->configArrCount++] = "wpaprotos";
            this->instance->configArr[this->instance->configArrCount++] = "wpa2";
        }
        
        if (ad->ad_auth_upper & (APPLE80211_AUTHTYPE_WPA_8021X | APPLE80211_AUTHTYPE_WPA2_8021X |APPLE80211_AUTHTYPE_SHA256_8021X) ) {
            // 企业WPA/WPA2认证
            this->instance->configArr[this->instance->configArrCount++] = "wpa";
            this->instance->configArr[this->instance->configArrCount++] = "wpaakms";
            this->instance->configArr[this->instance->configArrCount++] = "802.1x";
            this->instance->configArr[this->instance->configArrCount++] = "wpaprotos";
            this->instance->configArr[this->instance->configArrCount++] = "wpa1,wpa2";
        }
        
        if (ad->ad_auth_upper & (APPLE80211_AUTHTYPE_WPA3_8021X | APPLE80211_AUTHTYPE_WPA3_FT_8021X) ) {
            // 企业WPA2认证
            this->instance->configArr[this->instance->configArrCount++] = "wpa";
            this->instance->configArr[this->instance->configArrCount++] = "wpaakms";
            this->instance->configArr[this->instance->configArrCount++] = "802.1x";
            this->instance->configArr[this->instance->configArrCount++] = "wpaprotos";
            this->instance->configArr[this->instance->configArrCount++] = "wpa2";
        }
        
        // 加密方式
        if (ad->ad_key.key_cipher_type & (APPLE80211_CIPHER_WEP_40 | APPLE80211_CIPHER_WEP_104)) {
            // 共享密钥
            this->instance->configArr[this->instance->configArrCount++] = "nwkey";
            this->instance->configArr[this->instance->configArrCount++] = this->instance->i_psk;
        }
        
        if (ad->ad_key.key_cipher_type & (APPLE80211_CIPHER_TKIP | APPLE80211_CIPHER_PMK)) {
            // WPA/WPA2认证
            snprintf(this->instance->key_tmp, sizeof(this->instance->key_tmp), "0x%s", this->instance->i_psk);
            this->instance->configArr[this->instance->configArrCount++] = "wpakey";
            this->instance->configArr[this->instance->configArrCount++] = this->instance->key_tmp;
        }
        
        ifconfig(this->instance->configArr, this->instance->configArrCount);

    }
    
    this->instance->ic->ic_deauth_reason = APPLE80211_REASON_UNSPECIFIED;
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setDISASSOCIATE(struct apple80211_disassoc_data *ad)
{
    struct ifnet *ifp = &this->instance->ic->ic_if;

    if (this->instance->ic->ic_state != IEEE80211_S_RUN) {
        return kIOReturnSuccess;
    }

    DebugLog("this->instance->ic->ic_des_essid = %s", this->instance->ic->ic_des_essid);

    ieee80211_new_state(this->instance->ic, IEEE80211_S_ASSOC,
        IEEE80211_FC0_SUBTYPE_DISASSOC);

    while (this->instance->ic->ic_state == IEEE80211_S_RUN) {
        tsleep_nsec(&this->instance->ic->ic_state, 0, "DISASSOC", MSEC_TO_NSEC(50));
    }
    
    this->instance->scanFreeResults(0);
    
    this->instance->configArrCount = 0;
    this->instance->configArr[this->instance->configArrCount++] = ifp->if_xname;
    this->instance->configArr[this->instance->configArrCount++] = "-nwid";
    this->instance->configArr[this->instance->configArrCount++] = (const char *)this->instance->ic->ic_des_essid;
    ifconfig(this->instance->configArr, this->instance->configArrCount);
    
    ieee80211_new_state(this->instance->ic, IEEE80211_S_SCAN, -1);
    
    this->instance->ic->ic_deauth_reason = APPLE80211_REASON_AUTH_LEAVING;

    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getSUPPORTED_CHANNELS(struct apple80211_sup_channel_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->num_channels = 0;
    for (int i = 0; i < IEEE80211_CHAN_MAX && ad->num_channels < APPLE80211_MAX_CHANNELS; i++) {
        if (this->instance->ic->ic_channels[i].ic_freq != 0) {
            ad->supported_channels[ad->num_channels].channel = ieee80211_chan2ieee(this->instance->ic, &this->instance->ic->ic_channels[i]);
            ad->supported_channels[ad->num_channels].flags   = this->instance->chanspec2applechannel(this->instance->ic->ic_channels[i].ic_flags, this->instance->ic->ic_channels[i].ic_xflags);
            ad->supported_channels[ad->num_channels].version = APPLE80211_VERSION;
            ad->num_channels++;
        }
    }
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getLOCALE(struct apple80211_locale_data *ld)
{
    ld->version = APPLE80211_VERSION;
    ld->locale  = APPLE80211_LOCALE_ROW;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getDEAUTH(struct apple80211_deauth_data *dd)
{
    if (!dd)
        return kIOReturnError;
    
    dd->version = APPLE80211_VERSION;
//    dd->deauth_reason = APPLE80211_REASON_ASSOC_LEAVING;
    dd->deauth_reason = this->instance->ic->ic_deauth_reason;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getASSOCIATION_STATUS(struct apple80211_assoc_status_data *ret)
{
    if (!ret)
        return kIOReturnError;
    
    ret->version = APPLE80211_VERSION;
    if (this->instance->ic->ic_state == IEEE80211_S_RUN)
        ret->status = APPLE80211_STATUS_SUCCESS;
    else
        ret->status = APPLE80211_STATUS_UNAVAILABLE;

    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setSCANCACHE_CLEAR(void *req)
{
    DebugLog("----");
    
    if (this->instance->ic->ic_state != IEEE80211_S_RUN) {
        this->instance->scanFreeResults(0);
    }
    
    //if doing background or active scan, don't free nodes.
    if ((this->instance->ic->ic_flags & IEEE80211_F_BGSCAN) || (this->instance->ic->ic_flags & IEEE80211_F_ASCAN)) {
        return kIOReturnSuccess;
    }
    
    ieee80211_free_allnodes(this->instance->ic, 0);
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setDEAUTH(struct apple80211_deauth_data *da)
{
    DebugLog("");
    
    if (this->instance->ic->ic_state < IEEE80211_S_SCAN) {
        return kIOReturnSuccess;
    }
    
    if (this->instance->ic->ic_state > IEEE80211_S_AUTH && this->instance->ic->ic_bss != NULL)
        IEEE80211_SEND_MGMT(this->instance->ic, this->instance->ic->ic_bss, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_AUTH_LEAVE);
    
    this->instance->ic->ic_deauth_reason = APPLE80211_REASON_AUTH_LEAVING;
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getMCS(struct apple80211_mcs_data* md)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    md->version = APPLE80211_VERSION;
    md->index = this->instance->ic->ic_bss->ni_txmcs;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getLINK_CHANGED_EVENT_DATA(struct apple80211_link_changed_event_data *ed)
{
    if (ed == NULL)
        return 16;

    bzero(ed, sizeof(*ed));
    
    ed->isLinkDown = this->instance->ic->ic_state == IEEE80211_S_INIT;
    DebugLog("ed->isLinkDown = %d", ed->isLinkDown);
    
    if (ed->isLinkDown) {
        ed->voluntary = true;
        ed->reason = APPLE80211_LINK_DOWN_REASON_DEAUTH;
    } else {
        ed->rssi = -(100 - this->instance->ic->ic_bss->ni_rssi);
    }
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
setSCAN_REQ(struct apple80211_scan_data *ssid)
{
    if (ssid == NULL) {
        return 0x16;
    }
    DebugLog("Scan requested. "
          "Type: %u "
          "BSS Type: %u "
          "PHY Mode: %u "
          "Dwell time: %u "
//             "unk: %u "
//          "unk time: %u "
          "Rest time: %u "
          "Num channels: %u "
          "BSSID: %s "
          "SSID: %s "
          "SSIDLEN: %d "
//          "scan_flags: %d "
          "sizeof: %u ",
          ssid->scan_type,
          ssid->bss_type,
          ssid->phy_mode,
          ssid->dwell_time,
//             ssid->unk,
//          ssid->unk_time,
          ssid->rest_time,
          ssid->num_channels,
          ether_sprintf(ssid->bssid.octet),
          ssid->ssid,
          ssid->ssid_len,
//          ssid->scan_flags,
          sizeof(*ssid));
    
    uint32_t min_channel = 165;
    uint32_t max_channel = 0;
    for (int i = 0; i < ssid->num_channels; i++) {
        min_channel = min(min_channel, ssid->channels[i].channel);
        max_channel = max(max_channel, ssid->channels[i].channel);
    }
    DebugLog("min_channel = %d max_channel = %d", min_channel, max_channel);
    
    struct ifnet *ifp = &this->instance->ic->ic_if;
    UInt32 ms = 200;
    
    // 设置ssid，主动扫描
    if (ssid->ssid_len > 0) {
        
        // 更新ssid列表
        this->instance->scanComplete();
        
        // 先查找ssid存不存在
        bool found = false;
        
        struct apple80211_scan_result_list *scan_result_list, *tmp;
        SLIST_FOREACH_SAFE(scan_result_list, &this->instance->scan_result_lists, list, tmp) {
            // 查找ssid
            if (strcmp((char*)ssid->ssid, (char*)scan_result_list->scan_result.asr_ssid) == 0) {
                // 找到
                found = true;
                break;
            }
        }
        
        if (!found) {
            // 没有找到则主动扫描3秒
            ms = 1000;
            
            // 没有找到才清空之前主动扫描的ssid，否则继续主动扫描
            this->instance->ic->direct_scan_count = 0;
            bzero(this->instance->ic->direct_scan, sizeof(this->instance->ic->direct_scan));
            
        }
        
        // 主动扫描
        bool direct_found = false;
        for (int i = 0; i < this->instance->ic->direct_scan_count; i++) {
            // 查找ssid
            if (strcmp((char*)ssid->ssid, (char*)this->instance->ic->direct_scan[i].ssid) == 0) {
                // 找到
                direct_found = true;
                break;
            }
        }
        
        if (!found && !direct_found) {
            // 不在ssid列表里并且也不在主动扫描里，则添加
            this->instance->ic->direct_scan[this->instance->ic->direct_scan_count].len = ssid->ssid_len;
            memcpy(this->instance->ic->direct_scan[this->instance->ic->direct_scan_count].ssid,
                   ssid->ssid, ssid->ssid_len);
            this->instance->ic->direct_scan_count++;
        }
        
    }
    
    for (int i = 0; i < this->instance->ic->direct_scan_count; i++) {
        DebugLog("%s, %d", this->instance->ic->direct_scan[i].ssid, ms);
    }
    
    ieee80211_begin_cache_bgscan(ifp);
    
    if (this->instance->fScanSource) {
        this->instance->fScanSource->setTimeoutMS(ms);
        this->instance->fScanSource->enable();
    }

    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getCURRENT_NETWORK(apple80211_scan_result *oneResult)
{
    if (this->instance->ic->ic_state != IEEE80211_S_RUN || this->instance->ic->ic_bss == NULL)
        return kIOReturnError;
    
    bzero(oneResult, sizeof(apple80211_scan_result));
    
    struct ieee80211_node *ni = this->instance->ic->ic_bss;
    
    this->instance->scanConvertResult(ni, oneResult);
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getCOLOCATED_NETWORK_SCOPE_ID(apple80211_colocated_network_scope_id *as)
{
    if (!as)
        return kIOReturnBadArgument;
    as->version = APPLE80211_VERSION;
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD_SkywalkInterface::
getSCAN_RESULT(struct apple80211_scan_result *sr)
{
    IOReturn ret = 0x05;
    struct apple80211_scan_result_list *next;
    
    if (!SLIST_EMPTY(&this->instance->scan_result_lists)) {
        next = this->instance->scan_result_next;
        if (next != NULL) {
            
            bzero(sr, sizeof(*sr));
            
            struct apple80211_scan_result *_one = &next->scan_result;
            
            sr->version = APPLE80211_VERSION;

            sr->asr_channel.version = APPLE80211_VERSION;
            sr->asr_channel.channel = _one->asr_channel.channel;
            sr->asr_channel.flags = _one->asr_channel.flags;

            sr->asr_use = 1;
            sr->asr_noise = 0;
            sr->asr_rssi = _one->asr_rssi;
            sr->asr_snr = sr->asr_rssi - sr->asr_noise;
            sr->asr_beacon_int = _one->asr_beacon_int;
            sr->asr_cap = _one->asr_cap;
            IEEE80211_ADDR_COPY(sr->asr_bssid.octet, _one->asr_bssid.octet);
            sr->asr_nrates = _one->asr_nrates;
            for (int r = 0; r < sr->asr_nrates; r++)
                sr->asr_rates[r] = _one->asr_rates[r];
            sr->asr_ssid_len = _one->asr_ssid_len;
            sr->asr_ssid_len = _one->asr_ssid_len;
            if (sr->asr_ssid_len != 0)
                memcpy(sr->asr_ssid, _one->asr_ssid, sr->asr_ssid_len);
            
            sr->asr_age = _one->asr_age;
            sr->asr_ie_len = _one->asr_ie_len;
            if (sr->asr_ie_len > 0) {
                memcpy(sr->asr_ie_data, _one->asr_ie_data, min(sr->asr_ie_len, sizeof(sr->asr_ie_data)));
            }
            
//            memcpy(sr, &next->scan_result, sizeof(*sr));
//            DebugLog("sr->asr_ssid = %s", sr->asr_ssid);
            
            this->instance->scan_result_next = SLIST_NEXT(this->instance->scan_result_next, list);
            ret = kIOReturnSuccess;
        }
    }
    
    if (ret != kIOReturnSuccess) {
        DebugLog("");
    }
    
    return  ret;
}
