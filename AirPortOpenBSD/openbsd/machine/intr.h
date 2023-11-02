//
//  intr.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/13.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef intr_h
#define intr_h

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "AirPortOpenBSD.hpp"
#else
#include "AirPortOpenBSDLegacy.hpp"
#endif

class pci_intr_handle : public OSObject {
    OSDeclareDefaultStructors(pci_intr_handle)
public:
    IOWorkLoop*        workloop;
    IOInterruptEventSource*    intr;
    IOPCIDevice*        dev;
    int (*func)(void* arg);
    void* arg;
    int ih;
    const char *intrstr;
};

#endif /* intr_h */
