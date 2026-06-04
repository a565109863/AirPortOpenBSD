//
//  apple80211.h
//  AirPortOpenBSD
//
//  Created by Zhong on 2019/5/11.
//  Copyright © 2019 Roman Peshkov. All rights reserved.
//

#ifndef apple80211_h
#define apple80211_h

#if __MAC_X_VERSION >= __MAC_15_0
#define PRODUCT_NAME                            "AirPortOpenBSD_Sequoia"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Sequoia
#define AirPortOpenBSD                          AirPortOpenBSD_Sequoia
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Sequoia
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Sequoia
#define IOTimeout                               IOTimeout_Sequoia
#define pci_intr_handle                         pci_intr_handle_Sequoia
#elif __MAC_X_VERSION >= __MAC_14_4
#define PRODUCT_NAME                            "AirPortOpenBSD_Sonoma14_4"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Sonoma14_4
#define AirPortOpenBSD                          AirPortOpenBSD_Sonoma14_4
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Sonoma14_4
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Sonoma14_4
#define IOTimeout                               IOTimeout_Sonoma14_4
#define pci_intr_handle                         pci_intr_handle_Sonoma14_4
#elif __MAC_X_VERSION >= __MAC_14_0
#define PRODUCT_NAME                            "AirPortOpenBSD_Sonoma14_0"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Sonoma14_0
#define AirPortOpenBSD                          AirPortOpenBSD_Sonoma14_0
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Sonoma14_0
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Sonoma14_0
#define IOTimeout                               IOTimeout_Sonoma14_0
#define pci_intr_handle                         pci_intr_handle_Sonoma14_0
#elif __MAC_X_VERSION >= __MAC_13_0
#define PRODUCT_NAME                            "AirPortOpenBSD_Ventura"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Ventura
#define AirPortOpenBSD                          AirPortOpenBSD_Ventura
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Ventura
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Ventura
#define IOTimeout                               IOTimeout_Ventura
#define pci_intr_handle                         pci_intr_handle_Ventura
#elif __MAC_X_VERSION >= __MAC_12_0
#define PRODUCT_NAME                            "AirPortOpenBSD_Monterey"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Monterey
#define AirPortOpenBSD                          AirPortOpenBSD_Monterey
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Monterey
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Monterey
#define IOTimeout                               IOTimeout_Monterey
#define pci_intr_handle                         pci_intr_handle_Monterey
#elif __MAC_X_VERSION >= __MAC_11_0
#define PRODUCT_NAME                            "AirPortOpenBSD_BigSur"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_BigSur
#define AirPortOpenBSD                          AirPortOpenBSD_BigSur
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_BigSur
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_BigSur
#define IOTimeout                               IOTimeout_BigSur
#define pci_intr_handle                         pci_intr_handle_BigSur
#elif __MAC_X_VERSION >= __MAC_10_15
#define PRODUCT_NAME                            "AirPortOpenBSD_Catalina"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Catalina
#define AirPortOpenBSD                          AirPortOpenBSD_Catalina
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Catalina
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Catalina
#define IOTimeout                               IOTimeout_Catalina
#define pci_intr_handle                         pci_intr_handle_Catalina
#elif __MAC_X_VERSION >= __MAC_10_14
#define PRODUCT_NAME                            "AirPortOpenBSD_Mojave"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Mojave
#define AirPortOpenBSD                          AirPortOpenBSD_Mojave
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_Mojave
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_Mojave
#define IOTimeout                               IOTimeout_Mojave
#define pci_intr_handle                         pci_intr_handle_Mojave
#elif __MAC_X_VERSION >= __MAC_10_13
#define PRODUCT_NAME                            "AirPortOpenBSD_HighSierra"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_HighSierra
#define AirPortOpenBSD                          AirPortOpenBSD_HighSierra
#define AirPortOpenBSDWLANSkywalkInterface      AirPortOpenBSDWLANSkywalkInterface_HighSierra
#define AirPortOpenBSDWLANEthernetInterface     AirPortOpenBSDWLANEthernetInterface_HighSierra
#define IOTimeout                               IOTimeout_HighSierra
#define pci_intr_handle                         pci_intr_handle_HighSierra
#else
#error "Unsupported __MAC_X_VERSION value"
#endif

#include "apple_private_spi.h"
#include "debug.h"
#include "IO80211WorkLoop.h"

#if __MAC_X_VERSION >= __MAC_14_0
#include "IO80211WorkQueue.h"
#include "IO80211Controller.h"
#include "IO80211InfraInterface.h"
#include "IO80211InfraProtocol.h"
#include "IOSkywalkPacketBufferPool.h"
#include "IOSkywalkLegacyEthernetInterface.h"
#include "IO80211SkywalkInterface.h"
#include "AirPortOpenBSDWLANSkywalkInterface.hpp"
#include "AirPortOpenBSDWLANEthernetInterface.hpp"
#else
#include "apple80211/Legacy/IO80211ControllerLegacy.h"
#include "apple80211/Legacy/IO80211Interface.h"
#include "apple80211/Legacy/IO80211VirtualInterface.h"
#include "apple80211/Legacy/IO80211P2PInterface.h"
#include "Legacy/AirPortOpenBSDWLANLegacyEthernetInterface.hpp"
#endif


#define IOController                IO80211Controller
//#define WorkLoop                    IO80211WorkLoop

#define kIONetworkLinkUndefined     kIO80211NetworkLinkUndefined
#define kIONetworkLinkDown          kIO80211NetworkLinkDown
#define kIONetworkLinkUp            kIO80211NetworkLinkUp

#define APPLE_POWER_OFF             APPLE80211_POWER_OFF
#define APPLE_POWER_ON              APPLE80211_POWER_ON
#define APPLE_POWER_TX              APPLE80211_POWER_TX
#define APPLE_POWER_RX              APPLE80211_POWER_RX

#endif /* apple80211_h */
