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

#endif /* rwlock_h */
