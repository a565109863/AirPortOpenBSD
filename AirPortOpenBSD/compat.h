//
//  compat.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/19.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef compat_h
#define compat_h

#include <sys/endian.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_radiotap.h>

struct rx_radiotap_header {
    struct ieee80211_radiotap_header wr_ihdr;
    uint64_t    wr_tsft;
    uint8_t        wr_flags;
    uint8_t        wr_rate;
    uint16_t    wr_chan_freq;
    uint16_t    wr_chan_flags;
    int8_t        wr_dbm_antsignal;
    int8_t        wr_dbm_antnoise;
} __packed;

struct tx_radiotap_header {
    struct ieee80211_radiotap_header wt_ihdr;
    uint8_t        wt_flags;
    uint8_t        wt_rate;
    uint16_t    wt_chan_freq;
    uint16_t    wt_chan_flags;
    uint8_t        wt_hwqueue;
} __packed;

static __inline uint64_t
SEC_TO_NSEC(uint64_t seconds)
{
    if (seconds > UINT64_MAX / 1000000000ULL)
        return UINT64_MAX;
    return seconds * 1000000000ULL;
}

static __inline uint64_t
MSEC_TO_NSEC(uint64_t milliseconds)
{
    if (milliseconds > UINT64_MAX / 1000000ULL)
        return UINT64_MAX;
    return milliseconds * 1000000ULL;
}

static char* hexdump(uint8_t *buf, size_t len)
{
    ssize_t str_len = len * 3 + 1;
    char *str = (char*)IOMalloc(str_len);
    if (!str)
        return nullptr;
    for (size_t i = 0; i < len; i++)
    snprintf(str + 3 * i, (len - i) * 3, "%02x ", buf[i]);
    str[MAX(str_len - 2, 0)] = 0;
    return str;
}


void interrupt_func(OSObject *owner, IOInterruptEventSource *src, int count);

int loadfirmware(const char *name, u_char **bufp, size_t *buflen);

void ether_ifattach(struct ifnet *ifp);
void ether_ifdetach(struct ifnet *ifp);

int tsleep_nsec(void* chan,  int pri,  const char* wmesg,  int timo);
void wakeup_sleep(void *chan, bool one);
#define wakeup(x) wakeup_sleep(x, false);
#define wakeup_one(x) wakeup_sleep(x, true);

static u_int8_t empty_macaddr[IEEE80211_ADDR_LEN] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include "AirPort_OpenBSD.hpp"

#endif /* compat_h */
