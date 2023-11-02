//
//  AirPortOpenBSD_IOCTL.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include "AirPortOpenBSD.hpp"

//
// MARK: 12 - CARD_CAPABILITIES
//

IOReturn AirPortOpenBSD::getCARD_CAPABILITIES(OSObject *object, struct apple80211_capability_data *cd)
{
    uint32_t caps = this->ic->ic_caps;
    memset(cd, 0, sizeof(struct apple80211_capability_data));
    
    if (caps & IEEE80211_C_WEP)
        cd->capabilities[0] |= 1 << APPLE80211_CAP_WEP;
    if (caps & IEEE80211_C_RSN)
        cd->capabilities[0] |= 1 << APPLE80211_CAP_TKIP | 1 << APPLE80211_CAP_AES_CCM;
    // Disable not implemented capabilities
    // if (caps & IEEE80211_C_PMGT)
    //     cd->capabilities[0] |= 1 << APPLE80211_CAP_PMGT;
    // if (caps & IEEE80211_C_IBSS)
    //     cd->capabilities[0] |= 1 << APPLE80211_CAP_IBSS;
    // if (caps & IEEE80211_C_HOSTAP)
    //     cd->capabilities[0] |= 1 << APPLE80211_CAP_HOSTAP;
    // AES not enabled, like on Apple cards
    
    if (caps & IEEE80211_C_SHSLOT)
        cd->capabilities[1] |= 1 << (APPLE80211_CAP_SHSLOT - 8);
    if (caps & IEEE80211_C_SHPREAMBLE)
        cd->capabilities[1] |= 1 << (APPLE80211_CAP_SHPREAMBLE - 8);
    if (caps & IEEE80211_C_RSN)
        cd->capabilities[1] |= 1 << (APPLE80211_CAP_WPA1 - 8) | 1 << (APPLE80211_CAP_WPA2 - 8) | 1 << (APPLE80211_CAP_TKIPMIC - 8);
    // Disable not implemented capabilities
    // if (caps & IEEE80211_C_TXPMGT)
    //     cd->capabilities[1] |= 1 << (APPLE80211_CAP_TXPMGT - 8);
    // if (caps & IEEE80211_C_MONITOR)
    //     cd->capabilities[1] |= 1 << (APPLE80211_CAP_MONITOR - 8);
    // WPA not enabled, like on Apple cards

    cd->version = APPLE80211_VERSION;
    cd->capabilities[2] = 0xFF; // BURST, WME, SHORT_GI_40MHZ, SHORT_GI_20MHZ, WOW, TSN, ?, ?
    cd->capabilities[3] = 0x2B;
    cd->capabilities[5] = 0x40;
    cd->capabilities[6] = (
//                           1 |    //MFP capable
                           0x8 |
                           0x4 |
                           0x80
                           );
    *(uint16_t *)&cd->capabilities[8] = 0x201;
//
//    cd->capabilities[2] |= 0x10;
//    cd->capabilities[5] |= 0x1;
//
//    cd->capabilities[2] |= 0x2;
//
//    cd->capabilities[3] |= 0x20;
//
//    cd->capabilities[0] |= 0x80;
//
//    cd->capabilities[3] |= 0x80;
//    cd->capabilities[4] |= 0x4;
//
//    cd->capabilities[4] |= 0x1;
//    cd->capabilities[3] |= 0x1;
//    cd->capabilities[6] |= 0x8;
//
//    cd->capabilities[3] |= 3;
//    cd->capabilities[4] |= 2;
//    cd->capabilities[6] |= 0x10;
//    cd->capabilities[5] |= 0x20;
//    cd->capabilities[5] |= 0x80;
//
//    if (cd->capabilities[6] & 0x20) {
//        cd->capabilities[2] |= 8;
//    }
//    cd->capabilities[5] |= 8;
//    cd->capabilities[8] |= 2;
//
//    cd->capabilities[11] |= (2 | 4 | 8 | 0x10 | 0x20 | 0x40 | 0x80);
    
    return kIOReturnSuccess;
    
//    if (cd == NULL) {
//        return 0x16;
//    }
//    
//    int32_t rdx = 0;
//    int8_t rcx = 0;
//    int8_t rax = 0;
//    
//    memset(cd, 0, sizeof(*cd));
//    cd->version = APPLE80211_VERSION;
//
//    u_int32_t caps = 0;
//
//    caps |=  1 << APPLE80211_CAP_AES;
//    caps |=  1 << APPLE80211_CAP_AES_CCM;
//
//    if (this->ic->ic_caps & IEEE80211_C_WEP)              caps |= 1 << APPLE80211_CAP_WEP;
//    if (this->ic->ic_caps & IEEE80211_C_RSN) {
//        caps |=  1 << APPLE80211_CAP_TKIP;
//        caps |=  1 << APPLE80211_CAP_TKIPMIC;
//        caps |=  1 << APPLE80211_CAP_WPA;
//        caps |=  1 << APPLE80211_CAP_WPA1;
//        caps |=  1 << APPLE80211_CAP_WPA2;
//    }
//    if (this->ic->ic_caps & IEEE80211_C_MONITOR)          caps |= 1 << APPLE80211_CAP_MONITOR;
//    if (this->ic->ic_caps & IEEE80211_C_SHSLOT)           caps |= 1 << APPLE80211_CAP_SHSLOT;
//    if (this->ic->ic_caps & IEEE80211_C_SHPREAMBLE)       caps |= 1 << APPLE80211_CAP_SHPREAMBLE;
////    if (this->ic->ic_caps & IEEE80211_C_AHDEMO)           caps |= 1 << APPLE80211_CAP_IBSS;
//    if (this->ic->ic_caps & IEEE80211_C_PMGT)             caps |= 1 << APPLE80211_CAP_PMGT;
//    if (this->ic->ic_caps & IEEE80211_C_TXPMGT)           caps |= 1 << APPLE80211_CAP_TXPMGT;
////    if (this->ic->ic_caps & IEEE80211_C_QOS)              caps |= 1 << APPLE80211_CAP_WME;
//
//    cd->capabilities[0] = (caps & 0xff);
//    cd->capabilities[1] = (caps >> 8) & 0xff;
//    
//    cd->capabilities[0] = 0xeb;
//    cd->capabilities[1] = 0x7e;
//
//    rcx = cd->capabilities[2];
//
//    if (1) {
//        cd->capabilities[2] |= 0x13; // 无线网络唤醒;
//    } else {
//        cd->capabilities[2] |= 0x3;
//    }
//
//    rdx |= 0x1;
//    rdx |= 0x2;
//    rdx |= 0x4;
//    if ((rdx & 0x1) != 0x0) {
////        DebugLog("待确认功能1");
//        cd->capabilities[2] |= 0x28;
//    } else {
//        cd->capabilities[2] |= 0x20;
//    }
//
//    if ((rdx & 0x2) != 0x0) {
////        DebugLog("待确认功能2");
//        cd->capabilities[2] |= 0x4;
//    }
//
//    if ((rdx & 0x4) != 0x0) {
////        DebugLog("待确认功能4");
//        cd->capabilities[5] |= 0x8;    // 待确认功能
//    }
//
//
//    cd->capabilities[3] |= 0x2;
//    if (1) {
//        cd->capabilities[4] |= 0x1; // 隔空投送
//        cd->capabilities[6] |= 0x8;
//    }
//
//
//    if (0) {
//        cd->capabilities[8] |= 0x8;  // checkBoardId
//    }
//
//    cd->capabilities[3] |= 0x21;
//
//    rax = cd->capabilities[2];
//    cd->capabilities[2] = rax | 0x80;
//    if (0) {
//        cd->capabilities[5] |= 0x4; // 如果是0x4331，则支持0x4
//    }
//    if (this->scanReqMultiple) {
//        cd->capabilities[2] = rax | 0xc0; // 批量扫描;
//    }
//    cd->capabilities[6] |= 0x84;
//
////    if ((OSMetaClassBase::safeMetaCast(r15, **qword_55a050) != 0x0) && (*(int32_t *)(r14 + 0x2f44) >= 0x0)) {
//    if (1) {
//        cd->capabilities[3] |= 0x8;
//    }
////    if (*(*(r14 + 0xa10) + 0x928) != 0x0) {
//    if (1) {
////        DebugLog("待确认功能5");
//        cd->capabilities[4] |= 0xac;
//    }
////    DebugLog("待确认功能6");
//    cd->capabilities[6] |= 0x1;
//    if (1) {
//        cd->capabilities[7] |= 0x4; // 自动解锁
//    }
//
//    rax = cd->capabilities[8];
////    if (AirPort_BrcmNIC::isCntryDefaultSupported(r14) != 0x0) {
//    if (0) {
////        DebugLog("待确认功能7");
//        cd->capabilities[5] |= 0x80;
//    }
//    else {
////        DebugLog("待确认功能8");
//        rax = rax | 0x80;
//    }
//    cd->capabilities[7] |= 0x80;
//
//    cd->capabilities[8] = rax | 0x40;
//
////    rcx = cd->capabilities[9];
////    cd->capabilities[9] = rcx | 0x8;
////
//////    rdx = 0xaa52;
//////    if (rdx != 0xaa52) {
//////        if (rdx == 0x4350) {
//////            cd->capabilities[9] = rcx | 0x28;
//////        }
//////    }
//////    else {
//////        cd->capabilities[9] = rcx | 0x28; // 不发送数据，认证失败
//////    }
//    
//    return kIOReturnSuccess;
    
}


//
// MARK: 19 - POWER
//

IOReturn AirPortOpenBSD::getPOWER(OSObject *object, struct apple80211_power_data *ret)
{
    ret->version = APPLE80211_VERSION;
    ret->num_radios = 4;
    for (int i = 0; i < ret->num_radios; i++) {
        ret->power_state[i] = this->powerState;
    }

    return kIOReturnSuccess;
}


IOReturn AirPortOpenBSD::setPOWER(OSObject *object, struct apple80211_power_data *pd)
{
    IOReturn ret = kIOReturnSuccess;

    if (pd->num_radios > 0) {
        DebugLog("");
        ret = this->changePowerState(object, pd->power_state[0]);
//        this->getCommandGate()->runAction(setPowerStateAction, &pd->power_state[0]);
    }

    return ret;
}


//
// MARK: 43 - DRIVER_VERSION
//

IOReturn AirPortOpenBSD::getDRIVER_VERSION(OSObject *object, struct apple80211_version_data *hv)
{
    if (!hv)
        return kIOReturnError;
    struct ifnet *ifp = &this->ic->ic_if;
    
    hv->version = APPLE80211_VERSION;
    
    char fwname[256];
    snprintf(fwname, sizeof(fwname), "%s %s", PRODUCT_NAME, ifp->fwver);
    
    strncpy(hv->string, fwname, sizeof(hv->string));
    hv->string_len = strlen(fwname);
    return kIOReturnSuccess;
}

//
// MARK: 44 - HARDWARE_VERSION
//

IOReturn AirPortOpenBSD::getHARDWARE_VERSION(OSObject *object, struct apple80211_version_data *hv)
{
    if (!hv)
        return kIOReturnError;
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, "Hardware 1.0", sizeof(hv->string));
    hv->string_len = strlen("Hardware 1.0");
    return kIOReturnSuccess;
}

//
// MARK: 51 - COUNTRY_CODE
//

IOReturn AirPortOpenBSD::getCOUNTRY_CODE(OSObject *object, struct apple80211_country_code_data *ccd)
{
    if (this->ic->ic_state != IEEE80211_S_RUN || this->ic->ic_bss == NULL || this->ic->ic_bss->ni_countryie == NULL) {
        memcpy(ccd, &this->ccd, sizeof(*ccd));
        return kIOReturnSuccess;
    }
    
    ccd->version = APPLE80211_VERSION;
    bcopy(&this->ic->ic_bss->ni_countryie[2], ccd->cc, APPLE80211_MAX_CC_LEN);
    
    return kIOReturnSuccess;
}

IOReturn AirPortOpenBSD::setCOUNTRY_CODE(OSObject *object, struct apple80211_country_code_data *ccd)
{
    struct ifnet *ifp = &this->ic->ic_if;
    memcpy(&this->ccd, ccd, sizeof(*ccd));
    
    this->postMessage(APPLE80211_M_COUNTRY_CODE_CHANGED);
    
    return kIOReturnSuccess;
}
