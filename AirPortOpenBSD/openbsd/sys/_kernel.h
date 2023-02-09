//
//  _kernel.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/19.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef _kernel_h
#define _kernel_h

#include <sys/__types.h>
#include <sys/kern_compat.h>
#include <sys/libkern.h>

#include <crypto/md5.h>
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#include <crypto/aes.h>
#include <crypto/key_wrap.h>
#include <crypto/cmac.h>
#include <crypto/hmac.h>
#include <crypto/michael.h>

extern struct ifnet *_ifp;
extern IOWorkLoop *_fWorkloop;
extern IOCommandGate *_fCommandGate;

extern int logStr_i;
extern int debug_log;

//#define IEEE80211_STA_ONLY    1

#define INET6 1
#define NBPFILTER 1

#define BPF_DIRECTION_IN    1
#define BPF_DIRECTION_OUT   (1<<1)

#define IEEE80211_DEBUG     0
#define IWM_DEBUG           0
//#undef DEBUG

#ifdef DEBUG

    #if MAC_TARGET < __MAC_11_0
        #define DebugLog(args...) \
        if(debug_log) { \
            uint64_t new_thread_id = thread_tid(current_thread()); \
            int argsStrSize = 1024; \
            char *argsStr = (typeof argsStr)IOMalloc(argsStrSize); \
            snprintf(argsStr, argsStrSize, args); \
            kprintf("i=%d: %s: line = %d tid = %llu sysuptime = %llu %s", logStr_i++, __FUNCTION__, __LINE__, new_thread_id, sysuptime(), argsStr); \
            IOFree(argsStr, argsStrSize); \
        }
        #define DebugLogClean()
    #else
        #define DebugLog(args...) \
        if(debug_log) { \
            uint64_t new_thread_id = thread_tid(current_thread()); \
            int argsStrSize = 1024; \
            char *argsStr = (typeof argsStr)IOMalloc(argsStrSize); \
            snprintf(argsStr, argsStrSize, args); \
            kprintf("%s: line = %d tid = %llu sysuptime = %llu %s", __FUNCTION__, __LINE__, new_thread_id, sysuptime(), argsStr); \
            if (_ifp->if_softc != NULL) { \
                int logStrSize = 2048; \
                char *logStr = (typeof logStr)IOMalloc(logStrSize); \
                snprintf(logStr, logStrSize, "%s: line = %d tid = %llu sysuptime = %llu %s", __FUNCTION__, __LINE__, new_thread_id, sysuptime(), argsStr); \
                OSString *log = OSString::withCString(logStr); \
                int logKeySize = 256; \
                char *logKey = (typeof logKey)IOMalloc(logKeySize); \
                snprintf(logKey, logKeySize, "DebugLog_%06d", logStr_i++); \
                struct device *dev = (struct device *)_ifp->if_softc; \
                dev->dev->setProperty(logKey, log); \
                IOFree(logKey, logKeySize); \
                IOFree(logStr, logStrSize); \
            } \
            IOFree(argsStr, argsStrSize); \
        }

        #define DebugLogClean() \
        if(debug_log) { \
            if (_ifp->if_softc != NULL) { \
                int logKeySize = 256; \
                char *logKey = (typeof logKey)IOMalloc(logKeySize); \
                while(logStr_i-- > 0) { \
                    snprintf(logKey, logKeySize, "DebugLog_%06d", logStr_i); \
                    struct device *dev = (struct device *)_ifp->if_softc; \
                    dev->dev->removeProperty(logKey); \
                } \
                IOFree(logKey, logKeySize); \
            } \
            logStr_i = 0; \
        }
    #endif

#else
#define DebugLog(args...)
#define DebugLogClean()
#endif

#define DPRINTF(X) do {                \
if (IEEE80211_DEBUG) {            \
printf("%s: ", __func__);    \
printf X;            \
}                    \
} while(0)


#define err(x, arg...)      kprintf(arg)
#define errx(x, arg...)     kprintf(arg)
#define warnx(arg...)       kprintf(arg)
#define warn(arg...)        kprintf(arg)
#define putchar(x)          kprintf("%c", x)
#define fputs(x, y)         kprintf(x)

#define suser(x) 0

#define explicit_bzero bzero

#undef KASSERT
#define KASSERT(x)  assert(x)

#undef DELAY
#define DELAY IODelay
#define delay IODelay

#define __packed __attribute__((packed))
#define __force
#define __predict_false(exp)    ((exp) != 0)

#define splassert(x)
#define DEF_STRONG(x)

#define hz 1000

#define container_of(ptr,type,member) ((type*)(((uintptr_t)ptr) - offsetof(type, member)))

/*
 * XXX These are also used for aligning stack, which needs to be on a quad
 * word boundary for 88k.
 */
#define    _ALIGNED_POINTER(p,t)    ((((unsigned long)(p)) & (sizeof(t) - 1)) == 0)
#define    _MAX_PAGE_SHIFT          12    /* same as PAGE_SHIFT */

#define    NODEV    (dev_t)(-1)    /* non-existent device */

#define    ALIGNED_POINTER(p,t)    _ALIGNED_POINTER(p,t)

#define abs(x) ((x) < 0 ? (-x) : (x))


#define _KERNEL 1
#define M_DEVBUF 2

#define    M_CANFAIL    0x0004

#define    INFSLP    UINT64_MAX

// 毫秒
static time_t getuptime()
{
    uint64_t m, f;
    clock_get_uptime(&m);
    absolutetime_to_nanoseconds(m, &f);
    return (f / 1000000);
}

static inline void
USEC_TO_TIMEVAL(uint64_t us, struct timeval *tv)
{
    tv->tv_sec = us / 1000000;
    tv->tv_usec = us % 1000000;
}

/*
 * ppsratecheck(): packets (or events) per second limitation.
 */
static int
ppsratecheck(struct timeval *lasttime, int *curpps, int maxpps)
{
    struct timeval tv, delta;
    int rv;

    microuptime(&tv);

    timersub(&tv, lasttime, &delta);

    /*
     * check for 0,0 is so that the message will be seen at least once.
     * if more than one second have passed since the last update of
     * lasttime, reset the counter.
     *
     * we do increment *curpps even in *curpps < maxpps case, as some may
     * try to use *curpps for stat purposes as well.
     */
    if (maxpps == 0)
        rv = 0;
    else if ((lasttime->tv_sec == 0 && lasttime->tv_usec == 0) ||
        delta.tv_sec >= 1) {
        *lasttime = tv;
        *curpps = 0;
        rv = 1;
    } else if (maxpps < 0)
        rv = 1;
    else if (*curpps < maxpps)
        rv = 1;
    else
        rv = 0;

#if 1 /*DIAGNOSTIC?*/
    /* be careful about wrap-around */
    if (*curpps + 1 > *curpps)
        *curpps = *curpps + 1;
#else
    /*
     * assume that there's not too many calls to this function.
     * not sure if the assumption holds, as it depends on *caller's*
     * behavior, not the behavior of this function.
     * IMHO it is wrong to make assumption on the caller's behavior,
     * so the above #if is #if 1, not #ifdef DIAGNOSTIC.
     */
    *curpps = *curpps + 1;
#endif
    
    return (rv);
}

/*
 * ratecheck(): simple time-based rate-limit checking.  see ratecheck(9)
 * for usage and rationale.
 */
static int
ratecheck(struct timeval *lasttime, const struct timeval *mininterval)
{
    struct timeval tv, delta;
    int rv = 0;

    getmicrouptime(&tv);

    timersub(&tv, lasttime, &delta);

    /*
     * check for 0,0 is so that the message will be seen at least once,
     * even if interval is huge.
     */
    if (timercmp(&delta, mininterval, >=) ||
        (lasttime->tv_sec == 0 && lasttime->tv_usec == 0)) {
        *lasttime = tv;
        rv = 1;
    }

    return (rv);
}

static
void get_hexstring( u_int8_t *buf, char *_val, int lenp)
{
    int len = lenp;
    u_int8_t *p = buf;
    char *val = _val;
    
#define toChar(x)  (x) + ((x) > 9 ? (- 10 + 'a') : '0')
    
    while (p < buf + len) {
        val[0] = toChar(*p >> 4);
        val[1] = toChar(*p ^ *p >> 4 << 4);
        
        val += 2;
        p++;
    }
    val[0] = '\0';

}

static inline uint64_t sysuptime()
{
    struct timeval tv;
    
    microuptime(&tv);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


#define DWORD UInt32
#define QWORD UInt64
#define HIQWORD(l)           ((DWORD)((((QWORD)(l)) >> 32) & 0xffffffff))

static uint32_t sysuptime1() {
    uint64_t    result;
    absolutetime_to_nanoseconds(mach_absolute_time(), &result);
    uint32_t rax = HIQWORD(0x431bde82d7b634db * result) >> 0x12;
    return rax;
}


#endif /* _kernel_h */
