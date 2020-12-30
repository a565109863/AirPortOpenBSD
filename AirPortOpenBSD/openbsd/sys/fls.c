//
//  fls.c
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/7/11.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <libkern/libkern.h>
#include <sys/fls.h>

/*
 * Find Last Set bit
 */
int fls(int mask)
{
    int bit;
    
    if (mask == 0)
        return (0);
    for (bit = 1; mask != 1; bit++)
        mask = (unsigned int)mask >> 1;
    return (bit);
}

/*
 * Find Last Set bit
 */
int flsl(long mask)
{
    int bit;
    
    if (mask == 0)
        return (0);
    for (bit = 1; mask != 1; bit++)
        mask = (unsigned long)mask >> 1;
    return (bit);
}
