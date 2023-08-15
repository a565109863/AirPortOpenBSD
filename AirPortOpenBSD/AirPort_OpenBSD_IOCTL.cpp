//
//  AirPort_OpenBSD_IOCTL.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPort_OpenBSD.hpp"

//
// MARK: 1 - SSID
//

IOReturn AirPort_OpenBSD_Class::getSSID(OSObject *object, struct apple80211_ssid_data *sd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    
    struct ifnet *ifp = &this->ic->ic_if;
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

IOReturn AirPort_OpenBSD_Class::setSSID(OSObject *object, struct apple80211_ssid_data *sd)
{
    DebugLog("");
    
    struct ifnet *ifp = &this->ic->ic_if;
    this->configArr[0] = ifp->if_xname;
    this->configArr[1] = "nwid";
    this->configArr[2] = (const char *)sd->ssid;
    this->configArrCount = 2;
    ifconfig(this->configArr, this->configArrCount);
    
    return kIOReturnSuccess;
}

//
// MARK: 2 - AUTH_TYPE
//

IOReturn AirPort_OpenBSD_Class::getAUTH_TYPE(OSObject *object, struct apple80211_authtype_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->authtype_lower = current_authtype_lower;
    ad->authtype_upper = current_authtype_upper;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setAUTH_TYPE(OSObject *object, struct apple80211_authtype_data *ad)
{
    current_authtype_lower = ad->authtype_lower;
    current_authtype_upper = ad->authtype_upper;
    return kIOReturnSuccess;
}

//
// MARK: 3 - CIPHER_KEY
//

IOReturn AirPort_OpenBSD_Class::setCIPHER_KEY(OSObject *object, struct apple80211_key *key)
{
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
        DebugLog(":Set key request, but failed to allocate memory for hexdump\n");
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
                    setPTK(key->key, key->key_len);
                    getNetworkInterface()->postMessage(APPLE80211_M_RSN_HANDSHAKE_DONE);
                    break;
                case 0: // GTK
                    setGTK(key->key, key->key_len, key->key_index, key->key_rsc);
                    getNetworkInterface()->postMessage(APPLE80211_M_RSN_HANDSHAKE_DONE);
                    break;
            }
            break;
        case APPLE80211_CIPHER_PMK:
            DebugLog("Setting WPA PMK is not supported\n");
            break;
        case APPLE80211_CIPHER_PMKSA:
            DebugLog("Setting WPA PMKSA is not supported\n");
            break;
    }
    
    return kIOReturnSuccess;
}

//
// MARK: 4 - CHANNEL
//

IOReturn AirPort_OpenBSD_Class::getCHANNEL(OSObject *object, struct apple80211_channel_data *cd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    memset(cd, 0, sizeof(*cd));
    cd->version = APPLE80211_VERSION;
    cd->channel.version = APPLE80211_VERSION;
    cd->channel.channel = ieee80211_chan2ieee(this->ic, this->ic->ic_bss->ni_chan);
    cd->channel.flags = chanspec2applechannel(this->ic->ic_bss->ni_chan->ic_flags, this->ic->ic_bss->ni_chan->ic_xflags);
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setCHANNEL(OSObject *object, struct apple80211_channel_data *data)
{
    DebugLog("channel=%d\n", data->channel.channel);
    return kIOReturnSuccess;
}

//
// MARK: 5 - POWERSAVE
//

IOReturn AirPort_OpenBSD_Class::getPOWERSAVE(OSObject *object, struct apple80211_powersave_data* pd)
{
//    pd->version = APPLE80211_VERSION;
//    pd->powersave_level = powersave_level;
//    return kIOReturnSuccess;
    
    struct ifnet *ifp = &this->ic->ic_if;
    struct ieee80211_power power;
    if (ifp->if_ioctl(ifp, SIOCG80211POWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    pd->version = APPLE80211_VERSION;
    pd->powersave_level = power.i_maxsleep;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setPOWERSAVE(OSObject *object, struct apple80211_powersave_data* pd)
{
//    powersave_level = pd->powersave_level;
//    return kIOReturnSuccess;
    
    struct ifnet *ifp = &this->ic->ic_if;
    
    struct ieee80211_power power = {NULL, 1, pd->powersave_level};
    if (ifp->if_ioctl(ifp, SIOCS80211POWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    DebugLog("power.i_enabled = %d, power.i_maxsleep = %d", power.i_enabled, pd->powersave_level);
    
    return kIOReturnSuccess;
}

//
// MARK: 6 - PROTMODE
//

IOReturn AirPort_OpenBSD_Class::getPROTMODE(OSObject *object, struct apple80211_protmode_data* pd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN)
        return kIOReturnError;
    memset(pd, 0, sizeof(*pd));
    pd->version = APPLE80211_VERSION;
    pd->protmode = APPLE80211_PROTMODE_AUTO;
    pd->threshold = this->ic->ic_rtsthreshold;
    return kIOReturnSuccess;
}

//
// MARK: 7 - TXPOWER
//

IOReturn AirPort_OpenBSD_Class::getTXPOWER(OSObject *object, struct apple80211_txpower_data *txd)
{
    struct ifnet *ifp = &this->ic->ic_if;
    struct ieee80211_txpower power;
    if (ifp->if_ioctl(ifp, SIOCG80211TXPOWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    txd->version = APPLE80211_VERSION;
    txd->txpower = power.i_val;
    txd->txpower_unit = APPLE80211_UNIT_PERCENT;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setTXPOWER(OSObject *object, struct apple80211_txpower_data *txd)
{
    struct ifnet *ifp = &this->ic->ic_if;
    
    struct ieee80211_txpower power = {NULL, 0, power.i_val};
    if (ifp->if_ioctl(ifp, SIOCS80211TXPOWER, (caddr_t)&power) != 0)
        return kIOReturnError;
    
    DebugLog("power.i_val = %d, power.txpower_unit = %d", power.i_val, txd->txpower_unit);
    
    return kIOReturnSuccess;
}

//
// MARK: 8 - RATE
//

IOReturn AirPort_OpenBSD_Class::getRATE(OSObject *object, struct apple80211_rate_data *rate_data)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    
    if (ifp->tx_tap == NULL)
        return kIOReturnError;
    
    struct tx_radiotap_header *tap =(struct tx_radiotap_header *)ifp->tx_tap;
    
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

IOReturn AirPort_OpenBSD_Class::getBSSID(OSObject *object, struct apple80211_bssid_data *bd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return 0x16;
    
    memset(bd, 0, sizeof(*bd));
    bd->version = APPLE80211_VERSION;
    IEEE80211_ADDR_COPY(bd->bssid.octet, this->ic->ic_bss->ni_bssid);
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setBSSID(OSObject *object, struct apple80211_bssid_data *bd)
{
    DebugLog("bssid=%s", ether_sprintf(bd->bssid.octet));
    return kIOReturnSuccess;
}

//
// MARK: 10 - SCAN_REQ
//

IOReturn AirPort_OpenBSD_Class::setSCAN_REQ(OSObject *object, struct apple80211_scan_data *sd)
{
    if (sd == NULL) {
        return 0x16;
    }
    DebugLog("Scan requested. "
          "Type: %u "
          "BSS Type: %u "
          "PHY Mode: %u "
          "Dwell time: %u "
             "unk: %u "
          "unk time: %u "
          "Rest time: %u "
          "Num channels: %u "
          "BSSID: %s "
          "SSID: %s "
          "SSIDLEN: %d "
//          "scan_flags: %d "
          "sizeof: %u ",
          sd->scan_type,
          sd->bss_type,
          sd->phy_mode,
          sd->dwell_time,
             sd->unk,
          sd->unk_time,
          sd->rest_time,
          sd->num_channels,
          ether_sprintf(sd->bssid.octet),
          sd->ssid,
          sd->ssid_len,
//          sd->scan_flags,
          sizeof(*sd));
    
    for (int i = 0; i < sd->num_channels; i++) {
        DebugLog("i = %d channel = %d", i, sd->channels[i].channel);
    }
    
    if (this->ic->ic_state == IEEE80211_S_SCAN || this->ic->ic_state == IEEE80211_S_RUN) {
        // 设置ssid，主动扫描
        if (sd->ssid_len > 0) {
            
            if (this->ic->ic_state > IEEE80211_S_SCAN && memcmp(sd->ssid, this->ic->ic_des_essid, max(sd->ssid_len, this->ic->ic_des_esslen)) == 0) {
                bzero(&this->scan_ssid, sizeof(this->scan_ssid));
            } else {
                struct apple80211_ssid_data *ssid, ssid1 = {};
                ssid1.version = APPLE80211_VERSION;
                ssid1.ssid_len = sd->ssid_len;
                bcopy(sd->ssid, ssid1.ssid, ssid1.ssid_len);
                ssid = &ssid1;
                
                // 查找有没有在已知网络里
                bool found_known_ssid = false;
                struct apple80211_ssid_data_known_list *known_ssid_list, *tmp;
                SLIST_FOREACH_SAFE(known_ssid_list, &this->known_ssid_lists, list, tmp) {
                    // 查找ssid和channel
                    if (memcmp(ssid->ssid, known_ssid_list->ssid.ssid, max(ssid->ssid_len, known_ssid_list->ssid.ssid_len)) == 0) {
                        // 找到已知网络ssid
                        found_known_ssid = true;
                        break;
                    }
                }
                
                //            if (!found_known_ssid) {
                //                // 加入已知网络
                //                known_ssid_list = (typeof known_ssid_list)IOMalloc(sizeof(struct apple80211_ssid_data_known_list));
                //                bzero(known_ssid_list, sizeof(struct apple80211_ssid_data_known_list));
                //                bcopy(ssid, &known_ssid_list->ssid, sizeof(struct apple80211_ssid_data));
                //                SLIST_INSERT_HEAD(&this->known_ssid_lists, known_ssid_list, list);
                //            }
                
                if (!found_known_ssid || this->ic->ic_state == IEEE80211_S_SCAN) {
                    // 没找到，或正在扫描中，则主动扫描
                    DebugLog("this->scan_ssid.ssid = %s, sd->ssid = %s", this->scan_ssid.ssid, sd->ssid);
                    
                    // 查找有没有在扫描结果里
                    bool found_scan_result = false;
                    
                    struct apple80211_scan_result_list *scan_result_list, *tmp;
                    SLIST_FOREACH_SAFE(scan_result_list, &this->scan_result_lists, list, tmp) {
                        // 查找ssid
                        if (memcmp((char*) sd->ssid, (char*)scan_result_list->scan_result.asr_ssid, max(scan_result_list->scan_result.asr_ssid_len, sd->ssid_len)) == 0) {
                            // 找到扫描结果的ssid
                            found_scan_result = true;
                            
                            bzero(&this->scan_ssid, sizeof(this->scan_ssid));
                            break;
                        }
                    }
                    
                    if (!found_scan_result) {
                        // 不存在
                        if (memcmp(this->scan_ssid.ssid, sd->ssid, max(this->scan_ssid.ssid_len, sd->ssid_len)) != 0) {
                            
                            bzero(&this->scan_ssid, sizeof(this->scan_ssid));
                            
                            this->scan_ssid.ssid_len = sd->ssid_len;
                            bcopy(sd->ssid, this->scan_ssid.ssid, this->scan_ssid.ssid_len);
                            
                            struct ifnet *ifp = &this->ic->ic_if;
                            this->configArr[0] = ifp->if_xname;
                            this->configArr[1] = "nwid";
                            this->configArr[2] = (const char *)sd->ssid;
                            this->configArrCount = 2;
                            ifconfig(this->configArr, this->configArrCount);
                            
                            while ((this->ic->ic_flags & IEEE80211_F_ASCAN) && this->ic->ic_state == IEEE80211_S_SCAN) {
                                DebugLog("this->scan_ssid.ssid = %s, sd->ssid = %s", this->scan_ssid.ssid, sd->ssid);
                                tsleep_nsec(&this->ic->ic_flags, 0, "scan ssid", SEC_TO_NSEC(1));
                            }
                            
                            DebugLog("this->scan_ssid.ssid = %s, sd->ssid = %s", this->scan_ssid.ssid, sd->ssid);
                            
                        }
                    }
                }
            }
        }
    }
    
    this->scanComplete();
    
    if (this->fScanSource) {
        this->fScanSource->setTimeoutMS(sd->dwell_time);
        this->fScanSource->enable();
    }

    return kIOReturnSuccess;
}

//
// MARK: 86 - SCAN_REQ_MULTIPLE
//

IOReturn AirPort_OpenBSD_Class::setSCAN_REQ_MULTIPLE(OSObject *object, struct apple80211_scan_multiple_data *smd)
{
    if (smd == NULL) {
        return 0x16;
    }
    
    DebugLog("Scan requested. Type: %u "
          "SSID count: %u "
          "PHY Mode: %u "
          "Dwell time: %u "
          "Rest time: %u "
//          "unk_2: %d "
          "Num channels: %u ",
          smd->scan_type,
          smd->ssid_count,
          smd->phy_mode,
          smd->dwell_time,
          smd->rest_time,
//          smd->unk_2,
          smd->num_channels);
    
    for (int i = 0; i < smd->ssid_count; i++) {
        DebugLog("i = %d ssid = %s ssid_len = %d", i, smd->ssids[i].ssid, smd->ssids[i].ssid_len);
    }
    
    // 保存已知网络
    for (int i = 0; i < smd->ssid_count; i++) {
        struct apple80211_ssid_data *ssid = &smd->ssids[i];
        if (ssid->ssid_len > 0) {
            // 查找有没有在已知网络里
            bool found_known_ssid = false;
            struct apple80211_ssid_data_known_list *known_ssid_list, *tmp;
            SLIST_FOREACH_SAFE(known_ssid_list, &this->known_ssid_lists, list, tmp) {
                // 查找ssid和channel
                if (memcmp(ssid->ssid, known_ssid_list->ssid.ssid, max(ssid->ssid_len, known_ssid_list->ssid.ssid_len)) == 0) {
                    // 找到已知网络ssid
                    found_known_ssid = true;
                    break;
                }
            }
            
            if (!found_known_ssid) {
                // 加入已知网络
                known_ssid_list = (typeof known_ssid_list)IOMalloc(sizeof(struct apple80211_ssid_data_known_list));
                bzero(known_ssid_list, sizeof(struct apple80211_ssid_data_known_list));
                bcopy(ssid, &known_ssid_list->ssid, sizeof(struct apple80211_ssid_data));
                SLIST_INSERT_HEAD(&this->known_ssid_lists, known_ssid_list, list);
            }
        }
    }
    
    if (this->ic->ic_state == IEEE80211_S_SCAN) {
        
#if MAC_VERSION_MAJOR < MAC_VERSION_MAJOR_Ventura
        
        uint32_t max_channel = 0;
        for (int i = 0; i < smd->num_channels; i++) {
            DebugLog("i = %d channel = %d", i, smd->channels[i].channel);
            max_channel = max(max_channel, smd->channels[i].channel);
        }
        
        for (int i = 0; i < smd->ssid_count && smd->ssid_count == 2; i++) {
            struct apple80211_ssid_data *ssid = &smd->ssids[i];
            if (ssid->ssid_len > 0) {
                if (this->ic->ic_state > IEEE80211_S_SCAN && memcmp(ssid->ssid, this->ic->ic_des_essid, max(ssid->ssid_len, this->ic->ic_des_esslen)) == 0) {
                    bzero(&this->scan_ssid, sizeof(this->scan_ssid));
                    continue;
                }
                
                if (this->active_scan == 0 && smd->num_channels == 6) {
                    // 准备主动扫描ssid
                    if (memcmp(this->scan_ssid.ssid, ssid->ssid, max(this->scan_ssid.ssid_len, ssid->ssid_len)) != 0) {
                        
                        bzero(&this->scan_ssid, sizeof(this->scan_ssid));
                        
                        this->scan_ssid.ssid_len = ssid->ssid_len;
                        bcopy(ssid->ssid, this->scan_ssid.ssid, this->scan_ssid.ssid_len);
                        
                        this->active_scan = 1;
                        
                    }
                } else if (this->active_scan == 1 && max_channel == 165) {
                    
                    if (memcmp(this->scan_ssid.ssid, ssid->ssid, max(this->scan_ssid.ssid_len, ssid->ssid_len)) == 0) {
                        // 开始主动扫描ssid
                        DebugLog("this->scan_ssid.ssid = %s, ssid->ssid = %s", this->scan_ssid.ssid, ssid->ssid);
                        
                        struct ifnet *ifp = &this->ic->ic_if;
                        this->configArr[0] = ifp->if_xname;
                        this->configArr[1] = "nwid";
                        this->configArr[2] = (const char *)this->scan_ssid.ssid;
                        this->configArrCount = 2;
                        ifconfig(this->configArr, this->configArrCount);
                        
                        while ((this->ic->ic_flags & IEEE80211_F_ASCAN) && this->ic->ic_state == IEEE80211_S_SCAN) {
                            DebugLog("this->scan_ssid.ssid = %s, ssid->ssid = %s", this->scan_ssid.ssid, ssid->ssid);
                            tsleep_nsec(&this->ic->ic_flags, 0, "scan ssid", SEC_TO_NSEC(1));
                        }
                        
                        DebugLog("this->scan_ssid.ssid = %s, ssid->ssid = %s", this->scan_ssid.ssid, ssid->ssid);
                        
                    }
                    
                    this->active_scan = 0;
                }
            }
        }
        
        if (max_channel == 165) {
            this->active_scan = 0;
        }
        
#endif
    
    }
    
    this->scanComplete();
    
    if (this->fScanSource) {
        this->fScanSource->setTimeoutMS(smd->dwell_time);
        this->fScanSource->enable();
    }

    return kIOReturnSuccess;
}

//
// MARK: 11 - SCAN_RESULT
//

IOReturn AirPort_OpenBSD_Class::getSCAN_RESULT(OSObject *object, struct apple80211_scan_result **sr)
{
    IOReturn ret = kIOReturnSuccess;
    struct apple80211_scan_result_list *next;
    
    if (!SLIST_EMPTY(&this->scan_result_lists)) {
        if (sr != NULL) {
            next = this->scan_result_next;
            if (next != NULL) {
                *sr = &next->scan_result;
                this->scan_result_next = SLIST_NEXT(this->scan_result_next, list);
                ret = kIOReturnSuccess;
                
//                DebugLog("asr_ssid = %s", next->scan_result.asr_ssid);
            } else {
                ret = 0x05;
            }
        } else {
            ret = 0x16;
        }
    } else {
        ret = 0x10;
    }
    
    if (ret != kIOReturnSuccess) {
        DebugLog("");
    }
    
    return  ret;
}

//
// MARK: 12 - CARD_CAPABILITIES
//

IOReturn AirPort_OpenBSD_Class::getCARD_CAPABILITIES(OSObject *object, struct apple80211_capability_data *cd)
{
    if (cd == NULL) {
        return 0x16;
    }
    
    int32_t rdx = 0;
    int8_t rcx = 0;
    int8_t rax = 0;
    
    memset(cd, 0, sizeof(*cd));
    cd->version = APPLE80211_VERSION;

    u_int32_t caps = 0;

    caps |=  1 << APPLE80211_CAP_AES;
    caps |=  1 << APPLE80211_CAP_AES_CCM;

    if (this->ic->ic_caps & IEEE80211_C_WEP)              caps |= 1 << APPLE80211_CAP_WEP;
    if (this->ic->ic_caps & IEEE80211_C_RSN) {
        caps |=  1 << APPLE80211_CAP_TKIP;
        caps |=  1 << APPLE80211_CAP_TKIPMIC;
        caps |=  1 << APPLE80211_CAP_WPA;
        caps |=  1 << APPLE80211_CAP_WPA1;
        caps |=  1 << APPLE80211_CAP_WPA2;
    }
    if (this->ic->ic_caps & IEEE80211_C_MONITOR)          caps |= 1 << APPLE80211_CAP_MONITOR;
    if (this->ic->ic_caps & IEEE80211_C_SHSLOT)           caps |= 1 << APPLE80211_CAP_SHSLOT;
    if (this->ic->ic_caps & IEEE80211_C_SHPREAMBLE)       caps |= 1 << APPLE80211_CAP_SHPREAMBLE;
//    if (this->ic->ic_caps & IEEE80211_C_AHDEMO)           caps |= 1 << APPLE80211_CAP_IBSS;
    if (this->ic->ic_caps & IEEE80211_C_PMGT)             caps |= 1 << APPLE80211_CAP_PMGT;
    if (this->ic->ic_caps & IEEE80211_C_TXPMGT)           caps |= 1 << APPLE80211_CAP_TXPMGT;
//    if (this->ic->ic_caps & IEEE80211_C_QOS)              caps |= 1 << APPLE80211_CAP_WME;

    cd->capabilities[0] = (caps & 0xff);
    cd->capabilities[1] = (caps >> 8) & 0xff;
    
    cd->capabilities[0] = 0xeb;
    cd->capabilities[1] = 0x7e;

    rcx = cd->capabilities[2];

    if (1) {
        cd->capabilities[2] |= 0x13; // 无线网络唤醒;
    } else {
        cd->capabilities[2] |= 0x3;
    }

    rdx |= 0x1;
    rdx |= 0x2;
    rdx |= 0x4;
    if ((rdx & 0x1) != 0x0) {
//        DebugLog("待确认功能1");
        cd->capabilities[2] |= 0x28;
    } else {
        cd->capabilities[2] |= 0x20;
    }

    if ((rdx & 0x2) != 0x0) {
//        DebugLog("待确认功能2");
        cd->capabilities[2] |= 0x4;
    }

    if ((rdx & 0x4) != 0x0) {
//        DebugLog("待确认功能4");
        cd->capabilities[5] |= 0x8;    // 待确认功能
    }


    cd->capabilities[3] |= 0x2;
    if (1) {
        cd->capabilities[4] |= 0x1; // 隔空投送
        cd->capabilities[6] |= 0x8;
    }


    if (0) {
        cd->capabilities[8] |= 0x8;  // checkBoardId
    }

    cd->capabilities[3] |= 0x21;

    rax = cd->capabilities[2];
    cd->capabilities[2] = rax | 0x80;
    if (0) {
        cd->capabilities[5] |= 0x4; // 如果是0x4331，则支持0x4
    }
    if (this->scanReqMultiple) {
        cd->capabilities[2] = rax | 0xc0; // 批量扫描;
    }
    cd->capabilities[6] |= 0x84;

//    if ((OSMetaClassBase::safeMetaCast(r15, **qword_55a050) != 0x0) && (*(int32_t *)(r14 + 0x2f44) >= 0x0)) {
    if (1) {
        cd->capabilities[3] |= 0x8;
    }
//    if (*(*(r14 + 0xa10) + 0x928) != 0x0) {
    if (1) {
//        DebugLog("待确认功能5");
        cd->capabilities[4] |= 0xac;
    }
//    DebugLog("待确认功能6");
    cd->capabilities[6] |= 0x1;
    if (1) {
        cd->capabilities[7] |= 0x4; // 自动解锁
    }

    rax = cd->capabilities[8];
//    if (AirPort_BrcmNIC::isCntryDefaultSupported(r14) != 0x0) {
    if (0) {
//        DebugLog("待确认功能7");
        cd->capabilities[5] |= 0x80;
    }
    else {
//        DebugLog("待确认功能8");
        rax = rax | 0x80;
    }
    cd->capabilities[7] |= 0x80;

    cd->capabilities[8] = rax | 0x40;

//    rcx = cd->capabilities[9];
//    cd->capabilities[9] = rcx | 0x8;
//
////    rdx = 0xaa52;
////    if (rdx != 0xaa52) {
////        if (rdx == 0x4350) {
////            cd->capabilities[9] = rcx | 0x28;
////        }
////    }
////    else {
////        cd->capabilities[9] = rcx | 0x28; // 不发送数据，认证失败
////    }
    
    return kIOReturnSuccess;
    
}

//
// MARK: 13 - STATE
//

IOReturn AirPort_OpenBSD_Class::getSTATE(OSObject *object,
                                     struct apple80211_state_data *ret)
{
    int linkState = 0;
    if (OSDynamicCast(IO80211Interface, object) != NULL) {
        linkState = OSDynamicCast(IO80211Interface, object)->linkState();
    } else {
        linkState = OSDynamicCast(IO80211VirtualInterface, object)->linkState();
    }
    if (linkState == kIO80211NetworkLinkUp) {
        ret->version = APPLE80211_VERSION;
        ret->state = APPLE80211_S_RUN; // this->ic->ic_state;
        return kIOReturnSuccess;
    } else {
        return 0x16;
    }
    
}

//
// MARK: 14 - PHY_MODE
//

IOReturn AirPort_OpenBSD_Class::getPHY_MODE(OSObject *object,
                                        struct apple80211_phymode_data *pd) {
    pd->version = APPLE80211_VERSION;
    
    u_int32_t ic_modecaps= this->ic->ic_modecaps;
    u_int32_t phy_mode  = APPLE80211_MODE_UNKNOWN;
    
    if (ic_modecaps & (1<<IEEE80211_MODE_AUTO))       phy_mode |= APPLE80211_MODE_AUTO;
    if (ic_modecaps & (1<<IEEE80211_MODE_11A))        phy_mode |= APPLE80211_MODE_11A;
    if (ic_modecaps & (1<<IEEE80211_MODE_11B))        phy_mode |= APPLE80211_MODE_11B;
    if (ic_modecaps & (1<<IEEE80211_MODE_11G))        phy_mode |= APPLE80211_MODE_11G;
    if (ic_modecaps & (1<<IEEE80211_MODE_11N))        phy_mode |= APPLE80211_MODE_11N;
    if (ic_modecaps & (1<<IEEE80211_MODE_11AC))       phy_mode |= APPLE80211_MODE_11AC;
    
    u_int32_t ic_curmode= this->ic->ic_curmode;
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

IOReturn AirPort_OpenBSD_Class::getOP_MODE(OSObject *object,
                                       struct apple80211_opmode_data *od) {
    uint32_t op_mode = 0;
    
    switch (this->ic->ic_opmode) {
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

//
// MARK: 16 - RSSI
//

IOReturn AirPort_OpenBSD_Class::getRSSI(OSObject *object,
                                    struct apple80211_rssi_data *rd_data)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if (ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)ifp->rx_tap;
    
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

IOReturn AirPort_OpenBSD_Class::getNOISE(OSObject *object,
                                     struct apple80211_noise_data *nd_data)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if (ifp->rx_tap == NULL)
        return kIOReturnError;
    
    struct rx_radiotap_header *tap =(struct rx_radiotap_header *)ifp->rx_tap;
    
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

IOReturn AirPort_OpenBSD_Class::getINT_MIT(OSObject *object, struct apple80211_intmit_data* imd)
{
    imd->version = APPLE80211_VERSION;
    imd->int_mit = APPLE80211_INT_MIT_AUTO;
    return kIOReturnSuccess;
}

//
// MARK: 19 - POWER
//

IOReturn AirPort_OpenBSD_Class::getPOWER(OSObject *object, struct apple80211_power_data *ret)
{
    ret->version = APPLE80211_VERSION;
    ret->num_radios = 4;
    for (int i = 0; i < ret->num_radios; i++) {
        ret->power_state[i] = this->powerState;
    }

    return kIOReturnSuccess;
}


IOReturn AirPort_OpenBSD_Class::setPOWER(OSObject *object, struct apple80211_power_data *pd)
{
    IOReturn ret = kIOReturnSuccess;

    if (pd->num_radios > 0) {
        ret = this->changePowerState(object, pd->power_state[0]);
    }

    return ret;
}



//
// MARK: 20 - ASSOCIATE
//

IOReturn AirPort_OpenBSD_Class::setASSOCIATE(OSObject *object, struct apple80211_assoc_data *ad)
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
    
    struct ifnet *ifp = &this->ic->ic_if;
    if (!(this->useAppleRSNSupplicant(ifp->iface))) {
        if (this->ic->ic_state < IEEE80211_S_SCAN) {
            return kIOReturnSuccess;
        }
        
        if (this->ic->ic_state == IEEE80211_S_ASSOC || this->ic->ic_state == IEEE80211_S_AUTH) {
            return kIOReturnSuccess;
        }
    }
    
    if (OSDynamicCast(IO80211P2PInterface, object) != NULL) {
        DebugLog("IO80211P2PInterface");
    } else {
        
        if (ad->ad_mode != APPLE80211_AP_MODE_IBSS) {
            
            struct apple80211_authtype_data _authtype_data;
            _authtype_data.version = APPLE80211_VERSION;
            _authtype_data.authtype_lower = ad->ad_auth_lower;
            _authtype_data.authtype_upper = ad->ad_auth_upper;
            this->setAUTH_TYPE(object, &_authtype_data);
            
            struct apple80211_rsn_ie_data rsn_ie_data;
            rsn_ie_data.version = APPLE80211_VERSION;
            rsn_ie_data.len = ad->ad_rsn_ie[1] + 2;
            memcpy(rsn_ie_data.ie, ad->ad_rsn_ie, rsn_ie_data.len);
            this->setRSN_IE(object, &rsn_ie_data);
            
            // OPEN认证           ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 0
            // WEP认证，开放系统    ad_auth_lower = 1, ad_auth_upper = 0, key_cipher_type = 1
            // WEP认证，共享密钥    ad_auth_lower = 2, ad_auth_upper = 0, key_cipher_type = 2
            // WPA认证            ad_auth_lower = 1, ad_auth_upper = 2, key_cipher_type = 6
            // WPA2认证           ad_auth_lower = 1, ad_auth_upper = 8, key_cipher_type = 6
            // 80211X认证         ad_auth_lower = 1, ad_auth_upper = 4, key_cipher_type = 0
            
            this->configArrCount = 0;
            this->configArr[0] = ifp->if_xname;
            this->configArr[1] = "nwid";
            this->configArr[2] = (const char *)ad->ad_ssid;
            this->configArrCount += 3;
            
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
                                    this->configArr[3] = "nwkey";
                                    this->configArr[4] = i_psk;
                                    this->configArrCount += 2;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA_8021X:
                        case APPLE80211_AUTHTYPE_WPA2_8021X:
                        case APPLE80211_AUTHTYPE_SHA256_8021X:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_NONE:
                                    // 企业WPA/WPA2认证
                                    this->configArr[3] = "wpa";
                                    this->configArr[4] = "wpaakms";
                                    this->configArr[5] = "802.1x";
                                    this->configArr[6] = "wpaprotos";
                                    this->configArr[7] = "wpa1,wpa2";
                                    this->configArrCount += 5;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA3_8021X:
                        case APPLE80211_AUTHTYPE_WPA3_FT_8021X:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_NONE:
                                    // 企业WPA2认证
                                    this->configArr[3] = "wpa";
                                    this->configArr[4] = "wpaakms";
                                    this->configArr[5] = "802.1x";
                                    this->configArr[6] = "wpaprotos";
                                    this->configArr[7] = "wpa2";
                                    this->configArrCount += 5;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA_PSK:
                        case APPLE80211_AUTHTYPE_WPA2_PSK:
                        case APPLE80211_AUTHTYPE_SHA256_PSK:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_TKIP:
                                case APPLE80211_CIPHER_PMK:
                                    // WPA/WPA2认证
                                    snprintf(key_tmp, sizeof(key_tmp), "0x%s", i_psk);
                                    this->configArr[3] = "wpakey";
                                    this->configArr[4] = key_tmp;
                                    this->configArr[5] = "wpaprotos";
                                    this->configArr[6] = "wpa1,wpa2";
                                    this->configArrCount += 4;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case APPLE80211_AUTHTYPE_WPA3_SAE:
                        case APPLE80211_AUTHTYPE_WPA3_FT_SAE:
                            switch (ad->ad_key.key_cipher_type) {
                                case APPLE80211_CIPHER_TKIP:
                                case APPLE80211_CIPHER_PMK:
                                    // WPA2认证
                                    snprintf(key_tmp, sizeof(key_tmp), "0x%s", i_psk);
                                    this->configArr[3] = "wpakey";
                                    this->configArr[4] = key_tmp;
                                    this->configArr[5] = "wpaprotos";
                                    this->configArr[6] = "wpa2";
                                    this->configArrCount += 4;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        
                            break;
                        default:
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
                                    this->configArr[3] = "nwkey";
                                    this->configArr[4] = i_psk;
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
            
        }
        
//        while (ic->ic_state <= IEEE80211_S_SCAN) {
//            tsleep_nsec(&ic->ic_state, 0, "ASSOC", MSEC_TO_NSEC(50));
//        }
        
        if (!(this->useAppleRSNSupplicant(ifp->iface))) {
            this->try_times = 10;
            this->disassoc_times = false;

            while (!(this->isConnected() || this->isRun80211X()) && this->try_times-- > 0) {
                tsleep_nsec(&ic->ic_state, PCATCH, "getASSOCIATION_STATUS",
                    SEC_TO_NSEC(1));
            };
        }

    }
    
    this->ic->ic_deauth_reason = APPLE80211_REASON_UNSPECIFIED;
    
    return kIOReturnSuccess;
    
}

//
// MARK: 21 - ASSOCIATE_RESULT
//

IOReturn AirPort_OpenBSD_Class::getASSOCIATE_RESULT(OSObject *object, struct apple80211_assoc_result_data *ad)
{
    ad->version = APPLE80211_VERSION;
    if (this->ic->ic_state == IEEE80211_S_RUN)
        ad->result = APPLE80211_RESULT_SUCCESS;
    else
        ad->result = APPLE80211_RESULT_UNAVAILABLE;
    return kIOReturnSuccess;

}

//
// MARK: 22 - DISASSOCIATE
//

IOReturn AirPort_OpenBSD_Class::setDISASSOCIATE(OSObject *object)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if (!(this->useAppleRSNSupplicant(ifp->iface))) {
        if (this->disassoc_times) {
            this->disassoc_times = false;
            return kIOReturnSuccess;
        }
        
        this->disassoc_times = true;
        
    //    _ifp->iface->postMessage(APPLE80211_M_SSID_CHANGED);
        
        return kIOReturnSuccess;
    }
    
    if (this->ic->ic_state != IEEE80211_S_RUN) {
        return kIOReturnSuccess;
    }
    
    DebugLog("this->ic->ic_des_essid = %s", this->ic->ic_des_essid);
    
    ieee80211_new_state(ic, IEEE80211_S_ASSOC,
        IEEE80211_FC0_SUBTYPE_DISASSOC);
    
    while (ic->ic_state == IEEE80211_S_RUN) {
        tsleep_nsec(&ic->ic_state, 0, "DISASSOC", MSEC_TO_NSEC(50));
    }
    
    this->scanFreeResults();
    
    ieee80211_deselect_ess(this->ic);
    ieee80211_new_state(this->ic, IEEE80211_S_SCAN, -1);
    
    while (ic->ic_state > IEEE80211_S_SCAN) {
        tsleep_nsec(&ic->ic_state, 0, "SCAN", MSEC_TO_NSEC(50));
    }
    
    this->ic->ic_deauth_reason = APPLE80211_REASON_AUTH_LEAVING;
    
    return kIOReturnSuccess;
}

//
// MARK: 27 - SUPPORTED_CHANNELS
//

IOReturn AirPort_OpenBSD_Class::getSUPPORTED_CHANNELS(OSObject *object,
                                                  struct apple80211_sup_channel_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->num_channels = 0;
    for (int i = 0; i < IEEE80211_CHAN_MAX && ad->num_channels < APPLE80211_MAX_CHANNELS; i++) {
        if (this->ic->ic_channels[i].ic_freq != 0) {
            ad->supported_channels[ad->num_channels].channel = ieee80211_chan2ieee(this->ic, &this->ic->ic_channels[i]);
            ad->supported_channels[ad->num_channels].flags   = chanspec2applechannel(this->ic->ic_channels[i].ic_flags, this->ic->ic_channels[i].ic_xflags);
            ad->supported_channels[ad->num_channels].version = APPLE80211_VERSION;
            ad->num_channels++;
        }
    }
    return kIOReturnSuccess;
}

//
// MARK: 28 - LOCALE
//

IOReturn AirPort_OpenBSD_Class::getLOCALE(OSObject *object, struct apple80211_locale_data *ld)
{
    ld->version = APPLE80211_VERSION;
    ld->locale  = APPLE80211_LOCALE_ROW;
    return kIOReturnSuccess;
}

//
// MARK: 29 - DEAUTH
//

IOReturn AirPort_OpenBSD_Class::getDEAUTH(OSObject *object, struct apple80211_deauth_data *dd)
{
    dd->version = APPLE80211_VERSION;
//    dd->deauth_reason = APPLE80211_REASON_ASSOC_LEAVING;
    dd->deauth_reason = this->ic->ic_deauth_reason;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setDEAUTH(OSObject *object, struct apple80211_deauth_data *da)
{
    DebugLog("");
    
    if (this->ic->ic_state < IEEE80211_S_SCAN) {
        return kIOReturnSuccess;
    }
    
    if (this->ic->ic_state > IEEE80211_S_AUTH && this->ic->ic_bss != NULL)
        IEEE80211_SEND_MGMT(this->ic, this->ic->ic_bss, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_AUTH_LEAVE);
    
    this->ic->ic_deauth_reason = APPLE80211_REASON_AUTH_LEAVING;
    
    return kIOReturnSuccess;
}

//
// MARK: 32 - RATE_SET
//

IOReturn AirPort_OpenBSD_Class::getRATE_SET(OSObject *object, struct apple80211_rate_set_data *ad)
{
    if (this->ic->ic_state == IEEE80211_S_RUN) {
        memset(ad, 0, sizeof(*ad));
        ad->version = APPLE80211_VERSION;
        ad->num_rates = this->ic->ic_bss->ni_rates.rs_nrates;
        size_t size = min(this->ic->ic_bss->ni_rates.rs_nrates, nitems(ad->rates));
        for (int i=0; i < size; i++) {
            struct apple80211_rate apple_rate = ad->rates[i];
            apple_rate.version = APPLE80211_VERSION;
            apple_rate.rate = this->ic->ic_bss->ni_rates.rs_rates[i];
            apple_rate.flags = 0;
        }
        return kIOReturnSuccess;
    }
    return kIOReturnError;
}

//
// MARK: 37 - TX_ANTENNA
//

IOReturn AirPort_OpenBSD_Class::getTX_ANTENNA(OSObject *object, apple80211_antenna_data *tx_antenna)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    if (!tx_antenna)
        return kIOReturnError;
    tx_antenna->version = APPLE80211_VERSION;
    
    tx_antenna->num_radios = this->tx_antenna.num_radios;
    for(int i = 0; i < this->tx_antenna.num_radios; i++) {
        tx_antenna->antenna_index[i] = this->tx_antenna.antenna_index[i];
    }
    
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setTX_ANTENNA(OSObject *object, apple80211_antenna_data *tx_antenna)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    if (!tx_antenna)
        return kIOReturnError;
    tx_antenna->version = APPLE80211_VERSION;
    this->tx_antenna.num_radios = tx_antenna->num_radios;
    for(int i = 0; i < this->tx_antenna.num_radios; i++) {
        this->tx_antenna.antenna_index[i] = tx_antenna->antenna_index[i];
    }
    return kIOReturnSuccess;
}

//
// MARK: 39 - ANTENNA_DIVERSITY
//

IOReturn AirPort_OpenBSD_Class::getANTENNA_DIVERSITY(OSObject *object,
                                                 apple80211_antenna_data *ad)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    if (!ad)
        return kIOReturnError;
    ad->version = APPLE80211_VERSION;
    
    ad->num_radios = this->ad.num_radios;
    for(int i = 0; i < this->ad.num_radios; i++) {
        ad->antenna_index[i] = this->ad.antenna_index[i];
    }
    
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setANTENNA_DIVERSITY(OSObject *object,
                                                 apple80211_antenna_data *ad)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    if (!ad)
        return kIOReturnError;
    ad->version = APPLE80211_VERSION;
    this->ad.num_radios = ad->num_radios;
    for(int i = 0; i < this->ad.num_radios; i++) {
        this->ad.antenna_index[i] = ad->antenna_index[i];
    }
    return kIOReturnSuccess;
}



//
// MARK: 43 - DRIVER_VERSION
//

IOReturn AirPort_OpenBSD_Class::getDRIVER_VERSION(OSObject *object, struct apple80211_version_data *hv)
{
    if (!hv)
        return kIOReturnError;
    struct ifnet *ifp = &this->ic->ic_if;
    
    hv->version = APPLE80211_VERSION;
    
    char fwname[256];
    snprintf(fwname, sizeof(fwname), "%s %s", PRODUCT_NAME, ifp->fwver);
    
    strncpy(hv->string, fwname, sizeof(hv->string));
    hv->string_len = strlen(fwname);
    return kIOReturnSuccess;
}

//
// MARK: 44 - HARDWARE_VERSION
//

IOReturn AirPort_OpenBSD_Class::getHARDWARE_VERSION(OSObject *object, struct apple80211_version_data *hv)
{
    if (!hv)
        return kIOReturnError;
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, "Hardware 1.0", sizeof(hv->string));
    hv->string_len = strlen("Hardware 1.0");
    return kIOReturnSuccess;
}

//
// MARK: 46 - RSN_IE
//

IOReturn AirPort_OpenBSD_Class::getRSN_IE(OSObject *object, struct apple80211_rsn_ie_data *data)
{
    if (this->ic->ic_bss == NULL || this->ic->ic_bss->ni_rsnie == NULL) {
        return kIOReturnError;
    }
    data->version = APPLE80211_VERSION;
    if (this->ic->ic_rsnie[1] > 0) {
        data->len = 2 + this->ic->ic_rsnie[1];
        memcpy(data->ie, this->ic->ic_rsnie, data->len);
    } else {
        data->len = 2 + this->ic->ic_bss->ni_rsnie[1];
        memcpy(data->ie, this->ic->ic_bss->ni_rsnie, data->len);
    }
    return kIOReturnSuccess;
}

//
// MARK: 46 - RSN_IE
//

IOReturn AirPort_OpenBSD_Class::setRSN_IE(OSObject *object, struct apple80211_rsn_ie_data *data)
{
    if (!data)
        return kIOReturnError;
    static_assert(sizeof(this->ic->ic_rsnie) == APPLE80211_MAX_RSN_IE_LEN, "Max RSN IE length mismatch");
    memcpy(this->ic->ic_rsnie, data->ie, APPLE80211_MAX_RSN_IE_LEN);
    if (this->ic->ic_state == IEEE80211_S_RUN && this->ic->ic_bss != nullptr)
        ieee80211_save_ie(data->ie, &this->ic->ic_bss->ni_rsnie);
    return kIOReturnSuccess;
}

//
// MARK: 48 - AP_IE_LIST
//

IOReturn AirPort_OpenBSD_Class::getAP_IE_LIST(OSObject *object, struct apple80211_ap_ie_data *data)
{
    if (this->ic->ic_bss == NULL || this->ic->ic_bss->ni_ie == NULL) {
        return kIOReturnError;
    }
    data->version = APPLE80211_VERSION;
    data->len = this->ic->ic_bss->ni_ie_len;
    memcpy(data->ie_data, this->ic->ic_bss->ni_ie, data->len);
    return kIOReturnSuccess;
}

//
// MARK: 49 - STATS
//

IOReturn AirPort_OpenBSD_Class::getSTATS(OSObject *object, struct apple80211_stats_data *data)
{
    if (this->ic->ic_bss == NULL || this->ic->ic_bss->ni_ie == NULL) {
        return kIOReturnError;
    }
    struct ifnet *ifp = &this->ic->ic_if;
    
    data->version = APPLE80211_VERSION;
    
    data->tx_frame_count = ifp->if_opackets;
    data->tx_errors = ifp->if_oerrors;
    data->rx_frame_count = ifp->if_ipackets;
    data->rx_errors = ifp->if_ierrors;
    
    return kIOReturnSuccess;
}

//
// MARK: 50 - ASSOCIATION_STATUS
//

IOReturn AirPort_OpenBSD_Class::getASSOCIATION_STATUS(OSObject *object, struct apple80211_assoc_status_data* ret)
{
    struct ifnet *ifp = &this->ic->ic_if;
    if (!(this->useAppleRSNSupplicant(ifp->iface))) {
        ret->version = APPLE80211_VERSION;
        ret->status = APPLE80211_STATUS_SUCCESS;
        
        if (this->isRun80211X()) {
            struct ieee80211com *ic = (struct ieee80211com *)_ifp;
            ieee80211_set_link_state(ic, LINK_STATE_UP);
            this->disassoc_times = true;
        } else if (this->isConnected()) {
            this->disassoc_times = true;
        } else {
            ret->status = APPLE80211_STATUS_UNAVAILABLE;
        }

        return kIOReturnSuccess;
    }
    
    ret->version = APPLE80211_VERSION;
    if (this->ic->ic_state == IEEE80211_S_RUN)
        ret->status = APPLE80211_STATUS_SUCCESS;
    else
        ret->status = APPLE80211_STATUS_UNAVAILABLE;

    return kIOReturnSuccess;
}

//
// MARK: 51 - COUNTRY_CODE
//

IOReturn AirPort_OpenBSD_Class::getCOUNTRY_CODE(OSObject *object, struct apple80211_country_code_data *ccd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL || this->ic->ic_bss->ni_countryie == NULL) {
        *ccd = this->ccd;
        return kIOReturnSuccess;
    }
    
    ccd->version = APPLE80211_VERSION;
    bcopy(&this->ic->ic_bss->ni_countryie[2], ccd->cc, APPLE80211_MAX_CC_LEN);
    
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setCOUNTRY_CODE(OSObject *object, struct apple80211_country_code_data *ccd)
{
    struct ifnet *ifp = &this->ic->ic_if;
    this->ccd = *ccd;
    
    ifp->iface->postMessage(APPLE80211_M_COUNTRY_CODE_CHANGED);
    
    return kIOReturnSuccess;
}

//
// MARK: 54 - RADIO_INFO
//

IOReturn AirPort_OpenBSD_Class::getRADIO_INFO(OSObject *object, struct apple80211_radio_info_data* md)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    if (!md)
        return kIOReturnError;
    md->version = APPLE80211_VERSION;
    md->count = 1;
    return kIOReturnSuccess;
}

//
// MARK: 57 - MCS
//
IOReturn AirPort_OpenBSD_Class::getMCS(OSObject *object, struct apple80211_mcs_data* md)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    md->version = APPLE80211_VERSION;
    md->index = this->ic->ic_bss->ni_txmcs;
    return kIOReturnSuccess;
}

//
// MARK: 65 - PHY_SUB_MODE
//

IOReturn AirPort_OpenBSD_Class::getPHY_SUB_MODE(OSObject *object, struct apple80211_physubmode_data* md)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    
    md->version = APPLE80211_VERSION;
    if (md->phy_mode == APPLE80211_MODE_11N) {
        md->phy_submode = 0x1;
        md->flags = 0x1e;
    }
    
    return kIOReturnSuccess;
}

//
// MARK: 66 - MCS_INDEX_SET
//

IOReturn AirPort_OpenBSD_Class::getMCS_INDEX_SET(OSObject *object, struct apple80211_mcs_index_set_data* md)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    memset(md, 0, sizeof(*md));
    md->version = APPLE80211_VERSION;
    size_t size = min(nitems(this->ic->ic_bss->ni_rxmcs), nitems(md->mcs_set_map));
    for (int i = 0; i < size; i++) {
        md->mcs_set_map[i] = this->ic->ic_bss->ni_rxmcs[i];
    }
    return kIOReturnSuccess;
}

//
// MARK: 80 - ROAM_THRESH
//

IOReturn AirPort_OpenBSD_Class::getROAM_THRESH(OSObject *object, struct apple80211_roam_threshold_data* rtd)
{
//    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
//        return kIOReturnError;
    rtd->threshold = 100;
    rtd->count = 0;
    return kIOReturnSuccess;
}

//
// MARK: 90 - SCANCACHE_CLEAR
//

IOReturn AirPort_OpenBSD_Class::setSCANCACHE_CLEAR(OSObject *object, struct device *dev)
{
    DebugLog("----");
    
    if (this->getNetworkInterface()->linkState() != kIO80211NetworkLinkUp) {
        this->scanFreeResults();
    }
    
    //if doing background or active scan, don't free nodes.
    if ((this->ic->ic_flags & IEEE80211_F_BGSCAN) || (this->ic->ic_flags & IEEE80211_F_ASCAN)) {
        return kIOReturnSuccess;
    }
    ieee80211_free_allnodes(this->ic, 0);
    return kIOReturnSuccess;
}

//
// MARK: 107 - ROAM
//

IOReturn AirPort_OpenBSD_Class::setROAM(OSObject *object, struct apple80211_sta_roam_data *data)
{
    return kIOReturnError;
}

//
// MARK: 156 - LINK_CHANGED_EVENT_DATA
//

IOReturn AirPort_OpenBSD_Class::getLINK_CHANGED_EVENT_DATA(OSObject *object, struct apple80211_link_changed_event_data *ed) {
    if (ed == NULL)
        return 16;

    bzero(ed, sizeof(*ed));
    
    ed->isLinkDown = this->ic->ic_state!= IEEE80211_S_RUN;
    struct apple80211_rssi_data rd_data;
    getRSSI(object, &rd_data);
    
    ed->rssi = rd_data.rssi[0];
    if (ed->isLinkDown) {
        ed->voluntary = true;
        ed->reason = APPLE80211_LINK_DOWN_REASON_DEAUTH;
    } else {
        ed->rssi = -(100 - this->ic->ic_bss->ni_rssi);
    }
    
    return kIOReturnSuccess;
}

//
// MARK: 181 - APPLE80211_IOC_VHT_MCS_INDEX_SET
//

IOReturn AirPort_OpenBSD_Class::getVHT_MCS_INDEX_SET(OSObject *object, struct apple80211_vht_mcs_index_set_data *data)
{
    if (this->ic->ic_bss == NULL || this->ic->ic_curmode < IEEE80211_MODE_11AC) {
        return kIOReturnError;
    }
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->mcs_map = this->ic->ic_bss->ni_vht_txmcs;
    
    return kIOReturnSuccess;
}

//
// MARK: 195 - MCS_VHT
//

IOReturn AirPort_OpenBSD_Class::getMCS_VHT(OSObject *object, struct apple80211_mcs_vht_data *data)
{
    if (this->ic->ic_bss == NULL || this->ic->ic_curmode < IEEE80211_MODE_11AC) {
        return kIOReturnError;
    }
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->guard_interval = (ieee80211_node_supports_vht_sgi80(this->ic->ic_bss) || ieee80211_node_supports_vht_sgi160(this->ic->ic_bss)) ? APPLE80211_GI_SHORT : APPLE80211_GI_LONG;
    data->index = this->ic->ic_bss->ni_txmcs;
    data->nss = this->ic->ic_bss->ni_vht_ss;
    
    u_int32_t flags = chanspec2applechannel(this->ic->ic_bss->ni_chan->ic_flags, this->ic->ic_bss->ni_chan->ic_xflags);
    
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

IOReturn AirPort_OpenBSD_Class::setMCS_VHT(OSObject *object, struct apple80211_mcs_vht_data *data)
{
    return kIOReturnError;
}

//
// MARK: 196 - TX_NSS
//

IOReturn AirPort_OpenBSD_Class::getTX_NSS(OSObject *object, struct apple80211_tx_nss_data *data)
{
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->nss = this->ic->ic_bss->ni_vht_ss;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::setTX_NSS(OSObject *object, struct apple80211_tx_nss_data *data)
{
    return kIOReturnError;
}

//
// MARK: 353 - NSS
//

IOReturn AirPort_OpenBSD_Class::getNSS(OSObject *object, struct apple80211_nss_data *data)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL)
        return kIOReturnError;
    
    memset(data, 0, sizeof(*data));
    data->version = APPLE80211_VERSION;
    data->nss = this->ic->ic_bss->ni_vht_ss;
    return kIOReturnSuccess;
}

IOReturn AirPort_OpenBSD_Class::apple80211Request(UInt32 request_type,
                                            int request_number,
                                            IOInterface* interface,
                                            void* data) {

    if (request_type != SIOCGA80211 && request_type != SIOCSA80211) {
        DebugLog("AirPort_OpenBSD: Invalid IOCTL request type: %u", request_type);
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
//        case 90:
//            DebugLog("IOCTL %s(%d)", isGet ? "get" : "set", request_number);
//            break;
//        default:
////    DebugLog("IOCTL %s(%d)", isGet ? "get" : "set", request_number);
////    DebugLog("IOCTL %s(%d) %s", isGet ? "get" : "set",  request_number, IOCTL_NAMES[request_number]);
//            break;
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
            IOCTL_SET(request_type, CIPHER_KEY, apple80211_key);
            break;
        case APPLE80211_IOC_CHANNEL: // 4
            IOCTL(request_type, CHANNEL, apple80211_channel_data);
            break;
        case APPLE80211_IOC_POWERSAVE: // 5
            IOCTL(request_type, POWERSAVE, apple80211_powersave_data);
            break;
        case APPLE80211_IOC_PROTMODE: // 6
            IOCTL_GET(request_type, PROTMODE, apple80211_protmode_data);
            break;
        case APPLE80211_IOC_TXPOWER: // 7
            IOCTL(request_type, TXPOWER, apple80211_txpower_data);
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
            IOCTL(request_type, DEAUTH, apple80211_deauth_data);
            break;
//        case APPLE80211_IOC_FRAG_THRESHOLD: // 31
//            IOCTL_GET(request_type, FRAG_THRESHOLD, apple80211_frag_threshold_data);
//            break;
        case APPLE80211_IOC_RATE_SET: // 32
            IOCTL_GET(request_type, RATE_SET, apple80211_rate_set_data);
            break;
        case APPLE80211_IOC_TX_ANTENNA: // 37
            IOCTL(request_type, TX_ANTENNA, apple80211_antenna_data);
            break;
        case APPLE80211_IOC_ANTENNA_DIVERSITY: // 39
            IOCTL(request_type, ANTENNA_DIVERSITY, apple80211_antenna_data);
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
            IOCTL(request_type, RSN_IE, apple80211_rsn_ie_data);
            break;
        case APPLE80211_IOC_AP_IE_LIST: // 48
            IOCTL_GET(request_type, AP_IE_LIST, apple80211_ap_ie_data);
            break;
        case APPLE80211_IOC_STATS: // 49
            IOCTL_GET(request_type, STATS, apple80211_stats_data);
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
        case APPLE80211_IOC_PHY_SUB_MODE: // 65
            IOCTL_GET(request_type, PHY_SUB_MODE, apple80211_physubmode_data);
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
//            IOCTL(request_type, IE, apple80211_ie_data);
//            break;
        case APPLE80211_IOC_SCAN_REQ_MULTIPLE: // 86
            IOCTL_SET(request_type, SCAN_REQ_MULTIPLE, apple80211_scan_multiple_data);
            break;
        case APPLE80211_IOC_SCANCACHE_CLEAR: // 90
            IOCTL_SET(request_type, SCANCACHE_CLEAR, device);
            break;
//        case APPLE80211_IOC_VIRTUAL_IF_CREATE: // 94
//            IOCTL_SET(request_type, VIRTUAL_IF_CREATE, apple80211_virt_if_create_data);
//            break;
//        case APPLE80211_IOC_VIRTUAL_IF_DELETE: // 95
//            IOCTL_SET(request_type, VIRTUAL_IF_DELETE, apple80211_virt_if_delete_data);
//            break;
        case APPLE80211_IOC_ROAM: // 107
            IOCTL_SET(request_type, ROAM, apple80211_sta_roam_data);
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
        case APPLE80211_IOC_VHT_MCS_INDEX_SET: // 181
            IOCTL_GET(request_type, VHT_MCS_INDEX_SET, apple80211_vht_mcs_index_set_data);
            break;
        case APPLE80211_IOC_MCS_VHT: // 195
            IOCTL(request_type, MCS_VHT, apple80211_mcs_vht_data);
        case APPLE80211_IOC_TX_NSS: // 196
            IOCTL(request_type, TX_NSS, apple80211_tx_nss_data);
            break;
//        case APPLE80211_IOC_ROAM_PROFILE: // 216
//            IOCTL(request_type, ROAM_PROFILE, apple80211_roam_profile_band_data);
//            break;
//        case APPLE80211_IOC_BTCOEX_PROFILES: // 221
//            IOCTL(request_type, BTCOEX_PROFILES, apple80211_btc_profiles_data);
//            break;
//        case APPLE80211_IOC_BTCOEX_CONFIG: // 222
//            IOCTL(request_type, BTCOEX_CONFIG, apple80211_btc_config_data);
//            break;
//        case APPLE80211_IOC_BTCOEX_OPTIONS: // 235
//            IOCTL(request_type, BTCOEX_OPTIONS, apple80211_btc_options_data);
//            break;
//        case APPLE80211_IOC_MAX_NSS_FOR_AP: // 259
//            IOCTL(request_type, MAX_NSS_FOR_AP, apple80211_btc_options_data);
//            break;
//        case APPLE80211_IOC_BTCOEX_MODE: // 87
//            IOCTL(request_type, BTCOEX_MODE, apple80211_btc_mode_data);
//            break;
        case APPLE80211_IOC_NSS: // 353
            IOCTL_GET(request_type, NSS, apple80211_nss_data);
            break;
            
//        case APPLE80211_IOC_P2P_LISTEN: // 92
//            IOCTL_SET(request_type, P2P_LISTEN, apple80211_p2p_listen_data);
//            break;
//        case APPLE80211_IOC_P2P_SCAN: // 93
//            IOCTL_SET(request_type, P2P_SCAN, apple80211_scan_data);
//            break;
//        case APPLE80211_IOC_P2P_GO_CONF: // 98
//            IOCTL_SET(request_type, P2P_GO_CONF, apple80211_p2p_go_conf_data);
//            break;
//
//        case APPLE80211_IOC_WOW_PARAMETERS: // 69
//            IOCTL(request_type, WOW_PARAMETERS, apple80211_wow_parameter_data);
//            break;
            
        case APPLE80211_IOC_BLOCK_ACK: // 62
        case APPLE80211_IOC_VENDOR_DBG_FLAGS: // 81
        case APPLE80211_IOC_CDD_MODE: // 109
        case APPLE80211_IOC_FACTORY_MODE: // 112
        case APPLE80211_IOC_ACL_POLICY: // 169
        case APPLE80211_IOC_ACL_LIST: // 173
        case APPLE80211_IOC_CHAIN_ACK: // 174
        case APPLE80211_IOC_INTERRUPT_STATS: // 183
        case APPLE80211_IOC_INTERRUPT_STATS_RESET: // 184
        case APPLE80211_IOC_TIMER_STATS: // 185
        case APPLE80211_IOC_TIMER_STATS_RESET: // 186
        case APPLE80211_IOC_CHANNELS_INFO: // 207
        case APPLE80211_IOC_LEAKY_AP_STATS_MODE: // 230
        case APPLE80211_IOC_COUNTRY_CHANNELS: // 237
        case APPLE80211_IOC_PRIVATE_MAC: // 238
        case APPLE80211_IOC_LLW_PARAMS: // 344
            DebugLog("IOCTL %s(%d)", isGet ? "get" : "set", request_number);
        default:
//            DPRINTF(("Unhandled Airport GET request %u\n", request_number));
            ret = kIOReturnUnsupported;
    }
#undef IOCTL
    
    return ret;
}
