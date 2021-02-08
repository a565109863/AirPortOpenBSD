//
//  _malloc.c
//  AirPortOpenBSD
//
//  Created by User-Mac on 2021/2/8.
//

#include <sys/_malloc.h>

LIST_HEAD(,malloc_ptr) ptr_list = LIST_HEAD_INITIALIZER(ptr_list);

void* malloc(vm_size_t len, int type, int how) {
    struct malloc_ptr *ptr = (struct malloc_ptr *)IOMalloc(sizeof(*ptr));
    ptr->ptr = IOMalloc(len);
    if (ptr->ptr == NULL) {
        IOFree(ptr, sizeof(*ptr));
        return NULL;
    }
    ptr->size = len;
    bzero(ptr->ptr, ptr->size);
    
    LIST_INSERT_HEAD(&ptr_list, ptr, list);
    
    return ptr->ptr;
}

void free(void* addr, int type, vm_size_t len)
{
    if (addr == NULL) {
        return;
    }
    
    struct malloc_ptr *ptr, *tmp;
    LIST_FOREACH_SAFE(ptr, &ptr_list, list, tmp) {
        if (ptr->ptr == addr) {
            LIST_REMOVE(ptr, list);
            IOFree(ptr->ptr, ptr->size);
            IOFree(ptr, sizeof(*ptr));
            return;
        }
    }
    
    if (len > 0) {
        IOFree(addr, len);
    }
}
