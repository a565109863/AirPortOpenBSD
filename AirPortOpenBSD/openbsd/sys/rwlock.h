//
//  rwlock.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/13.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef rwlock_h
#define rwlock_h

#define RW_WRITE        0x0001UL /* exclusive lock */

#define RW_INTR            0x0010UL /* interruptible sleep */

struct rwlock {
    IORWLock *_rwlock;
};

static void rw_init(struct rwlock *ioctl_rwl, const char *str)
{
    
}

static void rw_assert_wrlock(struct rwlock *ioctl_rwl)
{
    
}

static int rw_enter(struct rwlock *ioctl_rwl, UInt32 type)
{
    return 0;
}

static void rw_enter_write(struct rwlock *ioctl_rwl)
{
    
}

static void rw_exit_write(struct rwlock *ioctl_rwl)
{
    
}

static void rw_exit(struct rwlock *ioctl_rwl)
{
    
}

#endif /* rwlock_h */
