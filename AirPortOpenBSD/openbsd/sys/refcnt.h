//
//  refcnt.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/13.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef refcnt_h
#define refcnt_h

static void refcnt_init(struct refcnt *task_refs)
{
    
}

static void refcnt_take(struct refcnt *task_refs)
{
    
}

static void refcnt_rele_wake(struct refcnt *task_refs)
{
    
}

static void refcnt_rele(struct refcnt *task_refs)
{
    
}

static void refcnt_finalize(struct refcnt *task_refs, const char *str) {
    
}

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

#endif /* refcnt_h */
