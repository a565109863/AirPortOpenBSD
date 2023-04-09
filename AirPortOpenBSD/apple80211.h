//
//  apple80211.h
//  AirPortOpenBSD
//
//  Created by Zhong on 2019/5/11.
//  Copyright © 2019 Roman Peshkov. All rights reserved.
//

#ifndef apple80211_h
#define apple80211_h

#define MAC_VERSION_MAJOR_HighSierra    17
#define MAC_VERSION_MAJOR_Mojave        18
#define MAC_VERSION_MAJOR_Catalina      19
#define MAC_VERSION_MAJOR_BigSur        20
#define MAC_VERSION_MAJOR_Monterey      21
#define MAC_VERSION_MAJOR_Ventura       22

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Ventura
#define AirPort_OpenBSD_Class           AirPortOpenBSD_Ventura
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_Ventura_Interface
#define IOTimeout_Class                 IOTimeout_Ventura
#define pci_intr_handle_class           pci_intr_handle_Ventura
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Monterey
#define AirPort_OpenBSD_Class           AirPortOpenBSD_Monterey
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_Monterey_Interface
#define IOTimeout_Class                 IOTimeout_Monterey
#define pci_intr_handle_class           pci_intr_handle_Monterey
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_BigSur
#define AirPort_OpenBSD_Class           AirPortOpenBSD_BigSur
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_BigSur_Interface
#define IOTimeout_Class                 IOTimeout_BigSur
#define pci_intr_handle_class           pci_intr_handle_BigSur
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Catalina
#define AirPort_OpenBSD_Class           AirPortOpenBSD_Catalina
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_Catalina_Interface
#define IOTimeout_Class                 IOTimeout_Catalina
#define pci_intr_handle_class           pci_intr_handle_Catalina
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Mojave
#define AirPort_OpenBSD_Class           AirPortOpenBSD_Mojave
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_Mojave_Interface
#define IOTimeout_Class                 IOTimeout_Mojave
#define pci_intr_handle_class           pci_intr_handle_Mojave
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_HighSierra
#define AirPort_OpenBSD_Class           AirPortOpenBSD_HighSierra
#define AirPort_OpenBSD_Class_Interface AirPortOpenBSD_HighSierra_Interface
#define IOTimeout_Class                 IOTimeout_HighSierra
#define pci_intr_handle_class           pci_intr_handle_HighSierra
#endif

#include "apple_private_spi.h"
#include "debug.h"
#include "IO80211WorkLoop.h"
#include "IO80211Controller.h"
#include "IO80211Interface.h"
#include "IO80211VirtualInterface.h"
#include "IO80211P2PInterface.h"
#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Catalina
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
