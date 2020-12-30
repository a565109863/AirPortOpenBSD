//
//  kern_compat.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/2.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef kern_compat_h
#define kern_compat_h

#define splsoftnet() 1
#define splvm() 1
#define splnet() 1
#define splx(x)

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif

/* Macros to clear/set/test flags. */
#define SET(t, f)       (t) |= (f)
#define CLR(t, f)       (t) &= ~(f)
#define ISSET(t, f)     ((t) & (f))

#endif /* kern_compat_h */
