//
//  uvm_extern.h
//  AppleIntelWiFi
//
//  Created by Zhong-Mac on 2020/10/15.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef uvm_extern_h
#define uvm_extern_h


/*
 * uvm_object: all that is left of mach objects.
 */

struct uvm_object {
    void        *pgops;        /* pager ops */
    RBT_HEAD(uvm_objtree, vm_page)     memt;        /* pages in object */
    int                 uo_npages;    /* # of pages in memt */
    int                 uo_refs;    /* reference count */
};



struct uvm_constraint_range {
   paddr_t    ucr_low;
   paddr_t ucr_high;
};


/*
 * Allocation mode for physical pages.
 *
 *  kp_constraint - allocation constraint for physical pages.
 *  kp_object - if the pages should be allocated from an object.
 *  kp_align - physical alignment of the first page in the allocation.
 *  kp_boundary - boundary that the physical addresses can't cross if
 *   the allocation is contiguous.
 *  kp_nomem - don't allocate any backing pages.
 *  kp_maxseg - maximal amount of contiguous segments.
 *  kp_zero - zero the returned memory.
 *  kp_pageable - allocate pageable memory.
 */
struct kmem_pa_mode {
    struct uvm_constraint_range *kp_constraint;
    struct uvm_object **kp_object;
    paddr_t kp_align;
    paddr_t kp_boundary;
    int kp_maxseg;
    char kp_nomem;
    char kp_zero;
    char kp_pageable;
};


/*
* Allocation mode for virtual space.
*
*  kv_map - pointer to the pointer to the map we're allocating from.
*  kv_align - alignment.
*  kv_wait - wait for free space in the map if it's full. The default
*   allocators don't wait since running out of space in kernel_map and
*   kmem_map is usually fatal. Special maps like exec_map are specifically
*   limited, so waiting for space in them is necessary.
*  kv_singlepage - use the single page allocator.
*  kv_executable - map the physical pages with PROT_EXEC.
*/
struct kmem_va_mode {
void *kv_map;
vsize_t kv_align;
char kv_wait;
char kv_singlepage;
};

struct vm_map *kmem_map = NULL;

static struct kmem_va_mode kv_intrsafe = {
    .kv_map = &kmem_map,
};

#endif /* uvm_extern_h */
