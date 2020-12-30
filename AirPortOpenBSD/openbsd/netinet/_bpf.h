//
//  _bpf.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/23.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef _bpf_h
#define _bpf_h

/*
 * Descriptor associated with each attached hardware interface.
 */
struct bpf_if {
    struct bpf_if *bif_next;    /* list of all interfaces */
//    SMR_SLIST_HEAD(, bpf_d) bif_dlist;        /* descriptor list */
    struct bpf_if **bif_driverp;    /* pointer into softc */
    u_int bif_dlt;            /* link layer type */
    u_int bif_hdrlen;        /* length of header (with padding) */
    const char *bif_name;        /* name of "subsystem" */
    struct ifnet *bif_ifp;        /* corresponding interface */
};

int bpf_mtap_hdr(caddr_t arg, const void *tap, u_int dlen, mbuf_t m,
                        u_int direction);

void    bpfattach(caddr_t *driverp, struct ifnet *ifp, u_int dlt, u_int hdrlen);
void    bpfdetach(struct ifnet *ifp);
void    bpfsdetach(void *);
int     bpf_mtap(caddr_t arg, mbuf_t m, u_int direction);

#endif /* _bpf_h */
