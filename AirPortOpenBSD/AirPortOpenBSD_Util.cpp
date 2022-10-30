//
//  AirPortOpenBSD_Util.cpp
//  AirPortOpenBSD
//
//  Created by User-PC on 2020/8/10.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

void AirPortOpenBSD::firmwareLoadComplete( OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context) {
    AirPortOpenBSD *dev = (AirPortOpenBSD *)context;
    if(result == kOSReturnSuccess) {
        dev->firmwareData = OSData::withBytes(resourceData, resourceDataLength);
    } else
        printf("firmwareLoadComplete FAILURE: %08x.\n", result);
    IOLockWakeup(dev->fwLoadLock, dev, true);
}

void AirPortOpenBSD::firmwareLoadComplete(const char* name) {
    for (int i = 0; i < firmwares_total; i++) {
        if (strcmp(firmwares[i].name, name) == 0) {
            firmware fw = firmwares[i];
            this->firmwareData = OSData::withBytes(fw.data, fw.size);
            return;
        }
    }
}

int AirPortOpenBSD::loadfirmware(const char *firmware_name, u_char **bufp, size_t *buflen)
{
//    IOLockLock(this->fwLoadLock);
//
//    OSReturn ret = OSKextRequestResource(OSKextGetCurrentIdentifier(),
//                                         firmware_name,
//                                         firmwareLoadComplete,
//                                         this,
//                                         NULL);
//    if(ret != kOSReturnSuccess) {
//        IOLog("%s Unable to load firmware file %08x\n", __FUNCTION__, ret);
//        IOLockUnlock(this->fwLoadLock);
//        return 1;
//    }
//    IOLockSleep(this->fwLoadLock, this, THREAD_INTERRUPTIBLE);
//    IOLockUnlock(this->fwLoadLock);
    
    firmwareLoadComplete(firmware_name);
    
    *buflen = this->firmwareData->getLength();
    *bufp = (u_char *)malloc(*buflen, M_DEVBUF, M_NOWAIT);
    memcpy(*bufp , (u_char*)this->firmwareData->getBytesNoCopy(), *buflen);
    
    this->firmwareData->release();
    this->firmwareData = NULL;
    
//    *bufp = (u_char *)this->firmwareData->getBytesNoCopy();
    
    return 0;
}

void AirPortOpenBSD::if_input(struct ifnet* ifp, struct mbuf_list *ml)
{
    int packets = 0;
    mbuf_t m;
    while ((m = ml_dequeue(ml)) != NULL) {
        if (ifp->iface == NULL) {
            panic("%s ifq->iface == NULL!!!\n", __FUNCTION__);
            break;
        }
//        ifp->fInterface->inputPacket(m, mbuf_len(m), IONetworkInterface::kInputOptionQueuePacket, 0);
        int err = this->enqueueInputPacket2(m);
        if (err != kIOReturnSuccess)
            continue;
        
        packets ++;
        if (ifp->netStat != NULL) {
            ifp->if_ipackets++;
        }
    }
    
    if (packets)
        this->flushInputQueue2();
}

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


bool AirPortOpenBSD::isConnected()
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    
    return ic->ic_state == IEEE80211_S_RUN &&
    (ic->ic_opmode != IEEE80211_M_STA ||
     !(ic->ic_flags & IEEE80211_F_RSNON) ||
     ic->ic_bss->ni_port_valid);
}

bool AirPortOpenBSD::isRun80211X()
{
    struct ieee80211com *ic = (struct ieee80211com *)_ifp;
    
    return ic->ic_state == IEEE80211_S_RUN &&
    (ic->ic_opmode != IEEE80211_M_STA || (ic->ic_rsnakms & IEEE80211_AKM_8021X || ic->ic_rsnakms & IEEE80211_AKM_SHA256_8021X));
}

struct ieee80211_nodereq* AirPortOpenBSD::findScanResult(apple80211_assoc_data* ad)
{
    // find the scan result corresponding to the given assoc params
    if (!ad) return NULL;

    
    bool emptyBSSID = (strncmp((char*)&ad->ad_bssid, (char *)empty_macaddr, APPLE80211_ADDR_LEN) == 0);
    int i = 0;
    while (i < this->scanResults->getCount()) {
        OSObject* scanObj = this->scanResults->getObject(i++);
        if (scanObj == NULL) {
            continue;
        }
        
        OSData* scanresult = OSDynamicCast(OSData, scanObj);
        struct ieee80211_nodereq *na_node = (struct ieee80211_nodereq *)scanresult->getBytesNoCopy();
        
        if (!emptyBSSID && strncmp((char*) &ad->ad_bssid, (char*) na_node->nr_bssid, APPLE80211_ADDR_LEN) == 0) {
            return na_node;
        }
        
        if (strncmp((char*) na_node->nr_nwid,
                (char*) ad->ad_ssid,
                min(ad->ad_ssid_len, na_node->nr_nwid_len)) != 0)
        {
            continue;
        } else {
            return na_node;
        }
    }
    return NULL;
}

IOReturn AirPortOpenBSD::scanConvertResult(struct ieee80211_nodereq *nr, struct apple80211_scan_result* oneResult)
{
    bzero(oneResult, sizeof(apple80211_scan_result));

    oneResult->version = APPLE80211_VERSION;

    oneResult->asr_channel.version = APPLE80211_VERSION;
    oneResult->asr_channel.channel = nr->nr_channel;
    oneResult->asr_channel.flags = chanspec2applechannel(nr->nr_chan_flags, nr->nr_chan_xflags);

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
    
    oneResult->asr_age = (uint32_t)(airport_up_time() - nr->nr_age_ts);;
    oneResult->asr_ie_len = 0;
    if (nr->nr_ie != NULL && nr->nr_ie_len > 0) {
        oneResult->asr_ie_len = nr->nr_ie_len;
#if MontereyKernel > MacKernel
        oneResult->asr_ie_data = nr->nr_ie;
//        oneResult->asr_ie_data = IOMalloc(oneResult->asr_ie_len);
//        bcopy(nr->nr_ie, oneResult->asr_ie_data, oneResult->asr_ie_len);
#else
        memcpy(oneResult->asr_ie_data, nr->nr_ie, min(oneResult->asr_ie_len, sizeof(oneResult->asr_ie_data)));
#endif
    }

    return 0;
}

void AirPortOpenBSD::scanComplete()
{
    struct ieee80211_nodereq_all *na = (typeof na)IOMalloc(sizeof(*na));
    struct ieee80211_nodereq *nr = (typeof nr)IOMalloc(512 * sizeof(*nr));
    int i;

    bzero(na, sizeof(*na));
    bzero(nr, 512 * sizeof(*nr));
    na->na_node = nr;
    na->na_size = 512 * sizeof(*nr);
    strlcpy(na->na_ifname, "AirPortOpenBSD", strlen("AirPortOpenBSD"));

    if (ioctl(0, SIOCG80211ALLNODES, na) != 0) {
        warn("SIOCG80211ALLNODES");
        return;
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
        
        OSData* scanresult = OSData::withBytes(na_node, sizeof(*na_node));
        if (!scanresult) {
            continue;
        }
        
        this->scanResults->setObject(scanresult);

        RELEASE(scanresult);
    }

    if (this->scanResults->getCount() == 0) {
        this->ca->ca_activate((struct device *)if_softc, DVACT_WAKEUP);
    }

    IOFree(na, sizeof(*na));
    IOFree(nr, 512 * sizeof(*nr));
}

void AirPortOpenBSD::scanFreeResults()
{
    this->scanResults->flushCollection();
    this->scanIndex = 0;
    return;
}

void AirPortOpenBSD::scanDone(OSObject *owner, ...)
{
    struct device *dev = (struct device *)_ifp->if_softc;
    
//    dev->dev->scanFreeResults();
    dev->dev->scanComplete();
#ifndef Ethernet
    _ifp->iface->postMessage(APPLE80211_M_SCAN_DONE);
//    _ifp->iface->postMessage(APPLE80211_M_COUNTRY_CODE_CHANGED);
#endif
}

OSString *AirPortOpenBSD::getNVRAMProperty(char *name)
{
    OSString *value = OSString::withCString("");
    char val[120];
    int ret = 0;
    IOLog("Start Get NARAM value %s", name);
    if (IORegistryEntry *nvram = OSDynamicCast(IORegistryEntry, IODTNVRAM::fromPath("/options", gIODTPlane)))
    {
        if (OSData *buf = OSDynamicCast(OSData, nvram->getProperty(name))) {
            buf->appendByte('\0', 1);
            
            ret = snprintf(val, buf->getLength(), "%s", buf->getBytesNoCopy());
            IOLog("Get NARAM value %s=%s successed ret = %d", name,val, ret);
            if (ret == -1)
                return value;
            value = OSString::withCString(val);
            
            IOLog("Get NARAM value %s=%s successed", name,val);
        }else
        {
             IOLog("Get NARAM value %s failed: Cna't getProperty %s", name, name);
        }
        nvram->release();
    } else {
        IOLog("Get NARAM value %s failed: Can't get /options", name);
    }
    return value;
}
