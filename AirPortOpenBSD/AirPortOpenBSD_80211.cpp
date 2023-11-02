//
//  AirPortOpenBSD_80211.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/7/17.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

bool AirPortOpenBSD::useAppleRSNSupplicant(void *interface)
{
    return this->useAppleRSN;
}

SInt32 AirPortOpenBSD::enableFeature(IO80211FeatureCode code, void *data)
{
    if (code == kIO80211Feature80211n) {
        return 0;
    }
    return 102;
}
