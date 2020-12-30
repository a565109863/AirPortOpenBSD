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

class AirPortOpenBSD;
struct device {
    AirPortOpenBSD* dev;
    pci_intr_handle *ih[PCI_MSIX_QUEUES];
    u_int           ih_count;
    char dv_xname[16];
};

#endif /* device_h */
