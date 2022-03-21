//
//  pci.c
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <sys/_kernel.h>
#include <machine/bus.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include "AirPortOpenBSD.hpp"

#undef KASSERT
#define KASSERT(x)        assert(x)

#define PCI_MSIX_VEC_MASK    0xff
#define PCI_MSIX_VEC(pin)    ((pin) & PCI_MSIX_VEC_MASK)

pcireg_t
pci_conf_read(pci_chipset_tag_t pc, pcitag_t tag, int reg)
{
    return tag->configRead32(reg);
}

void
pci_conf_write(pci_chipset_tag_t pc, pcitag_t tag, int reg, pcireg_t data)
{
    tag->configWrite32(reg, data);
}

pcireg_t pci_mapreg_type(pci_chipset_tag_t pc, pcitag_t tag, int reg) {
    return kIOMapInhibitCache;
}

int
pci_mapreg_map(struct pci_attach_args *pa, int reg, pcireg_t type, int flags,
    bus_space_tag_t *tagp, bus_space_handle_t *handlep, bus_addr_t *basep,
    bus_size_t *sizep, bus_size_t maxsize)
{
    IOMemoryMap* map = pa->pa_tag->mapDeviceMemoryWithRegister(reg, type);
    if (map == 0)
        return kIOReturnError;
    
    *handlep = reinterpret_cast<caddr_t>(map->getVirtualAddress());
    
    if (tagp)
        *tagp = map;
    if (basep)
        *basep = map->getVirtualAddress();
    if (sizep)
        *sizep = map->getSize();
    
    return 0;
}

int
pci_matchbyid(struct pci_attach_args *pa, const struct pci_matchid *ids,
    int nent)
{
    const struct pci_matchid *pm;
    int i;

    for (i = 0, pm = ids; i < nent; i++, pm++)
        if (PCI_VENDOR(pa->pa_id) == pm->pm_vid &&
            PCI_PRODUCT(pa->pa_id) == pm->pm_pid)
            return (1);
    return (0);
}

int
pci_get_capability(pci_chipset_tag_t pc, pcitag_t tag, int capid,
    int *offset, pcireg_t *value)
{
    UInt8 _offset;
    UInt32 ret = tag->findPCICapability(capid, &_offset);
    if (ret) {
        if (value)
            *value = ret;
        if (offset)
            *offset = _offset;
    }
    return ret;
}

//int
//pci_get_capability(pci_chipset_tag_t pc, pcitag_t tag, int capid,
//    int *offset, pcireg_t *value)
//{
//    pcireg_t reg;
//    unsigned int ofs;
//
//    reg = pci_conf_read(pc, tag, PCI_COMMAND_STATUS_REG);
//    if (!(reg & PCI_STATUS_CAPLIST_SUPPORT))
//        return (0);
//
//    /* Determine the Capability List Pointer register to start with. */
//    reg = pci_conf_read(pc, tag, PCI_BHLC_REG);
//    switch (PCI_HDRTYPE_TYPE(reg)) {
//    case 0:    /* standard device header */
//    case 1: /* PCI-PCI bridge header */
//        ofs = PCI_CAPLISTPTR_REG;
//        break;
//    case 2:    /* PCI-CardBus bridge header */
//        ofs = PCI_CARDBUS_CAPLISTPTR_REG;
//        break;
//    default:
//        return (0);
//    }
//
//    ofs = PCI_CAPLIST_PTR(pci_conf_read(pc, tag, ofs));
//    while (ofs != 0) {
//        /*
//         * Some devices, like parts of the NVIDIA C51 chipset,
//         * have a broken Capabilities List.  So we need to do
//         * a sanity check here.
//         */
//        if ((ofs & 3) || (ofs < 0x40))
//            return (0);
//        reg = pci_conf_read(pc, tag, ofs);
//        if (PCI_CAPLIST_CAP(reg) == capid) {
//            if (offset)
//                *offset = ofs;
//            if (value)
//                *value = reg;
//            return (1);
//        }
//        ofs = PCI_CAPLIST_NEXT(reg);
//    }
//
//    return (0);
//}

int
pci_intr_map_msi(struct pci_attach_args *pa, pci_intr_handle_t *ihp)
{
    pci_chipset_tag_t pc = pa->pa_pc;
    pcitag_t tag = pa->pa_tag;

    if (pci_get_capability(pc, tag, PCI_CAP_MSI, NULL, NULL) == 0)
        return (-1);

    int vec = pci_intr_vec_count(pa, 1);
    if (vec == 0)
        return (-1);
    
    *ihp = vec; // PCI_INTR_MSIX | PCITAG_OFFSET(pa->pa_tag) | vec;
    return (0);
}

int
pci_intr_map_msix(struct pci_attach_args *pa, int vec, pci_intr_handle_t *ihp)
{
    pci_chipset_tag_t pc = pa->pa_pc;
    pcitag_t tag = pa->pa_tag;
    pcireg_t reg;

    if (vec & ~PCI_INTR_VEC_MASK)
        return (-1);

    if (pci_get_capability(pc, tag, PCI_CAP_MSIX, NULL, &reg) == 0)
        return (-1);

    if (vec > PCI_MSIX_MC_TBLSZ(reg))
        return (-1);

    KASSERT(!ISSET(pa->pa_tag, PCI_INTR_TYPE_MASK));
    KASSERT(!ISSET(pa->pa_tag, PCI_INTR_VEC_MASK));

    vec = pci_intr_vec_count(pa, PCI_MSIX_MC_TBLSZ(reg));
    if (vec == 0)
        return (-1);
    
    *ihp = vec; // PCI_INTR_MSIX | PCITAG_OFFSET(pa->pa_tag) | vec;
    return (0);
}

int pci_intr_vec_count(struct pci_attach_args *pa, int vec)
{
    
#ifndef kIOInterruptTypePCIMessagedX
#define kIOInterruptTypePCIMessagedX 0x00020000
#endif
    int type = vec > 1 ? kIOInterruptTypePCIMessagedX : kIOInterruptTypePCIMessaged;
    
    pa->dev.ih_count = 0;
    for (int i = 0; i < PCI_MSIX_QUEUES; i++) {
        int interruptType;
        IOReturn ret = pa->pa_tag->getInterruptType(i, &interruptType);
        if (ret != kIOReturnSuccess)
            break;
        
        if (interruptType & type)
        {
            pci_intr_handle *ihp = new pci_intr_handle();
            ihp->ih = i;
            ihp->dev = pa->pa_tag;  // pci device reference
            ihp->workloop = pa->dev.dev->fWorkloop;
            pa->dev.ih[pa->dev.ih_count++] = ihp;
            if (vec == 1)
                break;
        }
    }
    return pa->dev.ih_count;
}

void
pci_intr_unmap(struct pci_attach_args *pa, pci_intr_handle_t *ihp)
{
    pci_intr_handle *ih = pa->dev.ih[0];
    for (int i = 0; i < *ihp; i++, ih++) {
        delete &ih;
    }
    
}

const char* pci_intr_string(pci_chipset_tag_t pc, pci_intr_handle_t ih)
{
    return ih > 1 ? "msix" : "msi";
}

int
pci_intr_map(struct pci_attach_args *pa, pci_intr_handle_t *ihp)
{
    return 0;
}

void* pci_intr_establish(pci_chipset_tag_t pc, pci_intr_handle_t ih, int level, int (*handler)(void *), void *arg, const char *name)
{
    struct device *dev = (struct device *)arg;
    memcpy(dev, &dev->dev->pa->dev, sizeof(struct device));
    
    pci_intr_handle *ihp = dev->ih[0];
    for (int i = 0; i < dev->ih_count; i++, ihp++) {
        ihp->arg = arg;
        ihp->func = handler;
        ihp->intr = IOInterruptEventSource::interruptEventSource(ihp, (IOInterruptEventAction)interrupt_func, ihp->dev, ihp->ih);
        
        if (ihp->intr == 0)
            return 0;
        if (ihp->workloop->addEventSource(ihp->intr) != kIOReturnSuccess)
            return 0;
        
        ihp->intr->enable();
    }
    return dev->ih;
}

void
pci_intr_disestablish(pci_chipset_tag_t pc, void *cookie)
{
    struct device *dev = (struct device*)pc;
    
    pci_intr_handle *ih = (pci_intr_handle *)cookie;
    for (int i = 0; i < dev->ih_count; i++, ih++) {
        ih->intr->disable();
        
        ih->workloop->removeEventSource(ih->intr);
        ih->intr->release();
        ih->intr = NULL;
    }
}

void
pci_enable_msi(pci_chipset_tag_t pc, pcitag_t tag)
{
    pcireg_t reg;
    int off;
    
    if (pci_get_capability(pc, tag, PCI_CAP_MSI, &off, &reg) == 0)
        return;
    
    pci_conf_write(pc, tag, off, reg | PCI_MSI_MC_MSIE);
}

void
pci_disable_msi(pci_chipset_tag_t pc, pcitag_t tag)
{
    pcireg_t reg;
    int off;
    
    if (pci_get_capability(pc, tag, PCI_CAP_MSI, &off, &reg) == 0)
        return;
    
    pci_conf_write(pc, tag, off, reg & ~PCI_MSI_MC_MSIE);
}

void
pci_enable_msix(pci_chipset_tag_t pc, pcitag_t tag)
{
    pcireg_t reg;
    int off;

    if (pci_get_capability(pc, tag, PCI_CAP_MSIX, &off, &reg) == 0)
        return;

    pci_conf_write(pc, tag, off, reg | PCI_MSIX_MC_MSIXE);
}

void
pci_disable_msix(pci_chipset_tag_t pc, pcitag_t tag)
{
    pcireg_t reg;
    int off;

    if (pci_get_capability(pc, tag, PCI_CAP_MSIX, &off, &reg) == 0)
        return;

    pci_conf_write(pc, tag, off, reg & ~PCI_MSIX_MC_MSIXE);
}

int
pci_set_powerstate(pci_chipset_tag_t pc, pcitag_t tag, int state)
{
    pcireg_t reg;
    int offset, ostate = state;

    /*
     * Warn the firmware that we are going to put the device
     * into the given state.
     */
    pci_set_powerstate_md(pc, tag, state, 1);

    if (pci_get_capability(pc, tag, PCI_CAP_PWRMGMT, &offset, 0)) {
        if (state == PCI_PMCSR_STATE_D3) {
            /*
             * The PCI Power Management spec says we
             * should disable I/O and memory space as well
             * as bus mastering before we place the device
             * into D3.
             */
            reg = pci_conf_read(pc, tag, PCI_COMMAND_STATUS_REG);
            reg &= ~PCI_COMMAND_IO_ENABLE;
            reg &= ~PCI_COMMAND_MEM_ENABLE;
            reg &= ~PCI_COMMAND_MASTER_ENABLE;
            pci_conf_write(pc, tag, PCI_COMMAND_STATUS_REG, reg);
        }
        reg = pci_conf_read(pc, tag, offset + PCI_PMCSR);
        if ((reg & PCI_PMCSR_STATE_MASK) != state) {
            ostate = reg & PCI_PMCSR_STATE_MASK;

            pci_conf_write(pc, tag, offset + PCI_PMCSR,
                (reg & ~PCI_PMCSR_STATE_MASK) | state);
            if (state == PCI_PMCSR_STATE_D3 ||
                ostate == PCI_PMCSR_STATE_D3)
                delay(10 * 1000);
        }
    }

    /*
     * Warn the firmware that the device is now in the given
     * state.
     */
    pci_set_powerstate_md(pc, tag, state, 0);

    return (ostate);
}
