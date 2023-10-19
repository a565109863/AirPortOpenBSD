//
//  bus.c
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/6/19.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "bus.h"
#include "AirPort_OpenBSD.hpp"

int bus_dmamap_create(bus_dma_tag_t t, bus_size_t size, int nsegments, bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *map)
{
    int err = 0;
    
    if (map == NULL)
        return 1;
    (*map) = (bus_dmamap *)malloc(sizeof(struct bus_dmamap), M_DEVBUF, M_NOWAIT);
    (*map)->mbufCursor = IOMbufNaturalMemoryCursor::withSpecification(maxsegsz, nsegments);
    if ((*map)->mbufCursor == NULL) {
        err = 1;
        goto fail;
    }
    (*map)->dm_mapsize = maxsegsz;
    (*map)->_dm_segcnt = nsegments;
    (*map)->dm_segs = (bus_dma_segment_t *)mallocarray((*map)->_dm_segcnt, sizeof(bus_dma_segment_t), M_DEVBUF, M_NOWAIT);
    (*map)->alignment = 1;
    
    (*map)->_dm_cookie = malloc(sizeof(struct dm_cookie), M_DEVBUF, M_NOWAIT);
    
    SLIST_INSERT_HEAD(&t->bus_dmamap_list, *map, next);
    
fail:
    return err;
}

void bus_dmamap_destroy(bus_dma_tag_t t, bus_dmamap_t map)
{
    if (map == NULL)
        return;
    
    if (map->_dm_cookie) {
        free(map->_dm_cookie, M_DEVBUF, sizeof(struct dm_cookie));
    }
    if (map->dm_segs) {
        free(map->dm_segs, M_DEVBUF, map->_dm_segcnt * sizeof(bus_dma_segment_t));
    }
        
    if (map->mbufCursor) {
        map->mbufCursor->release();
        map->mbufCursor = NULL;
    }
    
    SLIST_REMOVE(&t->bus_dmamap_list, map, bus_dmamap, next);
    
    free(map, M_DEVBUF, sizeof(struct bus_dmamap));
    map = NULL;
}

int  bus_dmamem_alloc(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
int flags)
{
    int err = 0;
    bus_dmamap_t map = SLIST_FIRST(&t->bus_dmamap_list);
    
    if (map == NULL) {
        err = 1;
        goto fail;
    }
    
    map->bufDes = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), size, 0xFFFFFFFFFFFFF000ull);
    if (map->bufDes == NULL) {
        err = 1;
        goto fail;
    }
    
    segs->ds_addr = (bus_addr_t)map->bufDes->getBytesNoCopy();
    segs->ds_len = map->bufDes->getLength();
    
    bzero((void *)segs->ds_addr, segs->ds_len);
    
    map->alignment = alignment;
    map->kvap = segs->ds_addr;
    
fail:
    return err;
}

void bus_dmamem_free(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs)
{
    bus_dmamap_t map, map_info, tmp;
    SLIST_FOREACH_SAFE(map_info, &t->bus_dmamap_list, next, tmp) {
        // 查找bssid和channel
        if (segs->ds_addr == map_info->kvap) {
            // 找到
            map = map_info;
            break;
        }
    }
    
    if (map == NULL) {
        return;
    }
    
    if (map->bufDes == NULL) {
        return;
    }
    
    map->bufDes->release();
    map->bufDes = NULL;
    
//    segs->ds_addr = NULL;
//    segs->ds_len = 0;
//
//    bzero((void *)segs, sizeof(bus_dma_segment_t));
}

int bus_dmamem_map(bus_dma_tag_t t, bus_dma_segment_t *segs, int nsegs,
    size_t size, caddr_t *kvap, int flags)
{
    int err = 0;
    
    bus_dmamap_t map, map_info, tmp;
    SLIST_FOREACH_SAFE(map_info, &t->bus_dmamap_list, next, tmp) {
        // 查找bssid和channel
        if (segs->ds_addr == map_info->kvap) {
            // 找到
            map = map_info;
            break;
        }
    }
    
    if (map == NULL) {
        err = 1;
        goto done;
    }
    
    if (map->bufDes == NULL) {
        err = 1;
        goto done;
    }
    
    if (map->bufDes->prepare() != kIOReturnSuccess) {
        printf("prepare()\n");
        err = 1;
        goto fail1;
    }
    
    *kvap = (caddr_t)segs->ds_addr;

done:
    return err;

fail1:
    map->bufDes->release();
    map->bufDes = NULL;
    goto done;
}

void bus_dmamem_unmap(bus_dma_tag_t t, void *kvap,  bus_size_t size)
{
    bus_dmamap_t map, map_info, tmp;
    SLIST_FOREACH_SAFE(map_info, &t->bus_dmamap_list, next, tmp) {
        // 查找bssid和channel
        if (kvap == (void *)map_info->kvap) {
            // 找到
            map = map_info;
            break;
        }
    }
    
    if (map == NULL) {
        return;
    }
    
    if (map->bufDes == NULL) {
        return;
    }
    
    if (kvap == NULL) {
        return;
    }
    
    map->bufDes->complete();
}

int _bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, void *buf, bus_size_t buflen)
{
    int err = 0;

    UInt64 offset = 0;
    UInt32 numSegs = 1;
    struct dm_cookie *_dm_cookie;
    
    if (map == NULL) {
        err = 1;
        goto done;
    }
    
    _dm_cookie = (struct dm_cookie *)map->_dm_cookie;
    
    _dm_cookie->memDes = IOBufferMemoryDescriptor::withAddress(buf, buflen, kIODirectionInOut);
    if (_dm_cookie->memDes == NULL) {
        err = 1;
        goto done;
    }
    
    if (_dm_cookie->memDes->prepare() != kIOReturnSuccess) {
        printf("prepare()\n");
        err = 1;
        goto fail1;
    }

    _dm_cookie->dmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, buflen, IODMACommand::kMapped, 0, map->alignment);
    if (_dm_cookie->dmaCmd == NULL) {
        printf("withSpecification()\n");
        err = 1;
        goto fail2;
    }

    if (_dm_cookie->dmaCmd->setMemoryDescriptor(_dm_cookie->memDes) != kIOReturnSuccess) {
        printf("setMemoryDescriptor() failed.\n");
        err = 1;
        goto fail3;
    }

    map->dm_nsegs = 0;
    while (offset < buflen) {
        if (_dm_cookie->dmaCmd->genIOVMSegments(&offset, &map->dm_segs[map->dm_nsegs++], &numSegs) != kIOReturnSuccess) {
            printf("genIOVMSegments() failed.\n");
            err = 1;
            goto fail4;
        }
    }

done:
    return err;
    
fail4:
    _dm_cookie->dmaCmd->clearMemoryDescriptor();
    
fail3:
    _dm_cookie->dmaCmd->release();
    _dm_cookie->dmaCmd = NULL;
fail2:
    _dm_cookie->memDes->complete();
fail1:
    _dm_cookie->memDes->release();
    _dm_cookie->memDes = NULL;
    goto done;
}

int bus_dmamap_load_mbuf(bus_dma_tag_t t, bus_dmamap_t map, mbuf_t m0,
    int flags)
{
    if (map == NULL || m0 == NULL)
        return 2;

    int err = 0;

    if (map->_dm_segcnt == 1) {
        map->dm_nsegs = map->mbufCursor->getPhysicalSegments(m0, map->dm_segs, map->_dm_segcnt);
    } else {
        map->dm_nsegs = map->mbufCursor->getPhysicalSegmentsWithCoalesce(m0, map->dm_segs, map->_dm_segcnt);
    }
    if (map->dm_nsegs == 0)
        err = 1;
    else
        err = 0;
    
fail:
    return err;
}

int bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, void *buf,
bus_size_t buflen, struct proc *p, int flags)
{
    return _bus_dmamap_load(t, map, buf, buflen);
}

int bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, mbuf_t m0,
bus_size_t buflen, struct proc *p, int flags)
{
    if (buflen > 0) {
        mbuf_setlen(m0, buflen);
        mbuf_pkthdr_setlen(m0, buflen);
    }
    return bus_dmamap_load_mbuf(t, map, m0, flags);
}

int bus_dmamap_load_raw(bus_dma_tag_t t, bus_dmamap_t map, bus_dma_segment_t *segs,
int nsegs, bus_size_t size, int flags)
{
    return _bus_dmamap_load(t, map, (caddr_t)segs->ds_addr, size);
}

void bus_dmamap_unload(bus_dma_tag_t t, bus_dmamap_t map)
{
    if (map == NULL) {
        return;
    }
    
    if (map->_dm_cookie) {
        struct dm_cookie *_dm_cookie = (struct dm_cookie *)map->_dm_cookie;
        if (_dm_cookie->dmaCmd) {
            _dm_cookie->dmaCmd->clearMemoryDescriptor();
            _dm_cookie->dmaCmd->release();
            _dm_cookie->dmaCmd = NULL;
        }
        if (_dm_cookie->memDes) {
            _dm_cookie->memDes->complete();
            _dm_cookie->memDes->release();
            _dm_cookie->memDes = NULL;
        }
    }
    
    /*
     * No resources to free; just mark the mappings as
     * invalid.
     */
    bzero(map->dm_segs, map->_dm_segcnt * sizeof(bus_dma_segment_t));
    map->dm_nsegs = 0;
}

void bus_dmamap_sync(bus_dma_tag_t tag, bus_dmamap_t map, bus_addr_t offset, bus_size_t len, int ops)
{
//    OSSynchronizeIO();
    return;
}
