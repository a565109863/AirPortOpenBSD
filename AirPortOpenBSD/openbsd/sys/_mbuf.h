//
//  mbuf.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef mbuf_hpp
#define mbuf_hpp

#include "_kernel.h"
#include "mutex.h"

#define    mtod(x,t)    ((t) mbuf_data(x))

#define m_freem(m)  mbuf_freem(m)
#define m_free(m)   mbuf_free(m)


/*
 * Mbufs are of a single size, MSIZE, which includes overhead.  An mbuf may
 * add a single "mbuf cluster" of size MCLBYTES, which has no additional
 * overhead and is used instead of the internal data area; this is done when
 * at least MINCLSIZE of data must be stored.
 */


#define    M_EXTWR          0x0008    /* external storage is writable */
#define    MAXMCLBYTES      (64 * 1024)        /* largest cluster from the stack */
#define    MINCLSIZE        mbuf_get_minclsize()

/* length to m_copy to copy all */
#define M_COPYALL           MBUF_COPYALL

#define M_DONTWAIT          MBUF_DONTWAIT
#define M_WAIT              MBUF_WAITOK

#define MLEN                mbuf_get_mlen()
#define MHLEN               mbuf_get_mhlen()
#define M_EXT               MBUF_EXT
#define M_MCAST             MBUF_MCAST

#define m_adj               mbuf_adj
#define m_align             mbuf_align_32
#define m_cat               mbuf_concatenate
#define m_copydata          mbuf_copydata
#define m_trailingspace     mbuf_trailingspace
#define m_copyback          mbuf_copyback

#define M_PREPEND(m, len, how)  mbuf_prepend(&m, len, M_WAIT)
#define MGET(m, how, type)      mbuf_get(M_WAIT, type, &m)
#define MGETHDR(m, how, type)   mbuf_gethdr(M_WAIT, type, &m)

#define MCLGET(m, how) (void)   m_clget((m), (how), MCLBYTES)
#define MCLGETL(m, how, l)      m_clget((m), (how), (l))

#define m_dup_pkthdr(dest, src, how)    mbuf_copy_pkthdr(dest, src)

/*
 * Make a copy of an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
static mbuf_t
m_copym(mbuf_t m0, int off, int len, mbuf_how_t how)
{
    mbuf_t m;
    mbuf_copym(m0, off, len, M_WAIT, &m);
    return m;
    
}

static mbuf_t
m_gethdr(mbuf_how_t how, mbuf_type_t type)
{
    mbuf_t m;
    MGETHDR(m, how, type);
    return m;
}

static mbuf_t
m_clget(mbuf_t m, mbuf_how_t how, size_t pktlen)
{
    if (m == NULL) {
        m = m_gethdr(how, MT_DATA);
        if (m == NULL)
            return (NULL);
    }
    
    if (pktlen <= MCLBYTES) {
        pktlen = MCLBYTES;
    } else if (pktlen <= MBIGCLBYTES) {
        pktlen = MBIGCLBYTES;
    } else if (pktlen <= M16KCLBYTES) {
        pktlen = M16KCLBYTES;
    }
    
    mbuf_getcluster(M_WAIT, mbuf_type(m), pktlen, &m);
    
    return m;
}

static mbuf_t
m_split(mbuf_t m0, int len, mbuf_how_t how)
{
    mbuf_t m;
    mbuf_split(m0, len, M_WAIT, &m);
    return m;
}

static mbuf_t
m_pullup(mbuf_t m, int len)
{
    mbuf_pullup(&m, len);
    
    return m;
}


static mbuf_t m_dup_pkt(mbuf_t m0, unsigned int adj, mbuf_how_t how)
{
    mbuf_t m;
    int len;

    len = mbuf_pkthdr_len(m0) + adj;
    if (len > MAXMCLBYTES) /* XXX */
        return (NULL);

    MGET(m, how, mbuf_type(m0));
    if (m == NULL)
        return (NULL);

    if (m_dup_pkthdr(m, m0, how) != 0)
        goto fail;

    if (len > MHLEN) {
        MCLGETL(m, how, len);
        if (!ISSET(mbuf_type(m), M_EXT))
            goto fail;
    }

    mbuf_setlen(m, len);
    mbuf_pkthdr_setlen(m, len);
    m_adj(m, adj);
    m_copydata(m0, 0, mbuf_pkthdr_len(m0), mtod(m, caddr_t));

    return (m);

fail:
    m_freem(m);
    return (NULL);

}

/*
 * mbuf chain defragmenter. This function uses some evil tricks to defragment
 * an mbuf chain into a single buffer without changing the mbuf pointer.
 * This needs to know a lot of the mbuf internals to make this work.
 */
static inline int
m_defrag(mbuf_t m, mbuf_how_t how)
{
    mbuf_t m0;

    if (mbuf_next(m) == NULL)
        return (0);

    KASSERT(m->m_flags & M_PKTHDR);

    if ((m0 = m_gethdr(how, mbuf_type(m))) == NULL)
        return (ENOBUFS);
    if (mbuf_pkthdr_len(m) > MHLEN) {
        MCLGETL(m0, how, mbuf_pkthdr_len(m));
        if (!(mbuf_flags(m0) & M_EXT)) {
            m_free(m0);
            return (ENOBUFS);
        }
    }
    m_copydata(m, 0, mbuf_pkthdr_len(m), mtod(m0, caddr_t));
    mbuf_pkthdr_setlen(m0, mbuf_pkthdr_len(m));
    mbuf_setlen(m0, mbuf_pkthdr_len(m));

    /* free chain behind and possible ext buf on the first mbuf */
    m_freem(mbuf_next(m));
    mbuf_setnext(m, NULL);

    /*
     * Bounce copy mbuf over to the original mbuf and set everything up.
     * This needs to reset or clear all pointers that may go into the
     * original mbuf chain.
     */
    if (!(mbuf_flags(m0) & MBUF_EXT)) {
        mbuf_setdata(m, mbuf_pkthdr_header(m), mbuf_pkthdr_len(m));
        memcpy(mbuf_data(m), mbuf_data(m0), mbuf_len(m0));
    }

    mbuf_pkthdr_setlen(m, mbuf_len(m0));
    mbuf_setlen(m, mbuf_len(m0));

    mbuf_setflags(m0, mbuf_flags(m0) & ~(MBUF_EXT | M_EXTWR));    /* cluster is gone */
    m_free(m0);

    return (0);
}


/*
 * mbuf lists
 */

struct mbuf_list {
    mbuf_t      ml_head;
    mbuf_t      ml_tail;
    u_int       ml_len;
};


#define MBUF_LIST_INITIALIZER() { NULL, NULL, 0 }

/*
 * mbuf lists
 */

#define    ml_len(_ml)          ((_ml)->ml_len)
#define    ml_empty(_ml)        ((_ml)->ml_len == 0)

static void
ml_init(struct mbuf_list *ml)
{
    ml->ml_head = ml->ml_tail = NULL;
    ml->ml_len = 0;
}

static void
ml_enqueue(struct mbuf_list *ml, mbuf_t m)
{
    if (ml->ml_tail == NULL)
        ml->ml_head = ml->ml_tail = m;
    else {
        mbuf_setnextpkt(ml->ml_tail, m);
        ml->ml_tail = m;
    }
    
    mbuf_setnextpkt(m, NULL);
    ml->ml_len++;
}

static void
ml_enlist(struct mbuf_list *mla, struct mbuf_list *mlb)
{
    if (!ml_empty(mlb)) {
        if (ml_empty(mla))
            mla->ml_head = mlb->ml_head;
        else
            mbuf_setnextpkt(mla->ml_tail, mlb->ml_head);
        mla->ml_tail = mlb->ml_tail;
        mla->ml_len += mlb->ml_len;
        
        ml_init(mlb);
    }
}

static mbuf_t
ml_dequeue(struct mbuf_list *ml)
{
    mbuf_t m;
    
    m = ml->ml_head;
    if (m != NULL) {
        ml->ml_head = mbuf_nextpkt(m);
        if (ml->ml_head == NULL)
            ml->ml_tail = NULL;
        
        mbuf_setnextpkt(m, NULL);
        ml->ml_len--;
    }
    
    return (m);
}

static mbuf_t
ml_dechain(struct mbuf_list *ml)
{
    mbuf_t m0;
    
    m0 = ml->ml_head;
    
    ml_init(ml);
    
    return (m0);
}

static unsigned int
ml_purge(struct mbuf_list *ml)
{
    mbuf_t m, n;
    unsigned int len;
    
    for (m = ml->ml_head; m != NULL; m = n) {
        n = mbuf_nextpkt(m);
        m_freem(m);
    }
    
    len = ml->ml_len;
    ml_init(ml);
    
    return (len);
}






/*
 * mbuf queues
 */

struct mbuf_queue {
    struct mutex        mq_mtx;
    struct mbuf_list    mq_list;
    u_int               mq_maxlen;
    u_int               mq_drops;
};

#define    mq_len(_mq)              ml_len(&(_mq)->mq_list)
#define    mq_empty(_mq)            ml_empty(&(_mq)->mq_list)
#define    mq_full(_mq)             (mq_len((_mq)) >= (_mq)->mq_maxlen)
#define    mq_drops(_mq)            ((_mq)->mq_drops)
#define    mq_set_maxlen(_mq, _l)   ((_mq)->mq_maxlen = (_l))

static void
mq_init(struct mbuf_queue *mq, u_int maxlen, int ipl)
{
    mtx_init(&mq->mq_mtx, ipl);
    ml_init(&mq->mq_list);
    mq->mq_maxlen = maxlen;
}

static int
mq_enqueue(struct mbuf_queue *mq, mbuf_t m)
{
    int dropped = 0;
    
    mtx_enter(&mq->mq_mtx);
    if (mq_len(mq) < mq->mq_maxlen)
        ml_enqueue(&mq->mq_list, m);
    else {
        mq->mq_drops++;
        dropped = 1;
    }
    mtx_leave(&mq->mq_mtx);
    
    if (dropped) {
        m_freem(m);
    }
    
    return (dropped);
}

static mbuf_t
mq_dequeue(struct mbuf_queue *mq)
{
    mbuf_t m;
    
    mtx_enter(&mq->mq_mtx);
    m = ml_dequeue(&mq->mq_list);
    mtx_leave(&mq->mq_mtx);
    
    return (m);
}


static void
mq_init(IOPacketQueue **mq, u_int maxlen, int ipl)
{
    (*mq) = IOPacketQueue::withCapacity(4096);
}

static int
mq_enqueue(IOPacketQueue **mq, mbuf_t m)
{
    bool dropped = !(*mq)->lockEnqueue(m);
    
    if (dropped) {
        m_freem(m);
    }
    
    return (dropped);
}

static mbuf_t
mq_dequeue(IOPacketQueue **mq)
{
    return (*mq)->lockDequeue();
}


static int
mq_enlist(struct mbuf_queue *mq, struct mbuf_list *ml)
{
    mbuf_t m;
    int dropped = 0;
    
    mtx_enter(&mq->mq_mtx);
    if (mq_len(mq) < mq->mq_maxlen)
        ml_enlist(&mq->mq_list, ml);
    else {
        dropped = ml_len(ml);
        mq->mq_drops += dropped;
    }
    mtx_leave(&mq->mq_mtx);
    
    if (dropped) {
        while ((m = ml_dequeue(ml)) != NULL)
            m_freem(m);
    }
    
    return (dropped);
}

static void
mq_delist(struct mbuf_queue *mq, struct mbuf_list *ml)
{
    mtx_enter(&mq->mq_mtx);
    *ml = mq->mq_list;
    ml_init(&mq->mq_list);
    mtx_leave(&mq->mq_mtx);
}


static mbuf_t
mq_dechain(struct mbuf_queue *mq)
{
    mbuf_t m0;
    
    mtx_enter(&mq->mq_mtx);
    m0 = ml_dechain(&mq->mq_list);
    mtx_leave(&mq->mq_mtx);
    
    return (m0);
}

static unsigned int
mq_purge(struct mbuf_queue *mq)
{
    struct mbuf_list ml;
    
    mq_delist(mq, &ml);
    
    return (ml_purge(&ml));
}



#endif /* mbuf_hpp */
