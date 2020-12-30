//
//  mutex.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef mutex_hpp
#define mutex_hpp

struct mutex {
    IORecursiveLock *lock;
};

#define MUTEX_ASSERT_LOCKED(mtx) do { } while (0)

static void
__mtx_init(struct mutex *mtx)
{
    mtx->lock = IORecursiveLockAlloc();
}

static void
__mtx_enter(struct mutex *mtx)
{
    IORecursiveLockLock(mtx->lock);
}

static void
__mtx_leave(struct mutex *mtx)
{
    IORecursiveLockUnlock(mtx->lock);
}

#define _mtx_init(mtx, ipl)         __mtx_init((mtx))
#define _mtx_init_flags(m,i,n,f,t)  _mtx_init(m,i)
#define _mtx_enter(m)               __mtx_enter(m)
#define _mtx_enter_try(m)           __mtx_enter_try(m)
#define _mtx_leave(m)               __mtx_leave(m)

#define mtx_init(m, ipl)    _mtx_init_flags(m, ipl, NULL, 0, 0)
#define mtx_enter(m)        _mtx_enter(m)
#define mtx_enter_try(m)    _mtx_enter_try(m)
#define mtx_leave(m)        _mtx_leave(m)

typedef struct mutex    pthread_mutex_t;
typedef struct    pthread_cond*        pthread_cond_t;

#endif /* mutex_hpp */
