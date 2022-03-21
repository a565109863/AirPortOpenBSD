//
//  heapsort.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2021/11/23.
//

#ifndef heapsort_h
#define heapsort_h


#include <sys/_kernel.h>
#include <sys/_malloc.h>

extern int errno;

int
heapsort(void *vbase, size_t nmemb, size_t size,
         int (*compar)(const void *, const void *));

#endif /* heapsort_h */
