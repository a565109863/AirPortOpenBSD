//
//  pcivar.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef pcivar_h
#define pcivar_h

#include <sys/device.h>


#define PCI_INTR_INTX        0x00000000
#define PCI_INTR_MSI        0x80000000
#define PCI_INTR_MSIX        0x40000000
#define PCI_INTR_TYPE_MASK    0xc0000000
#define PCI_INTR_TYPE(_ih)    ((_ih) & PCI_INTR_TYPE_MASK)

#define PCI_INTR_TAG_MASK    0x00ffff00
#define PCI_INTR_TAG(_ih)    ((_ih) & PCI_INTR_TAG_MASK)

#define PCI_INTR_VEC_MASK    0x000000ff
#define PCI_INTR_VEC(_ih)    ((_ih) & PCI_INTR_VEC_MASK)
typedef u_int pci_intr_handle_t;


#define PCITAG_OFFSET(x)    ((x) & 0xffffffff)

//typedef pci_intr_handle* pci_intr_handle_t;

struct pci_matchid {
    pci_vendor_id_t        pm_vid;
    pci_product_id_t    pm_pid;
};


/*
 * PCI device attach arguments.
 */
struct pci_attach_args {
    device dev;
        bus_space_tag_t pa_iot;        /* pci i/o space tag */
        bus_space_tag_t pa_memt;    /* pci mem space tag */
//    bus_dma_tag_t pa_dmat;        /* DMA tag */
//        pci_chipset_tag_t pa_pc;
    int        pa_flags;    /* flags; see below */
    
    //    struct extent    *pa_ioex;
    //    struct extent    *pa_memex;
    //    struct extent    *pa_pmemex;
    //    struct extent    *pa_busex;
    
    u_int           pa_domain;
    u_int           pa_bus;
    u_int        pa_device;
    u_int        pa_function;
    //    pcitag_t    pa_tag;
    pcireg_t    pa_id, pa_class;
    
    //    pcitag_t    *pa_bridgetag;
    pci_intr_handle_t *pa_bridgeih;
    
    /*
     * Interrupt information.
     *
     * "Intrline" is used on systems whose firmware puts
     * the right routing data into the line register in
     * configuration space.  The rest are used on systems
     * that do not.
     */
    u_int        pa_intrswiz;    /* how to swizzle pins if ppb */
    //    pcitag_t    pa_intrtag;    /* intr. appears to come from here */
    //    pci_intr_pin_t    pa_intrpin;    /* intr. appears on this pin */
    //    pci_intr_line_t    pa_intrline;    /* intr. routing information */
    //    pci_intr_pin_t    pa_rawintrpin;    /* unswizzled pin */
    
    //
    UInt16 vendor;
    UInt16 device;
    UInt16 subsystem_device;
    UInt16 maxSnoop;
    UInt16 maxNoSnoop;
    UInt8 revision;
    
    IOWorkLoop*        workloop;
    pci_chipset_tag_t    pa_pc;
    pcitag_t        pa_tag;
    bus_dma_tag_t        pa_dmat;
};


/*
 * Flags given in the bus and device attachment args.
 *
 * OpenBSD doesn't actually use them yet -- csapuntz@cvs.openbsd.org
 */
#define    PCI_FLAGS_IO_ENABLED    0x01        /* I/O space is enabled */
#define    PCI_FLAGS_MEM_ENABLED    0x02        /* memory space is enabled */
#define    PCI_FLAGS_MRL_OKAY    0x04        /* Memory Read Line okay */
#define    PCI_FLAGS_MRM_OKAY    0x08        /* Memory Read Multiple okay */
#define    PCI_FLAGS_MWI_OKAY    0x10        /* Memory Write and Invalidate
okay */
#define    PCI_FLAGS_MSI_ENABLED    0x20        /* Message Signaled Interrupt
enabled */

int pci_matchbyid(struct pci_attach_args *, const struct pci_matchid *, int);
int    pci_get_capability(pci_chipset_tag_t, pcitag_t, int,
        int *, pcireg_t *);
pcireg_t    pci_mapreg_type(pci_chipset_tag_t pc, pcitag_t tag, int reg);
int    pci_mapreg_map(struct pci_attach_args *, int, pcireg_t, int,
        bus_space_tag_t *, bus_space_handle_t *, bus_addr_t *,
        bus_size_t *, bus_size_t);

pcireg_t    pci_conf_read(pci_chipset_tag_t, pcitag_t, int);
void        pci_conf_write(pci_chipset_tag_t, pcitag_t, int,
                    pcireg_t);
int        pci_intr_map(struct pci_attach_args *, pci_intr_handle_t *);
int        pci_intr_map_msi(struct pci_attach_args *, pci_intr_handle_t *);
int        pci_intr_map_msix(struct pci_attach_args *, int,
            pci_intr_handle_t *);
void        pci_intr_unmap(struct pci_attach_args *, pci_intr_handle_t *);
int        pci_intr_line(pci_chipset_tag_t, pci_intr_handle_t);
const char    *pci_intr_string(pci_chipset_tag_t, pci_intr_handle_t);
void        *pci_intr_establish(pci_chipset_tag_t, pci_intr_handle_t,
                 int, int (*)(void *), void *, const char *);
void        pci_intr_disestablish(pci_chipset_tag_t, void *);

int pci_intr_index(struct pci_attach_args *pa, pci_intr_handle_t *ihp);


void pci_enable_msi(pci_chipset_tag_t pc, pcitag_t tag);
void pci_disable_msi(pci_chipset_tag_t pc, pcitag_t tag);
void pci_enable_msix(pci_chipset_tag_t pc, pcitag_t tag);
void pci_disable_msix(pci_chipset_tag_t pc, pcitag_t tag);


#define    pci_set_powerstate_md(c, t, s, p)
int pci_set_powerstate(pci_chipset_tag_t, pcitag_t, int);


int pci_intr_vec_count(struct pci_attach_args *pa, int vec);

#endif /* pcivar_h */
