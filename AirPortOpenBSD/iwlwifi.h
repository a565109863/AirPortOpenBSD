//
//  iwlwifi.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/4/16.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef iwlwifi_h
#define iwlwifi_h

#define DRIVER_COUNT 9

extern struct cfdriver iwx_cd;
extern struct cfdriver iwm_cd;
extern struct cfdriver iwn_cd;
extern struct cfdriver iwi_cd;
extern struct cfdriver wpi_cd;
extern struct cfdriver ipw_cd;
extern struct cfdriver rtwn_cd;

static struct cfdriverlist cdlist = {
    {iwx_cd, iwm_cd, iwn_cd, wpi_cd, iwi_cd, ipw_cd, rtwn_cd, NULL, NULL},
    DRIVER_COUNT
};


extern const struct cfattach iwm_ca;
extern const struct cfattach iwx_ca;
extern const struct cfattach iwn_ca;
extern const struct cfattach iwi_ca;
extern const struct cfattach wpi_ca;
extern const struct cfattach ipw_ca;
extern const struct cfattach bwfm_pci_ca;
extern const struct cfattach bwi_pci_ca;
extern const struct cfattach rtwn_pci_ca;

static struct cfattachlist calist = {
    {iwx_ca, iwm_ca, iwn_ca, wpi_ca, iwi_ca, ipw_ca, rtwn_pci_ca, bwfm_pci_ca, bwi_pci_ca},
    DRIVER_COUNT
};

#endif /* iwlwifi_h */
