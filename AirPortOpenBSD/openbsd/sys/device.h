//
//  device.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/16.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef device_h
#define device_h

#include <sys/_kernel.h>
#include <machine/intr.h>
#include <sys/refcnt.h>

#define PCI_MSIX_QUEUES    16

#include "AirPort_OpenBSD.hpp"

class AirPort_OpenBSD_Class;
struct device {
    AirPort_OpenBSD_Class* dev;
    pci_intr_handle_class *ih[PCI_MSIX_QUEUES];
    u_int           ih_count;
    char dv_xname[16];
};

#endif /* device_h */
