//
//  AirPortOpenBSD_Ethernet.cpp
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/8/28.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

void AirPortOpenBSD::autoASSOC(OSObject *owner, ...)
{
    _ifp->fCommandGate->runActionBlock(^IOReturn{
        
        struct device *dev = (struct device *)_ifp->if_softc;

        OSData* asd = OSDynamicCast(OSData, dev->dev->assoc_data_Arr->getObject(dev->dev->assoc_data_index));

        if (!asd) return kIOReturnSuccess;
        struct apple80211_assoc_data *ad = (apple80211_assoc_data*) asd->getBytesNoCopy();

        dev->dev->setASSOCIATE(_ifp->iface, ad);
        
        
    //    struct apple80211_assoc_data ad;
    //    bzero(&ad, sizeof(struct apple80211_assoc_data));
    //    bcopy("Starbucks", ad.ad_ssid, sizeof(ad.ad_ssid));
    //    ad.ad_ssid_len = strlen((char *)ad.ad_ssid);
    //    ad.ad_key.key_cipher_type = APPLE80211_CIPHER_NONE;
    //    controller->setASSOCIATE(_ifp->iface, &ad);
        
        
        struct apple80211_assoc_status_data ret;
        dev->dev->getASSOCIATION_STATUS(_ifp->iface, &ret);
        if (ret.status == APPLE80211_STATUS_UNSPECIFIED_FAILURE) {
            dev->dev->times++;
    //        if (controller->times > 2) {
    //            controller->times = 0;
    //            controller->assoc_data_index = (controller->assoc_data_index + 1) % controller->assoc_data_Arr->getCount();
    //        }

//            controller->scanEventSource->setAction(&AirPortOpenBSD::autoASSOC);
//            controller->scanEventSource->setTimeoutUS(1000000);
        }
        
        return kIOReturnSuccess;
    });
    
}
