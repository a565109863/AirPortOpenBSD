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

int AirPortOpenBSD::chanspec2applechannel(int flags)
{
    int ret = 0;
    if (flags & IEEE80211_CHAN_2GHZ)    ret |= APPLE80211_C_FLAG_2GHZ;
    if (flags & IEEE80211_CHAN_5GHZ)    ret |= APPLE80211_C_FLAG_5GHZ;
    if (!(flags & IEEE80211_CHAN_PASSIVE))    ret |= APPLE80211_C_FLAG_ACTIVE;
    if (flags & IEEE80211_CHAN_OFDM)    ret |= APPLE80211_C_FLAG_20MHZ; // XXX ??
    if (flags & IEEE80211_CHAN_CCK)        ret |= APPLE80211_C_FLAG_10MHZ; // XXX ??
    if (flags & IEEE80211_CHAN_VHT)        ret |= APPLE80211_C_FLAG_5GHZ; // XXX ??
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

//    scanFreeResults();
    scanComplete();
    
    if (scanResults->getCount() == 0) return NULL;
    
    bool emptyBSSID = (strncmp((char*)&ad->ad_bssid, (char *)empty_macaddr, APPLE80211_ADDR_LEN) == 0);
    
    for (int i = 0; i < scanResults->getCount(); i++) {
        OSObject* scanObj = scanResults->getObject(scanIndex++);
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
    oneResult->asr_channel.flags = chanspec2applechannel(nr->nr_chan_flags);

    oneResult->asr_noise = 0;
    oneResult->asr_rssi = nr->nr_rssi - 100;
    oneResult->asr_snr = oneResult->asr_rssi - oneResult->asr_noise;
    oneResult->asr_beacon_int = nr->nr_intval;
    oneResult->asr_cap = nr->nr_capinfo;
    bcopy(nr->nr_bssid, oneResult->asr_bssid.octet, IEEE80211_ADDR_LEN);
    oneResult->asr_nrates = nr->nr_nrates;
    for (int r = 0; r < oneResult->asr_nrates; r++)
        oneResult->asr_rates[r] = nr->nr_rates[r];
    oneResult->asr_ssid_len = nr->nr_nwid_len;
    bcopy(nr->nr_nwid, oneResult->asr_ssid, oneResult->asr_ssid_len);

    oneResult->asr_age = 0;
    oneResult->asr_ie_len = 0;
    if (nr->nr_ie != NULL && nr->nr_ie_len > 0) {
        oneResult->asr_ie_len = nr->nr_ie_len;
        oneResult->asr_ie_data = nr->nr_ie;
//        oneResult->asr_ie_data = IOMalloc(oneResult->asr_ie_len);
//        bcopy(nr->nr_ie, oneResult->asr_ie_data, oneResult->asr_ie_len);
    }

    return 0;
}

void AirPortOpenBSD::scanComplete()
{
    int i;
    struct ieee80211_nodereq_all *na = (struct ieee80211_nodereq_all *)IOMalloc(sizeof(struct ieee80211_nodereq_all));
    struct ieee80211_nodereq *nr = (struct ieee80211_nodereq *)IOMalloc(512 * sizeof(struct ieee80211_nodereq));

    na->na_node = nr;
    na->na_size = 512 * sizeof(struct ieee80211_nodereq);
    strlcpy(na->na_ifname, "AirPortOpenBSD", strlen("AirPortOpenBSD"));

    if (_ifp->if_ioctl(_ifp, SIOCG80211ALLNODES, (caddr_t)na) != 0) {
        return;
    }

    for (i = 0; i < na->na_nodes; i++) {
        struct ieee80211_nodereq *na_node = na->na_node + i;

        if (strcmp((char *)na_node->nr_nwid, "") == 0)
        {
            continue;
        }
        
        OSData* scanresult = OSData::withBytes(na_node, sizeof(*na_node));
        if (!scanresult) {
            continue;
        }
        
        scanResults->setObject(scanresult);

        RELEASE(scanresult);
    }
    
    IOFree(na, sizeof(struct ieee80211_nodereq_all));
    IOFree(nr, 512 * sizeof(struct ieee80211_nodereq));
}

void AirPortOpenBSD::scanFreeResults()
{
    scanResults->flushCollection();
    scanIndex = 0;
    return;
}

void AirPortOpenBSD::scanDone(OSObject *owner, ...)
{
    struct device *dev = (struct device *)_ifp->if_softc;
    
//    dev->dev->scanFreeResults();
    dev->dev->scanComplete();
#ifndef Ethernet
    _ifp->iface->postMessage(APPLE80211_M_SCAN_DONE);
    _ifp->iface->postMessage(APPLE80211_M_COUNTRY_CODE_CHANGED);
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
