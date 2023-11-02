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
#define MAC_VERSION_MAJOR_Sonoma        23

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Sonoma
#define PRODUCT_NAME                            "AirPortOpenBSD_Sonoma"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Sonoma
#define AirPortOpenBSD                          AirPortOpenBSD_Sonoma
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_Sonoma_Interface
#define IOTimeout                               IOTimeout_Sonoma
#define pci_intr_handle                         pci_intr_handle_Sonoma
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Ventura
#define PRODUCT_NAME                            "AirPortOpenBSD_Ventura"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Ventura
#define AirPortOpenBSD                          AirPortOpenBSD_Ventura
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_Ventura_Interface
#define IOTimeout                               IOTimeout_Ventura
#define pci_intr_handle                         pci_intr_handle_Ventura
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Monterey
#define PRODUCT_NAME                            "AirPortOpenBSD_Monterey"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Monterey
#define AirPortOpenBSD                          AirPortOpenBSD_Monterey
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_Monterey_Interface
#define IOTimeout                               IOTimeout_Monterey
#define pci_intr_handle                         pci_intr_handle_Monterey
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_BigSur
#define PRODUCT_NAME                            "AirPortOpenBSD_BigSur"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_BigSur
#define AirPortOpenBSD                          AirPortOpenBSD_BigSur
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_BigSur_Interface
#define IOTimeout                               IOTimeout_BigSur
#define pci_intr_handle                         pci_intr_handle_BigSur
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Catalina
#define PRODUCT_NAME                            "AirPortOpenBSD_Catalina"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Catalina
#define AirPortOpenBSD                          AirPortOpenBSD_Catalina
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_Catalina_Interface
#define IOTimeout                               IOTimeout_Catalina
#define pci_intr_handle                         pci_intr_handle_Catalina
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_Mojave
#define PRODUCT_NAME                            "AirPortOpenBSD_Mojave"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_Mojave
#define AirPortOpenBSD                          AirPortOpenBSD_Mojave
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_Mojave_Interface
#define IOTimeout                               IOTimeout_Mojave
#define pci_intr_handle                         pci_intr_handle_Mojave
#endif

#if MAC_VERSION_MAJOR == MAC_VERSION_MAJOR_HighSierra
#define PRODUCT_NAME                            "AirPortOpenBSD_HighSierra"
#define AirPortOpenBSDWLANBusInterfacePCIe      AirPortOpenBSDWLANBusInterfacePCIe_HighSierra
#define AirPortOpenBSD                          AirPortOpenBSD_HighSierra
#define AirPortOpenBSD_EthernetInterface        AirPortOpenBSD_HighSierra_Interface
#define IOTimeout                               IOTimeout_HighSierra
#define pci_intr_handle                         pci_intr_handle_HighSierra
#endif

#include "apple_private_spi.h"
#include "debug.h"
#include "IO80211WorkLoop.h"

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "IO80211WorkQueue.h"
#include "IO80211Controller.h"
#include "IO80211InfraInterface.h"
#include "IO80211InfraProtocol.h"
#include "IOSkywalkPacketBufferPool.h"
#include "IOSkywalkLegacyEthernetInterface.h"
#include "IO80211SkywalkInterface.h"
#include "AirPortOpenBSD_SkywalkInterface.hpp"
#include "AirPortOpenBSD_EthernetInterface.hpp"
#else
#include "apple80211/Legacy/IO80211ControllerLegacy.h"
#include "apple80211/Legacy/IO80211Interface.h"
#include "apple80211/Legacy/IO80211VirtualInterface.h"
#include "apple80211/Legacy/IO80211P2PInterface.h"
#include "Legacy/AirPortOpenBSDLegacy_EthernetInterface.hpp"
#endif


#define IOController                IO80211Controller
//#define IOInterface                 IO80211Interface
//#define WorkLoop                    IO80211WorkLoop

#define kIONetworkLinkUndefined     kIO80211NetworkLinkUndefined
#define kIONetworkLinkDown          kIO80211NetworkLinkDown
#define kIONetworkLinkUp            kIO80211NetworkLinkUp

#define APPLE_POWER_OFF             APPLE80211_POWER_OFF
#define APPLE_POWER_ON              APPLE80211_POWER_ON
#define APPLE_POWER_TX              APPLE80211_POWER_TX
#define APPLE_POWER_RX              APPLE80211_POWER_RX

#endif /* apple80211_h */
