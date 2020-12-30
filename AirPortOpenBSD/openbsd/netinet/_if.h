//
//  _if.h
//  AppleIntelWiFi
//
//  Created by Zhong-Mac on 2020/10/15.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef _if_h
#define _if_h


struct if_rxring {
    int     rxr_adjusted;
    u_int    rxr_alive;
    u_int    rxr_cwm;
    u_int    rxr_lwm;
    u_int    rxr_hwm;
};

static
void
if_rxr_init(struct if_rxring *rxr, u_int lwm, u_int hwm)
{
    extern int ticks;

    memset(rxr, 0, sizeof(*rxr));

    rxr->rxr_adjusted = ticks;
    rxr->rxr_cwm = rxr->rxr_lwm = lwm;
    rxr->rxr_hwm = hwm;
}

static inline void
if_rxr_adjust_cwm(struct if_rxring *rxr)
{
    extern int ticks;

    if (rxr->rxr_alive >= rxr->rxr_lwm)
        return;
    else if (rxr->rxr_cwm < rxr->rxr_hwm)
        rxr->rxr_cwm++;

    rxr->rxr_adjusted = ticks;
}

static
u_int
if_rxr_get(struct if_rxring *rxr, u_int max)
{
    extern int ticks;
    u_int diff;

    if (ticks - rxr->rxr_adjusted >= 1) {
        /* we're free to try for an adjustment */
        if_rxr_adjust_cwm(rxr);
    }

    if (rxr->rxr_alive >= rxr->rxr_cwm)
        return (0);

    diff = min(rxr->rxr_cwm - rxr->rxr_alive, max);
    rxr->rxr_alive += diff;

    return (diff);
}


#define if_rxr_put(_r, _c)    do { (_r)->rxr_alive -= (_c); } while (0)
#define if_rxr_needrefill(_r)    ((_r)->rxr_alive < (_r)->rxr_lwm)
#define if_rxr_inuse(_r)    ((_r)->rxr_alive)
#define if_rxr_cwm(_r)        ((_r)->rxr_cwm)

#endif /* _if_h */
