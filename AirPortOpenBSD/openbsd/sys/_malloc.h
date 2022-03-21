//
//  malloc.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/19.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef malloc_h
#define malloc_h

#include <sys/_kernel.h>

struct malloc_ptr {
    LIST_ENTRY(malloc_ptr)    list;
    size_t              size;
    void                *ptr;
};

void* malloc(vm_size_t len, int type, int how);

static void* mallocarray(vm_size_t n, vm_size_t len, int type, int how) {
    return malloc(n * len, type, how);
}

static inline void *
kcalloc(size_t n, size_t size, int flags)
{
//    if (n != 0 && SIZE_MAX / n < size)
//        return NULL;
    return malloc(n * size, M_DEVBUF, flags | M_ZERO);
}

#define calloc(n, size) kcalloc(n, size, M_ZERO)

void *recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size);

void free(void* addr, int type, vm_size_t len);

#define km_alloc(s, kv, kp, kd) malloc(s, M_DEVBUF, M_WAIT)
#define km_free(v, s, kv, kp) free(v, M_WAIT, s)

static int m_copyin(const char *uaddr, void *kaddr, size_t len)
{
    memcpy(kaddr, (void *)uaddr, len);
    return 0;
}

static int m_copyout(const void *kaddr, char *uaddr, size_t len)
{
    memcpy((void *)uaddr, kaddr, len);
    return 0;
}

#undef copyin
#undef copyout
#define copyin(u, k, l) m_copyin((const char *)u, k, l)
#define copyout(k, u, l) m_copyout(k, (char *)u, l)

#endif /* malloc_h */
