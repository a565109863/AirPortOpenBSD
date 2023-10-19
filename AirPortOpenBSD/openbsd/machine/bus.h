#ifndef bus_h
#define bus_h

#include <sys/_malloc.h>

#define DMA_BIT_MASK(n)    (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

typedef caddr_t             bus_space_handle_t;
typedef mach_vm_address_t   bus_addr_t;
typedef uint32_t            bus_size_t;
typedef IOMemoryMap*        bus_space_tag_t;
typedef IOPhysicalSegment   bus_dma_segment_t;

#define ds_addr location
#define ds_len  length

struct dm_cookie {
    IOMemoryDescriptor *memDes;
    IODMACommand *dmaCmd;
};

/*
 *    bus_dmamap_t
 *
 *    Describes a DMA mapping.
 */
struct bus_dmamap {
    /*
     * PRIVATE MEMBERS: not for use by machine-independent code.
     */
//    bus_size_t    _dm_size;    /* largest DMA transfer mappable */
    int        _dm_segcnt;    /* number of segs this map can map */
//    bus_size_t    _dm_maxsegsz;    /* largest possible segment */
//    bus_size_t    _dm_boundary;    /* don't cross this */
//    int        _dm_flags;    /* misc. flags */

    void        *_dm_cookie;    /* cookie for bus-specific functions */

    /*
     * PUBLIC MEMBERS: these are used by machine-independent code.
     */
    bus_size_t    dm_mapsize;    /* size of the mapping */
    int        dm_nsegs;    /* # valid segments in mapping */
    bus_dma_segment_t *dm_segs;    /* segments; variable length */
    
    bus_size_t alignment;
    IOBufferMemoryDescriptor *bufDes;
    IOMbufNaturalMemoryCursor*    mbufCursor;
    
    bus_addr_t kvap;
    
    SLIST_ENTRY(bus_dmamap)    next;
};

typedef struct bus_dmamap* bus_dmamap_t;

struct bus_dma_tag {
    SLIST_HEAD(, bus_dmamap)    bus_dmamap_list;
};

typedef bus_dma_tag*               bus_dma_tag_t;


#define BUS_SPACE_BARRIER_READ  0x01        /* force read barrier */
#define BUS_SPACE_BARRIER_WRITE 0x02        /* force write barrier */

#define    BUS_DMA_WAITOK        0x0000
#define    BUS_DMA_NOWAIT        0x0001
#define    BUS_DMA_ALLOCNOW    0x0002
#define    BUS_DMA_COHERENT    0x0008
#define    BUS_DMA_BUS1        0x0010    /* placeholders for bus functions... */
#define    BUS_DMA_BUS2        0x0020
#define    BUS_DMA_BUS3        0x0040
#define    BUS_DMA_BUS4        0x0080
#define    BUS_DMA_READ        0x0100    /* mapping is device -> memory only */
#define    BUS_DMA_WRITE        0x0200    /* mapping is memory -> device only */
#define    BUS_DMA_STREAMING    0x0400    /* hint: sequential, unidirectional */
#define    BUS_DMA_ZERO        0x0800    /* zero memory in dmamem_alloc */
#define    BUS_DMA_NOCACHE        0x1000
#define    BUS_DMA_64BIT        0x2000    /* device handles 64bit dva */


OS_INLINE
uint8_t
_OSReadInt8(const volatile void* base, uintptr_t byteOffset)
{
    return *(volatile uint8_t *)((uintptr_t)base + byteOffset);
}

OS_INLINE
void
_OSWriteInt8(volatile void* base, uintptr_t byteOffset, uint8_t data)
{
    *(volatile uint8_t *)((uintptr_t)base + byteOffset) = data;
}


#define bus_space_read_8(c_st, sc_sh, reg)  OSReadLittleInt64((sc_sh), (reg))
#define bus_space_write_8(c_st, sc_sh, reg, val)                                           \
do { OSWriteLittleInt64((sc_sh), (reg), (val));               \
} while(0)

#define bus_space_read_4(c_st, sc_sh, reg)  OSReadLittleInt32((sc_sh), (reg))
#define bus_space_write_4(c_st, sc_sh, reg, val)                                           \
do { OSWriteLittleInt32((sc_sh), (reg), (val));               \
} while(0)

#define bus_space_read_2(c_st, sc_sh, reg)  OSReadLittleInt16((sc_sh), (reg))
#define bus_space_write_2(c_st, sc_sh, reg, val)                                           \
do { OSWriteLittleInt16((sc_sh), (reg), (val));               \
} while(0)

#define bus_space_read_1(c_st, sc_sh, reg)          _OSReadInt8((sc_sh), (reg))
#define bus_space_write_1(c_st, sc_sh, reg, val8)                              \
do { _OSWriteInt8((sc_sh), (reg), (val8));               \
} while(0)

#define bus_space_barrier(a, b, c, d, e)    //os_compiler_barrier()


/*
 *    void bus_space_write_multi_N(bus_space_tag_t tag,
 *        bus_space_handle_t bsh, bus_size_t offset,
 *        const u_intN_t *addr, size_t count);
 *
 * Write `count' 1, 2, 4, or 8 byte quantities from the buffer
 * provided to bus space described by tag/handle/offset.
 */

static __inline__ void
bus_space_write_multi_1(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int8_t *dest, size_t count)
{
    while ((int)--count >= 0)
        bus_space_write_1(tag, handle, offset, *dest++);
}

static __inline__ void
bus_space_write_multi_2(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int16_t *dest, size_t count)
{
    while ((int)--count >= 0)
        bus_space_write_2(tag, handle, offset, *dest++);
}

static __inline__ void
bus_space_write_multi_4(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int32_t *dest, size_t count)
{
    while ((int)--count >= 0)
        bus_space_write_4(tag, handle, offset, *dest++);
}

/*
 *    void bus_space_write_region_N(bus_space_tag_t tag,
 *        bus_space_handle_t bsh, bus_size_t offset,
 *        const u_intN_t *addr, size_t count);
 *
 * Write `count' 1, 2, 4, or 8 byte quantities from the buffer provided
 * to bus space described by tag/handle starting at `offset'.
 */

static __inline__ void
bus_space_write_region_1(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int8_t *dest, size_t count)
{
    while ((int)--count >= 0)
        bus_space_write_1(tag, handle, offset++, *dest++);
}

static __inline__ void
bus_space_write_region_2(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int16_t *dest, size_t count)
{
    while ((int)--count >= 0) {
        bus_space_write_2(tag, handle, offset, *dest++);
        offset += 2;
    }
}

static __inline__ void
bus_space_write_region_4(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int32_t *dest, size_t count)
{
    while ((int)--count >= 0) {
        bus_space_write_4(tag, handle, offset, *dest++);
        offset += 4;
    }
}

/*
 *    void bus_space_read_region_N(bus_space_tag_t tag,
 *        bus_space_handle_t bsh, bus_size_t offset,
 *        u_intN_t *addr, size_t count);
 *
 * Read `count' 1, 2, 4, or 8 byte quantities from bus space
 * described by tag/handle and starting at `offset' and copy into
 * buffer provided.
 */

static __inline__ void
bus_space_read_region_1(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int8_t *dest, size_t count)
{
    while ((int)--count >= 0)
        *dest++ = bus_space_read_1(tag, handle, offset++);
}

static __inline__ void
bus_space_read_region_2(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int16_t *dest, size_t count)
{
    while ((int)--count >= 0) {
        *dest++ = bus_space_read_2(tag, handle, offset);
        offset += 2;
    }
}

static __inline__ void
bus_space_read_region_4(bus_space_tag_t tag, bus_space_handle_t handle,
    bus_addr_t offset, u_int32_t *dest, size_t count)
{
    while ((int)--count >= 0) {
        *dest++ = bus_space_read_4(tag, handle, offset);
        offset += 4;
    }
}

int bus_dmamap_create(bus_dma_tag_t t, bus_size_t size, int nsegments, bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *map);
void bus_dmamap_destroy(bus_dma_tag_t t, bus_dmamap_t map);

int  bus_dmamem_alloc(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
                       int flags);
void bus_dmamem_free(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs);

int bus_dmamem_map(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs,
                size_t size, caddr_t *kvap, int flags);
void bus_dmamem_unmap(bus_dma_tag_t t, void *kvap,  bus_size_t size);

int bus_dmamap_load_mbuf(bus_dma_tag_t t, bus_dmamap_t map, mbuf_t m0, int flags);
int bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, void *buf, bus_size_t buflen, struct proc *p, int flags);
int bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, mbuf_t buf, bus_size_t buflen, struct proc *p, int flags);
int bus_dmamap_load_raw(bus_dma_tag_t t, bus_dmamap_t map, bus_dma_segment_t *segs,
    int nsegs, bus_size_t size, int flags);
void bus_dmamap_unload(bus_dma_tag_t t, bus_dmamap_t map);

void bus_dmamap_sync(bus_dma_tag_t tag, bus_dmamap_t dmam, bus_addr_t offset, bus_size_t len, int ops);

//#define bus_dmamap_sync(t, m, o, l, ops)
#define bus_space_unmap(t, map, size)


/*
 * Actions for ca_activate.
 */
#define    DVACT_DEACTIVATE    1    /* deactivate the device */
#define    DVACT_QUIESCE        2    /* warn the device about suspend */
#define    DVACT_SUSPEND        3    /* suspend the device */
#define    DVACT_RESUME        4    /* resume the device */
#define    DVACT_WAKEUP        5    /* tell device to recover after resume */
#define    DVACT_POWERDOWN        6    /* power device down */

/*
 * Minimal device structures.
 * Note that all ``system'' device types are listed here.
 */
enum devclass {
    DV_DULL,        /* generic, no special info */
    DV_CPU,            /* CPU (carries resource utilization) */
    DV_DISK,        /* disk drive (label, etc) */
    DV_IFNET,        /* network interface */
    DV_TAPE,        /* tape device */
    DV_TTY            /* serial line interface (???) */
};

/* Flags given to config_detach(), and the ca_detach function. */
#define    DETACH_FORCE    0x01        /* force detachment; hardware gone */
#define    DETACH_QUIET    0x02        /* don't print a notice */

struct cfdriver {
    void    **cd_devs;        /* devices found */
    char    *cd_name;        /* device name */
    enum    devclass cd_class;    /* device classification */
    int    cd_indirect;        /* indirectly configure subdevices */
    int    cd_ndevs;        /* size of cd_devs array */
};

typedef int (*cfmatch_t)(struct device *parent, void* match, void *aux);
typedef void (*cfscan_t)(struct ifnet *, void *);

/*
 * `configuration' attachment and driver (what the machine-independent
 * autoconf uses).  As devices are found, they are applied against all
 * the potential matches.  The one with the best match is taken, and a
 * device structure (plus any other data desired) is allocated.  Pointers
 * to these are placed into an array of pointers.  The array itself must
 * be dynamic since devices can be found long after the machine is up
 * and running.
 *
 * Devices can have multiple configuration attachments if they attach
 * to different attributes (busses, or whatever), to allow specification
 * of multiple match and attach functions.  There is only one configuration
 * driver per driver, so that things like unit numbers and the device
 * structure array will be shared.
 */
struct cfattach {
    size_t      ca_devsize;        /* size of dev data (for malloc) */
    cfmatch_t ca_match;        /* returns a match level */
    void    (*ca_attach)(struct device *, struct device *, void *);
    int    (*ca_detach)(struct device *, int);
    int    (*ca_activate)(struct device *, int);
};


enum {
    BUS_DMASYNC_PREREAD,
    BUS_DMASYNC_PREWRITE,
    BUS_DMASYNC_POSTREAD,
    BUS_DMASYNC_POSTWRITE
};

struct cfdriverlist {
    struct cfdriver cd[100];
    size_t size;
};

struct cfattachlist {
    struct cfattach ca[100];
    int size;
};

//extern struct cfattachlist calist;
//
//static void register_cfattach(struct cfattach *ca)
//{
//    calist.ca[calist.len] = ca;
//    calist.len ++;
//
//    IOLog("--%s: line = %d calist.len = %d", __FUNCTION__, __LINE__, calist.len);
//    IOSleep(1000);
//}
//
//#define register_ca(name) \
//extern struct cfattach name##_ca; \
//register_cfattach(&name##_ca);

#endif
