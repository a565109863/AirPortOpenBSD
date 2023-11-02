
#ifndef _IO80211WORKQUEUE_H
#define _IO80211WORKQUEUE_H

#include <Availability.h>
#include <IOKit/IOWorkLoop.h>

class IO80211WorkQueue : public IOWorkLoop
{
    OSDeclareDefaultStructors( IO80211WorkQueue )

public:
    
    virtual IOThread getThread() const APPLE_KEXT_OVERRIDE;
    virtual void enableAllInterrupts() const APPLE_KEXT_OVERRIDE;
    virtual void disableAllInterrupts() const APPLE_KEXT_OVERRIDE;
    virtual IOReturn runAction(Action action, OSObject *target,
        void *arg0 = NULL, void *arg1 = NULL,
        void *arg2 = NULL, void *arg3 = NULL) APPLE_KEXT_OVERRIDE;
    virtual int commandSleep(void *,unsigned long long);
    virtual void commandWakeup(void *);
    
    static IO80211WorkQueue * workQueue();
    
public:
    uint8_t filter[0x50];
};

#endif
