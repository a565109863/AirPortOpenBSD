//
//  libkern.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef libkern_hpp
#define libkern_hpp

#include <sys/random.h>
#include <sys/fls.h>
#include <sys/timingsafe_bcmp.h>

/* Return one word of randomness from a ChaCha20 generator */
static u_int32_t
arc4random(void)
{
    u_int32_t ret;
    read_random(&ret, sizeof(ret));
    return ret;
}

/*
 * Fill a buffer of arbitrary length with ChaCha20-derived randomness.
 */
static  inline void
arc4random_buf(void *buf, size_t n)
{
    read_random(buf, (u_int)n);
}

/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
static u_int32_t
arc4random_uniform(u_int32_t upper_bound)
{
    u_int32_t r, min;
    
    if (upper_bound < 2)
        return 0;
    
    /* 2**32 % x == (2**32 - x) % x */
    min = -upper_bound % upper_bound;
    
    /*
     * This could theoretically loop forever but each retry has
     * p > 0.5 (worst case, usually far better) of selecting a
     * number inside the range we need, so it should rarely need
     * to re-roll.
     */
    for (;;) {
        r = arc4random();
        if (r >= min)
            break;
    }
    
    return r % upper_bound;
}

#define    IPL_NET        3    /* network */
#define    IPL_MPSAFE  4
#define    IPL_SOFTNET 5

#endif /* libkern_hpp */
