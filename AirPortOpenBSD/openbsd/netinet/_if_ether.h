//
//  if_ether.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef if_ether_h
#define if_ether_h

#include "apple80211.h"

#include <net/if.h>
#include <net/if_var.h>
#include <sys/queue.h>
#include <net/if_dl.h>
#include <sys/_mbuf.h>
#include <netinet/if_media.h>
#include <netinet/_if.h>
#include <netinet/_bpf.h>

#define        ETHERTYPE_EAPOL ETHERTYPE_PAE

/*
 * Values for if_link_state.
 */
#define LINK_STATE_UNKNOWN      kIONetworkLinkUndefined     // 0 /* link unknown */
#define LINK_STATE_INVALID      kIONetworkLinkDown          // 1 /* link invalid */
#define LINK_STATE_DOWN         kIONetworkLinkDown          // 2 /* link is down */
#define LINK_STATE_KALIVE_DOWN  kIONetworkLinkDown          // 3 /* keepalive reports down */
#define LINK_STATE_UP           kIONetworkLinkUp            // 4 /* link is up */
#define LINK_STATE_HALF_DUPLEX  kIONetworkLinkUp            // 5 /* link is up and half duplex */
#define LINK_STATE_FULL_DUPLEX  kIONetworkLinkUp            // 6 /* link is up and full duplex */

#define LINK_STATE_IS_UP(_s)    \
((_s) >= LINK_STATE_UP || (_s) == LINK_STATE_UNKNOWN)

/*
 * Length of interface description, including terminating '\0'.
 */
#define    IFDESCRSIZE    64


//#define AF_MAX          36


//#define IF_DATA_TIMEVAL timeval

#define SRPL_HEAD(name, entry)        SLIST_HEAD(name, entry)


struct refcnt {
    unsigned int r_refs;
};

struct srp {
    void            *ref;
};

struct srpl {
    struct srp        sl_head;
};

struct ifnet {                /* and the entries */
    void    *if_softc;        /* [I] lower-level data for this if */
    struct    refcnt if_refcnt;
    TAILQ_ENTRY(ifnet) if_list;    /* [k] all struct ifnets are chained */
    TAILQ_HEAD(, ifaddr) if_addrlist; /* [N] list of addresses per if */
    TAILQ_HEAD(, ifmaddr) if_maddrlist; /* [N] list of multicast records */
    TAILQ_HEAD(, ifg_list) if_groups; /* [N] list of groups per if */
    struct task_list if_addrhooks;    /* [I] address change callbacks */
    struct task_list if_linkstatehooks; /* [I] link change callbacks*/
    struct task_list if_detachhooks; /* [I] detach callbacks */
                /* [I] check or clean routes (+ or -)'d */
    void    (*if_rtrequest)(struct ifnet *, int, struct rtentry *);
    char    if_xname[IFNAMSIZ];    /* [I] external name (name + unit) */
    int    if_pcount;        /* [k] # of promiscuous listeners */
    unsigned int if_bridgeidx;    /* [k] used by bridge ports */
    caddr_t    if_bpf;            /* packet filter structure */
    caddr_t if_switchport;        /* used by switch ports */
    caddr_t if_mcast;        /* used by multicast code */
    caddr_t if_mcast6;        /* used by IPv6 multicast code */
    caddr_t    if_pf_kif;        /* pf interface abstraction */
    union {
        struct srpl carp_s;    /* carp if list (used by !carp ifs) */
        struct ifnet *carp_d;    /* ptr to carpdev (used by carp ifs) */
    } if_carp_ptr;
#define if_carp        if_carp_ptr.carp_s
#define if_carpdev    if_carp_ptr.carp_d
    unsigned int if_index;        /* [I] unique index for this if */
    short    if_timer;        /* time 'til if_watchdog called */
    unsigned short if_flags;    /* [N] up/down, broadcast, etc. */
    int    if_xflags;        /* [N] extra softnet flags */
    struct    if_data if_data;    /* stats and other data about if */
    struct    cpumem *if_counters;    /* per cpu stats */
    uint32_t if_hardmtu;        /* [d] maximum MTU device supports */
    char    if_description[IFDESCRSIZE]; /* [c] interface description */
    u_short    if_rtlabelid;        /* [c] next route label */
    uint8_t if_priority;        /* [c] route priority offset */
    uint8_t if_llprio;        /* [N] link layer priority */
//    struct    timeout if_slowtimo;    /* [I] watchdog timeout */
    struct    task if_watchdogtask;    /* [I] watchdog task */
    struct    task if_linkstatetask;    /* [I] task to do route updates */

    /* procedure handles */
    SRPL_HEAD(, ifih) if_inputs;    /* [k] input routines (dequeue) */
    int    (*if_output)(struct ifnet *, mbuf_t, struct sockaddr *,
             struct rtentry *);    /* output routine (enqueue) */
                    /* link level output function */
    int    (*if_ll_output)(struct ifnet *, mbuf_t,
            struct sockaddr *, struct rtentry *);
    int    (*if_enqueue)(struct ifnet *, mbuf_t);
    void    (*if_start)(struct ifnet *);    /* initiate output */
    int    (*if_ioctl)(struct ifnet *, u_long, caddr_t); /* ioctl hook */
    void    (*if_watchdog)(struct ifnet *);    /* timer routine */
    int    (*if_wol)(struct ifnet *, int);    /* WoL routine **/
    
    /* queues */
//    IOPacketQueue *if_snd;        /* transmit queue */
    struct    mbuf_queue if_snd;        /* transmit queue */
    struct    ifqueue **if_ifqs;    /* [I] pointer to an array of sndqs */
    void    (*if_qstart)(struct ifqueue *);
    unsigned int if_nifqs;        /* [I] number of output queues */
    unsigned int if_txmit;        /* [c] txmitigation amount */

//    struct    ifiqueue if_rcv;    /* rx/input queue */
//    struct    ifiqueue **if_iqs;    /* [I] pointer to the array of iqs */
    unsigned int if_niqs;        /* [I] number of input queues */

    struct sockaddr_dl *if_sadl;    /* [N] pointer to our sockaddr_dl */

    void    *if_afdata[AF_MAX];
    
    void* rx_tap;
    void* tx_tap;
    
    IONetworkStats *netStat;
#define if_opackets     netStat->outputPackets
#define if_oerrors      netStat->outputErrors
#define if_ipackets     netStat->inputPackets
#define if_ierrors      netStat->inputErrors
#define if_collisions   netStat->collisions
    
    uint32_t if_imcasts;
    int if_ibytes;
    int if_hdrlen;
    
    AirPort_OpenBSD_Class_Interface *iface;
    int if_capabilities;
    int if_baudrate;
    int if_link_state;
    
    int if_power_state;
    
    char fwver[256];
    char fwname[256];
    int err = 0;
};

struct ifnet *if_get(const char* if_xname);

void if_attach(struct ifnet *ifp);
void if_detach(struct ifnet *ifp);
void if_start(struct ifnet *ifp);
void if_input(struct ifnet* ifp, struct mbuf_list *ml);
void post_message(struct ifnet *ifp, int msgCode);

void if_link_state_change(struct ifnet *);


#define ifq_enqueue mq_enqueue
#define ifq_dequeue mq_dequeue

#define IFQ_ENQUEUE(mgtq, m, error) error = mq_enqueue(mgtq, m)
#define IFQ_DEQUEUE(mgtq, m) m = mq_dequeue(mgtq)

static int
if_enqueue(struct ifnet *ifp, mbuf_t m)
{
    return ((*ifp->if_enqueue)(ifp, m));
}

static int
if_enqueue_ifq(struct ifnet *ifp, mbuf_t m)
{
    int error;
    
    IFQ_ENQUEUE(&ifp->if_snd, m, error);
    if (error)
        return (error);
    
    if_start(ifp);
    
    return (0);
}

static int ifq_oactive = 0;

static
void ifq_clr_oactive(IOPacketQueue **if_snd)
{
    ifq_oactive = 0;
    (*if_snd)->lockFlush();
}

static
int ifq_is_oactive(IOPacketQueue **if_snd)
{
    return ifq_oactive;
}

static
void ifq_set_oactive(IOPacketQueue **if_snd)
{
    ifq_oactive = 1;
}



static
void ifq_clr_oactive(struct mbuf_queue *if_snd)
{
    ifq_oactive = 0;
    mq_purge(if_snd);
}

static
int ifq_is_oactive(struct mbuf_queue *if_snd)
{
    return ifq_oactive;
}

static
void ifq_set_oactive(struct mbuf_queue *if_snd)
{
    ifq_oactive = 1;
}

static
int ifq_empty(struct mbuf_queue *if_snd)
{
    return ifq_oactive;
}

#define    ifq_len              mq_len
#define    ifq_empty            mq_empty
#define    ifq_set_maxlen       mq_set_maxlen

static inline void
ifq_restart(struct mbuf_queue *ifq)
{
//    ifq_serialize(ifq, &ifq->ifq_restart);
}


#define    ETHER_IS_MULTICAST(addr) (*(addr) & 0x01) /* is address mcast/bcast? */
#define    ETHER_IS_BROADCAST(addr) \
    (((addr)[0] & (addr)[1] & (addr)[2] & \
      (addr)[3] & (addr)[4] & (addr)[5]) == 0xff)
#define    ETHER_IS_ANYADDR(addr)        \
    (((addr)[0] | (addr)[1] | (addr)[2] | \
      (addr)[3] | (addr)[4] | (addr)[5]) == 0x00)
#define    ETHER_IS_EQ(a1, a2)    (memcmp((a1), (a2), ETHER_ADDR_LEN) == 0)



/* default interface priorities */
#define IF_WIRED_DEFAULT_PRIORITY    0
#define IF_WIRELESS_DEFAULT_PRIORITY    4
#define IF_CARP_DEFAULT_PRIORITY    15


/* Packet tag types */
#define PACKET_TAG_IPSEC_IN_DONE    0x0001  /* IPsec applied, in */
#define PACKET_TAG_IPSEC_OUT_DONE   0x0002  /* IPsec applied, out */
#define PACKET_TAG_GIF              0x0040  /* GIF processing done */
#define PACKET_TAG_GRE              0x0080  /* GRE processing done */
#define PACKET_TAG_DLT              0x0100 /* data link layer type */
#define PACKET_TAG_PF_DIVERT        0x0200 /* pf(4) diverted packet */
#define PACKET_TAG_PF_REASSEMBLED   0x0800 /* pf reassembled ipv6 packet */
#define PACKET_TAG_SRCROUTE         0x1000 /* IPv4 source routing options */
#define PACKET_TAG_TUNNEL           0x2000    /* Tunnel endpoint address */
#define PACKET_TAG_CARP_BAL_IP      0x4000  /* carp(4) ip balanced marker */

//#define LLADDR(s) s

/*
 * Ethernet multicast address structure.  There is one of these for each
 * multicast address or range of multicast addresses that we are supposed
 * to listen to on a particular interface.  They are kept in a linked list,
 * rooted in the interface's arpcom structure.  (This really has nothing to
 * do with ARP, or with the Internet address family, but this appears to be
 * the minimally-disrupting place to put it.)
 */
struct ether_multi {
    u_int8_t enm_addrlo[ETHER_ADDR_LEN]; /* low  or only address of range */
    u_int8_t enm_addrhi[ETHER_ADDR_LEN]; /* high or only address of range */
    u_int     enm_refcount;        /* no. claims to this addr/range */
    LIST_ENTRY(ether_multi) enm_list;
};

/*
 * Structure used by macros below to remember position when stepping through
 * all of the ether_multi records.
 */
struct ether_multistep {
    struct ether_multi  *e_enm;
};

/*
 * Macro for looking up the ether_multi record for a given range of Ethernet
 * multicast addresses connected to a given arpcom structure.  If no matching
 * record is found, "enm" returns NULL.
 */
#define ETHER_LOOKUP_MULTI(addrlo, addrhi, ac, enm)            \
    /* u_int8_t addrlo[ETHER_ADDR_LEN]; */                \
    /* u_int8_t addrhi[ETHER_ADDR_LEN]; */                \
    /* struct arpcom *ac; */                    \
    /* struct ether_multi *enm; */                    \
do {                                    \
    for ((enm) = LIST_FIRST(&(ac)->ac_multiaddrs);            \
        (enm) != NULL &&                        \
        (memcmp((enm)->enm_addrlo, (addrlo), ETHER_ADDR_LEN) != 0 ||\
         memcmp((enm)->enm_addrhi, (addrhi), ETHER_ADDR_LEN) != 0);    \
        (enm) = LIST_NEXT((enm), enm_list));            \
} while (/* CONSTCOND */ 0)

/*
 * Macro to step through all of the ether_multi records, one at a time.
 * The current position is remembered in "step", which the caller must
 * provide.  ETHER_FIRST_MULTI(), below, must be called to initialize "step"
 * and get the first record.  Both macros return a NULL "enm" when there
 * are no remaining records.
 */
#define ETHER_NEXT_MULTI(step, enm)                    \
    /* struct ether_multistep step; */                \
    /* struct ether_multi *enm; */                    \
do {                                    \
    if (((enm) = (step).e_enm) != NULL)                \
        (step).e_enm = LIST_NEXT((enm), enm_list);        \
} while (/* CONSTCOND */ 0)

#define ETHER_FIRST_MULTI(step, ac, enm)                \
    /* struct ether_multistep step; */                \
    /* struct arpcom *ac; */                    \
    /* struct ether_multi *enm; */                    \
do {                                    \
    (step).e_enm = LIST_FIRST(&(ac)->ac_multiaddrs);        \
    ETHER_NEXT_MULTI((step), (enm));                \
} while (/* CONSTCOND */ 0)


/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 */
struct    arpcom {
    struct     ifnet ac_if;            /* network-visible interface */
    u_int8_t ac_enaddr[ETHER_ADDR_LEN];    /* ethernet hardware address */
    char     ac__pad[2];            /* pad for some machines */
    LIST_HEAD(, ether_multi) ac_multiaddrs;    /* list of multicast addrs */
    int     ac_multicnt;            /* length of ac_multiaddrs */
    int     ac_multirangecnt;        /* number of mcast ranges */
    
};

static u_int8_t etherbroadcastaddr[ETHER_ADDR_LEN] =
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static u_int8_t etheranyaddr[ETHER_ADDR_LEN] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#define senderr(e) { error = (e); goto bad;}


/*
 * Convert Ethernet address to printable (loggable) representation.
 */
static char digits[] = "0123456789abcdef";
static char * ether_sprintf(u_char *ap)
{
    int i;
    static char etherbuf[ETHER_ADDR_LEN * 3];
    char *cp = etherbuf;

    for (i = 0; i < ETHER_ADDR_LEN; i++) {
        *cp++ = digits[*ap >> 4];
        *cp++ = digits[*ap++ & 0xf];
        *cp++ = ':';
    }
    *--cp = 0;
    return (etherbuf);
}

/*
 * Free the link level name for the specified interface.  This is
 * a detach helper.  This is called from if_detach() or from
 * link layer type specific detach functions.
 */
static void
if_free_sadl(struct ifnet *ifp)
{
    if (ifp->if_sadl == NULL)
        return;

    free(ifp->if_sadl, M_IFADDR, ifp->if_sadl->sdl_len);
}


/*
 * Allocate the link level name for the specified interface.  This
 * is an attachment helper.  It must be called after ifp->if_addrlen
 * is initialized, which may not be the case when if_attach() is
 * called.
 */
static void
if_alloc_sadl(struct ifnet *ifp)
{
    unsigned int socksize;
    int namelen, masklen;
    struct sockaddr_dl *sdl;

    /*
     * If the interface already has a link name, release it
     * now.  This is useful for interfaces that can change
     * link types, and thus switch link names often.
     */
    if_free_sadl(ifp);

    namelen = strlen(ifp->if_xname);
    masklen = offsetof(struct sockaddr_dl, sdl_data[0]) + namelen;
    socksize = masklen + ETHER_ADDR_LEN;
#define ROUNDUP(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
    if (socksize < sizeof(*sdl))
        socksize = sizeof(*sdl);
    socksize = ROUNDUP(socksize);
    sdl = (struct sockaddr_dl *)malloc(socksize, M_IFADDR, M_WAITOK|M_ZERO);
    sdl->sdl_len = socksize;
    sdl->sdl_family = AF_LINK;
    bcopy(ifp->if_xname, sdl->sdl_data, namelen);
    sdl->sdl_nlen = namelen;
    sdl->sdl_alen = ETHER_ADDR_LEN;
//    sdl->sdl_index = ifp->if_index;
//    sdl->sdl_type = ifp->if_type;
    ifp->if_sadl = sdl;
}


extern void ether_ifattach(struct ifnet *ifp);
extern void ether_ifdetach(struct ifnet *ifp);

static int if_setlladdr(struct ifnet * ifp, const uint8_t *lladdr)
{
    if (ifp->if_sadl == NULL)
    {
        if_alloc_sadl(ifp);
    }

    memcpy(((struct arpcom *)ifp)->ac_enaddr, lladdr, ETHER_ADDR_LEN);
    memcpy(LLADDR(ifp->if_sadl), lladdr, ETHER_ADDR_LEN);

    ether_ifattach(ifp);
    
    return (0);
}


/*
 * Defer the configuration of the specified device until after
 * root file system is mounted.
 */
static void config_mountroot(struct device *dev, void (*func)(struct device *))
{
    (*func)(dev);
}


static u_int32_t __pure
ether_crc32_le_update(u_int32_t crc, const u_int8_t *buf, size_t len)
{
    static const u_int32_t crctab[] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
    size_t i;

    for (i = 0; i < len; i++) {
        crc ^= buf[i];
        crc = (crc >> 4) ^ crctab[crc & 0xf];
        crc = (crc >> 4) ^ crctab[crc & 0xf];
    }

    return (crc);
}


#endif /* if_ether_h */
