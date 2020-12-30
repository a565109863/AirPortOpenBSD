//
//  apple80211.h
//  AirPortOpenBSD
//
//  Created by Zhong on 2019/5/11.
//  Copyright © 2019 Roman Peshkov. All rights reserved.
//

#ifndef apple80211_h
#define apple80211_h

#ifdef Ethernet

#include <net/bpf.h>
#include "apple80211/apple80211_ioctl.h"

//#include <net/ethernet.h>

#define IOController IOEthernetController
#define IOInterface IONetworkInterface
#define WorkLoop IOWorkLoop

#define kIONetworkLinkUndefined     0
#define kIONetworkLinkDown          1
#define kIONetworkLinkUp            2

#define APPLE_POWER_OFF             0
#define APPLE_POWER_ON              1
#define APPLE_POWER_TX              2
#define APPLE_POWER_RX              3

#else

#include "apple80211/IO80211Controller.h"
#include "apple80211/IO80211Interface.h"

#define IOController IO80211Controller
#define IOInterface IO80211Interface
#define WorkLoop IO80211WorkLoop

#define kIONetworkLinkUndefined     kIO80211NetworkLinkUndefined
#define kIONetworkLinkDown          kIO80211NetworkLinkDown
#define kIONetworkLinkUp            kIO80211NetworkLinkUp

#define APPLE_POWER_OFF             APPLE80211_POWER_OFF
#define APPLE_POWER_ON              APPLE80211_POWER_ON
#define APPLE_POWER_TX              APPLE80211_POWER_TX
#define APPLE_POWER_RX              APPLE80211_POWER_RX

#endif

#endif /* apple80211_h */
