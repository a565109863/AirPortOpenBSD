//
//  _malloc.c
//  AirPortOpenBSD
//
//  Created by User-Mac on 2021/2/8.
//

#include <sys/_malloc.h>

TAILQ_HEAD(,malloc_ptr) ptr_list = TAILQ_HEAD_INITIALIZER(ptr_list);

void* malloc(vm_size_t len, int type, int how) {
    struct malloc_ptr *ptr = (struct malloc_ptr *)IOMalloc(sizeof(*ptr));
    ptr->ptr = IOMalloc(len);
    if (ptr->ptr == NULL) {
        IOFree(ptr, sizeof(*ptr));
        return NULL;
    }
    ptr->size = len;
    bzero(ptr->ptr, ptr->size);
    
    TAILQ_INSERT_TAIL(&ptr_list, ptr, list);
    
    return ptr->ptr;
}

void free(void* addr, int type, vm_size_t len)
{
    if (addr == NULL) {
        return;
    }
    
    struct malloc_ptr *ptr, *tmp;
    TAILQ_FOREACH_SAFE(ptr, &ptr_list, list, tmp) {
        if (ptr->ptr == addr) {
            TAILQ_REMOVE(&ptr_list, ptr, list);
            IOFree(ptr->ptr, ptr->size);
            IOFree(ptr, sizeof(*ptr));
            return;
        }
    }
    
    if (len > 0) {
        IOFree(addr, len);
    }
}

void *
recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size)
{
    size_t oldsize, newsize;
    void *newptr;

    if (ptr == NULL)
        return calloc(newnmemb, size);

    newsize = newnmemb * size;

    oldsize = oldnmemb * size;
    
    newptr = malloc(newsize, M_DEVBUF, M_WAITOK);
    if (newptr == NULL)
        return NULL;

    if (newsize > oldsize) {
        memcpy(newptr, ptr, oldsize);
    } else
        memcpy(newptr, ptr, newsize);

    explicit_bzero(ptr, oldsize);
    free(ptr, M_DEVBUF, newnmemb * size);

    return newptr;
}
