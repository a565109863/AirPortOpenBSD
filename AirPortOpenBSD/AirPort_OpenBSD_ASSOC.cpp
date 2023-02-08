//
//  AirPort_OpenBSD_ASSOC.cpp
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2022/11/7.
//

#include "AirPort_OpenBSD.hpp"
#include <crypto/sha1.h>
#include <net80211/ieee80211_priv.h>
#include <net80211/ieee80211_var.h>

void AirPort_OpenBSD::setPTK(const u_int8_t *key, size_t key_len) {
    struct ieee80211_node *ni = this->ic->ic_bss;
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
        if ((*ic->ic_set_key)(this->ic, ni, k) != 0) {
            DebugLog("setting PTK failed\n");
            return;
        }
        else {
            DebugLog("setting PTK successfully\n");
        }
        ni->ni_flags &= ~IEEE80211_NODE_RSN_NEW_PTK;
        ni->ni_flags &= ~IEEE80211_NODE_TXRXPROT;
        ni->ni_flags |= IEEE80211_NODE_RXPROT;
    } else if (ni->ni_rsncipher != IEEE80211_CIPHER_USEGROUP)
        DebugLog("%s: unexpected pairwise key update received from %s\n",
              this->ic->ic_if.if_xname, ether_sprintf(ni->ni_macaddr));
}

void AirPort_OpenBSD::setGTK(const u_int8_t *gtk, size_t key_len, u_int8_t kid, u_int8_t *rsc) {
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
            if ((*ic->ic_set_key)(this->ic, ni, k) != 0) {
                DebugLog("setting GTK failed\n");
                return;
            }
            else {
                DebugLog("setting GTK successfully\n");
            }
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
            ieee80211_set_link_state(this->ic, LINK_STATE_UP);
            ni->ni_assoc_fail = 0;
            if (this->ic->ic_opmode == IEEE80211_M_STA)
                this->ic->ic_rsngroupcipher = ni->ni_rsngroupcipher;
        }
    }
}

