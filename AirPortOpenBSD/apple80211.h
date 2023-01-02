//
//  apple80211.h
//  AirPortOpenBSD
//
//  Created by Zhong on 2019/5/11.
//  Copyright © 2019 Roman Peshkov. All rights reserved.
//

#ifndef apple80211_h
#define apple80211_h

#include "apple_private_spi.h"
#include "debug.h"
#include "IO80211WorkLoop.h"
#include "IO80211Controller.h"
#include "IO80211Interface.h"
#include "IO80211VirtualInterface.h"
#include "IO80211P2PInterface.h"
#if __IO80211_TARGET >= __MAC_10_15
#include "IO80211SkywalkInterface.h"
#endif
#include "AirPort_OpenBSD_Interface.hpp"

#define IOController                IO80211Controller
#define IOInterface                 IO80211Interface
#define WorkLoop                    IO80211WorkLoop

#define kIONetworkLinkUndefined     kIO80211NetworkLinkUndefined
#define kIONetworkLinkDown          kIO80211NetworkLinkDown
#define kIONetworkLinkUp            kIO80211NetworkLinkUp

#define APPLE_POWER_OFF             APPLE80211_POWER_OFF
#define APPLE_POWER_ON              APPLE80211_POWER_ON
#define APPLE_POWER_TX              APPLE80211_POWER_TX
#define APPLE_POWER_RX              APPLE80211_POWER_RX

#endif /* apple80211_h */
