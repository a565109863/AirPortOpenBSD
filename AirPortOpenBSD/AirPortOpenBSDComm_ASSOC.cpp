//
//  AirPortOpenBSDComm_ASSOC.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/7.
//

#include <Availability.h>

#include "apple80211.h"

#if MAC_VERSION_MAJOR >= MAC_VERSION_MAJOR_Sonoma
#include "AirPortOpenBSD.hpp"
#else
#include "AirPortOpenBSDLegacy.hpp"
#endif

#include <crypto/sha1.h>
#include <net80211/ieee80211_priv.h>
#include <net80211/ieee80211_var.h>

void AirPortOpenBSD::setPTK(const u_int8_t *key, size_t key_len)
{
    DebugLog("");
    struct ieee80211_node *ni = ic->ic_bss;
    struct ieee80211_key *k;
    int keylen;
    
    ni->ni_rsn_supp_state = RSNA_SUPP_PTKDONE;
    
    if (ni->ni_rsncipher != IEEE80211_CIPHER_USEGROUP) {
        u_int64_t prsc;
        
        /* check that key length matches that of pairwise cipher */
        keylen = ieee80211_cipher_keylen(ni->ni_rsncipher);
        if (key_len != keylen) {
            DebugLog("PTK length mismatch. expected %d, got %zu\n", keylen, key_len);
            return;
        }
        prsc = /*(gtk == NULL) ? LE_READ_6(key->rsc) :*/ 0;
        
        /* map PTK to 802.11 key */
        k = &ni->ni_pairwise_key;
        memset(k, 0, sizeof(*k));
        k->k_cipher = ni->ni_rsncipher;
        k->k_rsc[0] = prsc;
        k->k_len = keylen;
        memcpy(k->k_key, key, k->k_len);
        /* install the PTK */
        switch ((*ic->ic_set_key)(ic, ni, k)) {
        case 0:
            break;
        case EBUSY:
            break;
        default:
            DebugLog("setting PTK failed\n");
            return;
        }
        DebugLog("setting PTK successfully\n");
        ni->ni_flags &= ~IEEE80211_NODE_RSN_NEW_PTK;
        ni->ni_flags &= ~IEEE80211_NODE_TXRXPROT;
        ni->ni_flags |= IEEE80211_NODE_RXPROT;
    } else if (ni->ni_rsncipher != IEEE80211_CIPHER_USEGROUP)
        DebugLog("%s: unexpected pairwise key update received from %s\n",
              ic->ic_if.if_xname, ether_sprintf(ni->ni_macaddr));
}

void AirPortOpenBSD::setGTK(const u_int8_t *gtk, size_t key_len, u_int8_t kid, u_int8_t *rsc)
{
    DebugLog("");
    struct ieee80211_node *ni = this->ic->ic_bss;
    struct ieee80211_key *k;
    int keylen;
    
    if (gtk != NULL) {
        /* check that key length matches that of group cipher */
        keylen = ieee80211_cipher_keylen(ni->ni_rsngroupcipher);
        if (key_len != keylen) {
            DebugLog("GTK length mismatch. expected %d, got %zu\n", keylen, key_len);
            return;
        }
        /* map GTK to 802.11 key */
        k = &this->ic->ic_nw_keys[kid];
        if (k->k_cipher == IEEE80211_CIPHER_NONE || k->k_len != keylen || memcmp(k->k_key, gtk, keylen) != 0) {
            memset(k, 0, sizeof(*k));
            k->k_id = kid;    /* 0-3 */
            k->k_cipher = ni->ni_rsngroupcipher;
            k->k_flags = IEEE80211_KEY_GROUP;
            //if (gtk[6] & (1 << 2))
            //  k->k_flags |= IEEE80211_KEY_TX;
            k->k_rsc[0] = LE_READ_6(rsc);
            k->k_len = keylen;
            memcpy(k->k_key, gtk, k->k_len);
            /* install the GTK */
            switch ((*ic->ic_set_key)(ic, ni, k)) {
            case 0:
                break;
            case EBUSY:
                break;
            default:
                DebugLog("setting GTK failed\n");
                return;
            }
            DebugLog("setting GTK successfully\n");
        }
    }
    
    if (true) {
        ni->ni_flags |= IEEE80211_NODE_TXRXPROT;
#ifndef IEEE80211_STA_ONLY
        if (this->ic->ic_opmode != IEEE80211_M_IBSS ||
            ++ni->ni_key_count == 2)
#endif
        {
            DebugLog("marking port %s valid\n",
                  ether_sprintf(ni->ni_macaddr));
            ni->ni_port_valid = 1;
            DebugLog("");
            ieee80211_set_link_state(this->ic, LINK_STATE_UP);
            ni->ni_assoc_fail = 0;
            if (this->ic->ic_opmode == IEEE80211_M_STA)
                this->ic->ic_rsngroupcipher = ni->ni_rsngroupcipher;
        }
    }
}

bool AirPortOpenBSD::isConnected()
{
    return ic->ic_state == IEEE80211_S_RUN &&
    (ic->ic_opmode != IEEE80211_M_STA ||
     !(ic->ic_flags & IEEE80211_F_RSNON) ||
     ic->ic_bss->ni_port_valid);
}

bool AirPortOpenBSD::isRun80211X()
{
    return ic->ic_state == IEEE80211_S_RUN &&
    (ic->ic_opmode != IEEE80211_M_STA || (ic->ic_rsnakms & IEEE80211_AKM_8021X || ic->ic_rsnakms & IEEE80211_AKM_SHA256_8021X));
}
