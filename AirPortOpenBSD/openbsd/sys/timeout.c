/*    $OpenBSD: kern_timeout.c,v 1.53 2017/12/14 02:42:18 dlg Exp $    */
/*
 * Copyright (c) 2001 Thomas Nordin <nordin@openbsd.org>
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

#include <sys/timeout.h>
#include <IOKit/IOCommandGate.h>

void IOTimeout::timeout_run(OSObject* obj, IOTimerEventSource* timer)
{
    if (obj == NULL) {
        return;
    }
    IOTimeout *vt = OSDynamicCast(IOTimeout, obj);
    if (vt == NULL) {
        return;
    }
    vt->fn(vt->arg);
}

void timeout_set(struct timeout* t, void (*func)(void *), void* arg)
{
    if (t->vt == NULL) {
        t->vt = new IOTimeout();
    }
    t->vt->timer = IOTimerEventSource::timerEventSource(t->vt,(IOTimerEventSource::Action)IOTimeout::timeout_run);
    _ifp->fWorkloop->addEventSource(t->vt->timer);
    t->vt->fn = func;
    t->vt->arg = arg;
}

void _timeout_add(struct timeout *t, UInt32 time, int type)
{
    if (t == NULL || t->vt == NULL || t->vt->timer == NULL) {
        return;
    }
    
    t->vt->timer->cancelTimeout();
    switch (type) {
        case SECS:
            t->vt->timer->setTimeout(time);
            break;
        case MSECS:
            t->vt->timer->setTimeoutMS(time);
            break;
        case USECS:
        default:
            t->vt->timer->setTimeoutUS(time);
            break;
    }
}

void timeout_add(struct timeout *t, int msecs)
{
    return _timeout_add(t, msecs, USECS);
}

void timeout_add_msec(struct timeout* t, int msecs)
{
    return _timeout_add(t, msecs, MSECS);
}

void timeout_add_sec(struct timeout* t, int secs)
{
    return _timeout_add(t, secs, SECS);
}

void timeout_add_usec(struct timeout* t, int usecs)
{
    return _timeout_add(t, usecs, USECS);
}

void timeout_del(struct timeout* t)
{
    if (t == NULL || t->vt == NULL || t->vt->timer == NULL) {
        return;
    }
    t->vt->timer->cancelTimeout();
}
