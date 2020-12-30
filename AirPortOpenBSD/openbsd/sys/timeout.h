/*    $OpenBSD: timeout.h,v 1.27 2017/11/24 02:36:53 dlg Exp $    */
/*
 * Copyright (c) 2000-2001 Artur Grabowski <art@openbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_TIMEOUT_H_
#define _SYS_TIMEOUT_H_

#include <sys/_kernel.h>
#include <sys/task.h>
#include <sys/libkern.h>
#include <netinet/_if_ether.h>

enum{
    MSECS,
    SECS,
    USECS
};

class IOTimeout : public OSObject {
    OSDeclareDefaultStructors(IOTimeout)
    
public:
    static void timeout_run(OSObject* obj, IOTimerEventSource* timer);
public:
    IOTimerEventSource* timer;
    char *fn_name;
    void (*fn)(void*);
    void* arg;
};

struct timeout {
    IOTimeout* vt;
};

void timeout_set(struct timeout *t, void (*fn)(void *), void *arg);
void timeout_add(struct timeout *t, int msecs);
void timeout_add_msec(struct timeout *t,int msecs);
void timeout_add_sec(struct timeout *t, int secs);
void timeout_add_usec(struct timeout *t,int usecs);
void timeout_del(struct timeout *t);

#define timeout_initialized(to) 1 //((to)->to_flags & TIMEOUT_INITIALIZED)

#define timeout_pending(t) ((t) && ((t)->vt) && (t)->vt->timer && ((t)->vt->timer->onThread()) && (!(t)->vt->timer->checkForWork()))

#endif    /* _SYS_TIMEOUT_H_ */
