//
//  qsort.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2021/11/23.
//

#ifndef qsort_h
#define qsort_h

#include <sys/_kernel.h>
#include <heapsort.h>

extern int errno;

void
qsort(void *a, size_t n, size_t es, int (*cmp)(const void *, const void *));

#endif /* qsort_h */
