//
//  AirPort_OpenBSD_SCAN.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/7.
//

#include "AirPort_OpenBSD.hpp"

int AirPort_OpenBSD::chanspec2applechannel(int ic_flags, int ic_xflags)
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


IOReturn AirPort_OpenBSD::scanConvertResult(struct ieee80211_nodereq *nr, struct apple80211_scan_result* oneResult)
{
    bzero(oneResult, sizeof(apple80211_scan_result));

    oneResult->version = APPLE80211_VERSION;

    oneResult->asr_channel.version = APPLE80211_VERSION;
    oneResult->asr_channel.channel = nr->nr_channel;
    oneResult->asr_channel.flags = chanspec2applechannel(nr->nr_chan_flags, nr->nr_chan_xflags);

    oneResult->asr_use = 1;
    oneResult->asr_noise = 0;
    oneResult->asr_rssi = nr->nr_rssi - 100;
    oneResult->asr_snr = oneResult->asr_rssi - oneResult->asr_noise;
    oneResult->asr_beacon_int = nr->nr_intval;
    oneResult->asr_cap = nr->nr_capinfo;
    bcopy(nr->nr_bssid, oneResult->asr_bssid.octet, IEEE80211_ADDR_LEN);
    oneResult->asr_nrates = nr->nr_nrates;
    for (int r = 0; r < oneResult->asr_nrates; r++)
        oneResult->asr_rates[r] = (nr->nr_rates[r] & IEEE80211_RATE_VAL) / 2;
    oneResult->asr_ssid_len = nr->nr_nwid_len;
    bcopy(nr->nr_nwid, oneResult->asr_ssid, oneResult->asr_ssid_len);
    
    oneResult->asr_age = 0;
    if (nr->nr_age_ts != 0) {
        oneResult->asr_age = (uint32_t)(sysuptime() - nr->nr_age_ts);
    }
    oneResult->asr_ie_len = 0;
    if (nr->nr_ie != NULL && nr->nr_ie_len > 0) {
        oneResult->asr_ie_len = nr->nr_ie_len;
#if MAC_TARGET < __MAC_12_0
        oneResult->asr_ie_data = IOMalloc(oneResult->asr_ie_len);
        if (oneResult->asr_ie_data != NULL) {
            memcpy(oneResult->asr_ie_data, nr->nr_ie, oneResult->asr_ie_len);
        } else {
            oneResult->asr_ie_len = 0;
        }
#else
        memcpy(oneResult->asr_ie_data, nr->nr_ie, min(oneResult->asr_ie_len, sizeof(oneResult->asr_ie_data)));
#endif
    }

    return 0;
}

IOReturn AirPort_OpenBSD::scanComplete()
{
    struct ieee80211_nodereq_all *na = (typeof na)IOMalloc(sizeof(*na));
    struct ieee80211_nodereq *nr = (typeof nr)IOMalloc(512 * sizeof(*nr));
    int i;

    bzero(na, sizeof(*na));
    bzero(nr, 512 * sizeof(*nr));
    na->na_node = nr;
    na->na_size = 512 * sizeof(*nr);
    strlcpy(na->na_ifname, this->getName(), strlen(this->getName()));

    if (ioctl(0, SIOCG80211ALLNODES, na) != 0) {
        IOFree(na, sizeof(*na));
        IOFree(nr, 512 * sizeof(*nr));
        
        warn("SIOCG80211ALLNODES");
        return kIOReturnError;
    }

    if (!na->na_nodes)
        printf("\t\tnone\n");
    else
        qsort(nr, na->na_nodes, sizeof(*nr), rssicmp);

    for (i = 0; i < na->na_nodes; i++) {
        struct ieee80211_nodereq *na_node = na->na_node + i;

        if (strncmp((char *)na_node->nr_nwid, "", sizeof(na_node->nr_nwid)) == 0)
        {
            continue;
        }
        
        // 查找有没有在缓存里
        bool found = false;
        
        struct apple80211_scan_result_list *scan_result_list, *tmp;
        SLIST_FOREACH_SAFE(scan_result_list, &this->scan_result_lists, list, tmp) {
            // 查找ssid和channel
            if (memcmp((char*) na_node->nr_nwid, (char*)scan_result_list->scan_result.asr_ssid, max(scan_result_list->scan_result.asr_ssid_len, na_node->nr_nwid_len)) == 0 && na_node->nr_channel == scan_result_list->scan_result.asr_channel.channel) {
                // 找到
                found = true;
                break;
            }
        }
        
        if (found) {
            // 已存在, 更新
            // 跳过已连接的ssid
//                if (memcmp((char*) na_node->nr_nwid, (char*)assoc_ssid.ssid, max(assoc_ssid.ssid_len, na_node->nr_nwid_len)) != 0) {
//                    // 找不到才更新
            this->scanConvertResult(na_node, &scan_result_list->scan_result);
//                }
            continue;
        }
        
        // 新增扫描结果
        scan_result_list = (typeof scan_result_list)IOMalloc(sizeof(struct apple80211_scan_result_list));
        
        this->scanConvertResult(na_node, &scan_result_list->scan_result);
        
        SLIST_INSERT_HEAD(&this->scan_result_lists, scan_result_list, list);
        
    }

    IOFree(na, sizeof(*na));
    IOFree(nr, 512 * sizeof(*nr));
    
    return kIOReturnSuccess;
}

void AirPort_OpenBSD::scanFreeResults()
{
    while (!SLIST_EMPTY(&this->scan_result_lists)) {
        struct apple80211_scan_result_list *scan_result_list = SLIST_FIRST(&this->scan_result_lists);
        SLIST_REMOVE_HEAD(&this->scan_result_lists, list);
        IOFree(scan_result_list, sizeof(struct apple80211_scan_result_list));
    }
    
    while (!SLIST_EMPTY(&this->known_ssid_lists)) {
        struct apple80211_ssid_data_known_list *known_ssid_list = SLIST_FIRST(&this->known_ssid_lists);
        SLIST_REMOVE_HEAD(&this->known_ssid_lists, list);
        IOFree(known_ssid_list, sizeof(struct apple80211_ssid_data_known_list));
    }
    
    bzero(&this->scan_ssid, sizeof(this->scan_ssid));
    
    return;
}

void AirPort_OpenBSD::apple80211_scan_done(OSObject *owner, IOTimerEventSource *sender)
{
    AirPort_OpenBSD *that = (AirPort_OpenBSD *)owner;
    that->postMessage(APPLE80211_M_SCAN_DONE);
}
