//
//  AirPortOpenBSDComm_SCAN.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/7.
//

#include <Availability.h>

#include "apple80211.h"

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "AirPortOpenBSD.hpp"
#else
#include "AirPortOpenBSDLegacy.hpp"
#endif

int AirPortOpenBSD::chanspec2applechannel(int ic_flags, int ic_xflags)
{
    struct ieee80211_channel ni_chan = {0, ic_flags, ic_xflags};
    
    int ret = 0;
    
    if (IEEE80211_IS_CHAN_2GHZ(&ni_chan)) {
        ret |= APPLE80211_C_FLAG_2GHZ;
    }
    
    if (IEEE80211_IS_CHAN_5GHZ(&ni_chan)) {
        ret |= APPLE80211_C_FLAG_5GHZ;
    }
    
    if (IEEE80211_CHAN_160MHZ_ALLOWED(&ni_chan)) {
        ret |= APPLE80211_C_FLAG_160MHZ;
    } else if (IEEE80211_CHAN_80MHZ_ALLOWED(&ni_chan)) {
        ret |= APPLE80211_C_FLAG_80MHZ;
    } else if (IEEE80211_CHAN_40MHZ_ALLOWED(&ni_chan)) {
        ret |= APPLE80211_C_FLAG_40MHZ;
    } else {
        ret |= APPLE80211_C_FLAG_20MHZ;
    }
    
//    if (!(ic_flags & IEEE80211_CHAN_PASSIVE))
//        ret |= APPLE80211_C_FLAG_ACTIVE;
    
    // 0x400 0x204 0x2  0x4 0x1 0x8 0x10 0x100
    return ret;//0x400 |0x204| 0x2|  0x4| 0x1| 0x8| 0x10| 0x100 | 0x20 | 0x40 | 0x80;
}

IOReturn AirPortOpenBSD::scanConvertResult(struct ieee80211_node *ni, struct apple80211_scan_result *oneResult)
{
    bzero(oneResult, sizeof(*oneResult));

    oneResult->version = APPLE80211_VERSION;

    oneResult->asr_channel.version = APPLE80211_VERSION;
    oneResult->asr_channel.channel = ieee80211_chan2ieee(this->ic, ni->ni_chan);
    oneResult->asr_channel.flags = this->chanspec2applechannel(ni->ni_chan->ic_flags, ni->ni_chan->ic_xflags);

    oneResult->asr_use = 1;
    oneResult->asr_noise = 0;
    oneResult->asr_rssi = ni->ni_rssi - 100;
    oneResult->asr_snr = oneResult->asr_rssi - oneResult->asr_noise;
    oneResult->asr_beacon_int = ni->ni_intval;
    oneResult->asr_cap = ni->ni_capinfo;
    IEEE80211_ADDR_COPY(oneResult->asr_bssid.octet, ni->ni_bssid);
    oneResult->asr_nrates = ni->ni_rates.rs_nrates;
    for (int r = 0; r < oneResult->asr_nrates; r++)
        oneResult->asr_rates[r] = (ni->ni_rates.rs_rates[r] & IEEE80211_RATE_VAL) / 2;
    oneResult->asr_ssid_len = ni->ni_esslen;
    bcopy(ni->ni_essid, oneResult->asr_ssid, oneResult->asr_ssid_len);
    
    oneResult->asr_age = 0;
    if (ni->ni_age_ts != 0) {
        oneResult->asr_age = (uint32_t)(sysuptime() - ni->ni_age_ts);
    }
    oneResult->asr_ie_len = 0;
    if (ni->ni_ie != NULL && ni->ni_ie_len > 0) {
        oneResult->asr_ie_len = ni->ni_ie_len;
#if MAC_VERSION_MAJOR < MAC_VERSION_MAJOR_Monterey
        oneResult->asr_ie_data = IOMalloc(oneResult->asr_ie_len);
        if (oneResult->asr_ie_data != NULL) {
            memcpy(oneResult->asr_ie_data, ni->ni_ie, oneResult->asr_ie_len);
        } else {
            oneResult->asr_ie_len = 0;
        }
#else
        memcpy(oneResult->asr_ie_data, ni->ni_ie, min(oneResult->asr_ie_len, sizeof(oneResult->asr_ie_data)));
#endif
    }
    
    return 0;
}

IOReturn AirPortOpenBSD::scanComplete()
{
    struct ieee80211_node *ni = RBT_MIN(ieee80211_tree, &ic->ic_tree);
    while (ni) {
        if (strncmp((char *)ni->ni_essid, "", sizeof(ni->ni_essid)) != 0)
        {
            struct timeval tv;
            microtime(&tv);
            
            // 查找有没有在缓存里
            bool found = false;
            
            struct apple80211_scan_result_list *scan_result_list, *tmp;
            SLIST_FOREACH_SAFE(scan_result_list, &this->scan_result_lists, list, tmp) {
                // 查找bssid和channel
                if (IEEE80211_ADDR_EQ(ni->ni_bssid, scan_result_list->scan_result.asr_bssid.octet) && ieee80211_chan2ieee(this->ic, ni->ni_chan) == scan_result_list->scan_result.asr_channel.channel) {
                    // 找到
                    found = true;
                    break;
                }
            }
            
            if (found) {
                // 已存在, 更新
                scan_result_list->asr_age_ts = tv.tv_sec;
                this->scanConvertResult(ni, &scan_result_list->scan_result);
            } else {
                // 新增扫描结果
                scan_result_list = (typeof scan_result_list)IOMalloc(sizeof(struct apple80211_scan_result_list));
                scan_result_list->asr_age_ts = tv.tv_sec;
                this->scanConvertResult(ni, &scan_result_list->scan_result);

                SLIST_INSERT_HEAD(&this->scan_result_lists, scan_result_list, list);
            }
            
        }
        
        ni = RBT_NEXT(ieee80211_tree, ni);
    }
    
    return kIOReturnSuccess;
}

void AirPortOpenBSD::scanFreeResults(uint64_t asr_age_ts)
{
    DebugLog("this->ic->ic_state = %d", this->ic->ic_state);
    
    struct timeval tv;
    microtime(&tv);
    
    struct apple80211_scan_result_list *scan_result_list, *tmp;
    SLIST_FOREACH_SAFE(scan_result_list, &this->scan_result_lists, list, tmp) {
        if (asr_age_ts == 0 || tv.tv_sec - scan_result_list->asr_age_ts > asr_age_ts) {
            SLIST_REMOVE(&this->scan_result_lists, scan_result_list, apple80211_scan_result_list, list);
            IOFree(scan_result_list, sizeof(struct apple80211_scan_result_list));
        }
    }
    
    return;
}

void AirPortOpenBSD::apple80211_scan_done(OSObject *owner, IOTimerEventSource *sender)
{
    AirPortOpenBSD *that = (AirPortOpenBSD *)owner;
    
    that->scanComplete();
    
    that->scan_result_next = SLIST_FIRST(&that->scan_result_lists);
    
    UInt32 msg = 0;
    that->postMessage(APPLE80211_M_SCAN_DONE, &msg, 4);
}
