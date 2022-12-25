//
//  _bpf.c
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/23.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "compat.h"
#include "_bpf.h"

/*
 *  bpf_iflist is the list of interfaces; each corresponds to an ifnet
 *  bpf_d_list is the list of descriptors
 */
struct bpf_if    *bpf_iflist;

int bpf_mtap_hdr(caddr_t arg, const void *tap, u_int dlen, mbuf_t m,
    u_int direction)
{
    struct bpf_if *bp = (struct bpf_if *)arg;
    
    if (m == NULL)
        return (0);

    if (bp == NULL)
        return (0);
    
    switch (direction) {
        case BPF_DIRECTION_IN:
            bp->bif_ifp->rx_tap = (void *)tap;
            break;
        case BPF_DIRECTION_OUT:
            bp->bif_ifp->tx_tap = (void *)tap;
            break;
        default:
            break;
    }
    return 0;
}


void *
bpfsattach(caddr_t *bpfp, const char *name, u_int dlt, u_int hdrlen)
{
    struct bpf_if *bp;

    if ((bp = (struct bpf_if *)malloc(sizeof(*bp), M_DEVBUF, M_NOWAIT)) == NULL)
        panic("bpfattach");
//    SMR_SLIST_INIT(&bp->bif_dlist);
    bp->bif_driverp = (struct bpf_if **)bpfp;
    bp->bif_name = name;
    bp->bif_ifp = NULL;
    bp->bif_dlt = dlt;

    bp->bif_next = bpf_iflist;
    bpf_iflist = bp;

    *bp->bif_driverp = bp;

    /*
     * Compute the length of the bpf header.  This is not necessarily
     * equal to SIZEOF_BPF_HDR because we want to insert spacing such
     * that the network layer header begins on a longword boundary (for
     * performance reasons and to alleviate alignment restrictions).
     */
    bp->bif_hdrlen = BPF_WORDALIGN(hdrlen + SIZEOF_BPF_HDR) - hdrlen;

    return (bp);
}

void
bpfattach(caddr_t *driverp, struct ifnet *ifp, u_int dlt, u_int hdrlen)
{
    struct bpf_if *bp;

    bp = (struct bpf_if *)bpfsattach(driverp, ifp->if_xname, dlt, hdrlen);
    bp->bif_ifp = ifp;
}

/* Detach an interface from its attached bpf device.  */
void
bpfdetach(struct ifnet *ifp)
{
    struct bpf_if *bp, *nbp;

//    KERNEL_ASSERT_LOCKED();

    for (bp = bpf_iflist; bp; bp = nbp) {
        nbp = bp->bif_next;
        if (bp->bif_ifp == ifp)
            bpfsdetach(bp);
    }
    ifp->if_bpf = NULL;
}

void
bpfsdetach(void *p)
{
    struct bpf_if *bp = (struct bpf_if *)p, *tbp;
//    struct bpf_d *bd;
//    int maj;

//    KERNEL_ASSERT_LOCKED();

    /* Locate the major number. */
//    for (maj = 0; maj < nchrdev; maj++)
//        if (cdevsw[maj].d_open == bpfopen)
//            break;

//    while ((bd = SMR_SLIST_FIRST_LOCKED(&bp->bif_dlist)))
//        vdevgone(maj, bd->bd_unit, bd->bd_unit, VCHR);

    for (tbp = bpf_iflist; tbp; tbp = tbp->bif_next) {
        if (tbp->bif_next == bp) {
            tbp->bif_next = bp->bif_next;
            break;
        }
    }

    if (bpf_iflist == bp)
        bpf_iflist = bp->bif_next;

    free(bp, M_DEVBUF, sizeof(*bp));
}

int bpf_mtap(caddr_t arg, mbuf_t m, u_int direction)
{
    struct bpf_if *bp = (struct bpf_if *)arg;
    mbuf_t m0;
    struct device *dev;
    
    if (m == NULL)
        return (0);

    if (bp == NULL)
        return (0);
    
    if (bp->bif_dlt == DLT_EN10MB) {
        switch (direction) {
            case BPF_DIRECTION_IN:
                dev = (struct device *)bp->bif_ifp->if_softc;
                mbuf_dup(m, M_DONTWAIT, &m0);
                bp->bif_ifp->iface->inputPacket(m0, 0, IONetworkInterface::kInputOptionQueuePacket);
                break;
            default:
                break;
        }
    }
    
    return 0;
}
