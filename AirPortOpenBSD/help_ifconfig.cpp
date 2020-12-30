//
//  help_ifconfig.cpp
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//


#include "help_ifconfig.h"


extern ifnet *_ifp;

/*
 * HMAC-SHA-1 (from RFC 2202).
 */
void hmac_sha1(const u_int8_t *, size_t, const u_int8_t *,
               size_t, u_int8_t []);


/*
 * HMAC-SHA-1 (from RFC 2202).
 */
void
hmac_sha1(const u_int8_t *text, size_t text_len, const u_int8_t *key,
          size_t key_len, u_int8_t digest[SHA1_DIGEST_LENGTH])
{
    SHA1_CTX ctx;
    u_int8_t k_pad[SHA1_BLOCK_LENGTH];
    u_int8_t tk[SHA1_DIGEST_LENGTH];
    int i;
    
    if (key_len > SHA1_BLOCK_LENGTH) {
        SHA1Init(&ctx);
        SHA1Update(&ctx, key, key_len);
        SHA1Final(tk, &ctx);
        
        key = tk;
        key_len = SHA1_DIGEST_LENGTH;
    }
    
    bzero(k_pad, sizeof k_pad);
    bcopy(key, k_pad, key_len);
    for (i = 0; i < SHA1_BLOCK_LENGTH; i++)
        k_pad[i] ^= 0x36;
    
    SHA1Init(&ctx);
    SHA1Update(&ctx, k_pad, SHA1_BLOCK_LENGTH);
    SHA1Update(&ctx, text, text_len);
    SHA1Final(digest, &ctx);
    
    bzero(k_pad, sizeof k_pad);
    bcopy(key, k_pad, key_len);
    for (i = 0; i < SHA1_BLOCK_LENGTH; i++)
        k_pad[i] ^= 0x5c;
    
    SHA1Init(&ctx);
    SHA1Update(&ctx, k_pad, SHA1_BLOCK_LENGTH);
    SHA1Update(&ctx, digest, SHA1_DIGEST_LENGTH);
    SHA1Final(digest, &ctx);
}


/// ------- start --------

#define MINIMUM(a, b)    (((a) < (b)) ? (a) : (b))

/*
 * Password-Based Key Derivation Function 2 (PKCS #5 v2.0).
 * Code based on IEEE Std 802.11-2007, Annex H.4.2.
 */
int
pkcs5_pbkdf2(const char *pass, size_t pass_len, const uint8_t *salt,
             size_t salt_len, uint8_t *key, size_t key_len, unsigned int rounds)
{
    uint8_t *asalt, obuf[SHA1_DIGEST_LENGTH];
    uint8_t d1[SHA1_DIGEST_LENGTH], d2[SHA1_DIGEST_LENGTH];
    unsigned int i, j;
    unsigned int count;
    size_t r;

    if (rounds < 1 || key_len == 0)
        return -1;
    if (salt_len == 0 || salt_len > SIZE_MAX - 4)
        return -1;
    if ((asalt = (uint8_t *)malloc(salt_len + 4, M_DEVBUF, M_NOWAIT)) == NULL)
        return -1;

    memcpy(asalt, salt, salt_len);

    for (count = 1; key_len > 0; count++) {
        asalt[salt_len + 0] = (count >> 24) & 0xff;
        asalt[salt_len + 1] = (count >> 16) & 0xff;
        asalt[salt_len + 2] = (count >> 8) & 0xff;
        asalt[salt_len + 3] = count & 0xff;
        hmac_sha1(asalt, salt_len + 4, (const u_int8_t *)pass, pass_len, d1);
        memcpy(obuf, d1, sizeof(obuf));

        for (i = 1; i < rounds; i++) {
            hmac_sha1(d1, sizeof(d1), (const u_int8_t *)pass, pass_len, d2);
            memcpy(d1, d2, sizeof(d1));
            for (j = 0; j < sizeof(obuf); j++)
                obuf[j] ^= d1[j];
        }

        r = MINIMUM(key_len, SHA1_DIGEST_LENGTH);
        memcpy(key, obuf, r);
        key += r;
        key_len -= r;
    };
    
    free(asalt, M_DEVBUF, salt_len + 4);
    explicit_bzero(d1, sizeof(d1));
    explicit_bzero(d2, sizeof(d2));
    explicit_bzero(obuf, sizeof(obuf));

    return 0;
}

/// Returns the corresponding lowercase character if \p x is uppercase.
inline char toLower(char x) {
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 'a';
    return x;
}

/// Returns the corresponding uppercase character if \p x is lowercase.
inline char toUpper(char x) {
    if (x >= 'a' && x <= 'z')
        return x - 'a' + 'A';
    return x;
}

inline bool isDigit(char C) { return C >= '0' && C <= '9'; }

#  define tolower(c) toLower(c)
#  define toupper(c) toUpper(c)
#  define isdigit(c) isDigit(c)

#  define isxdigit(c)   (isdigit((c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))

const char *get_string(const char *, const char *, u_int8_t *, int *);

const char *
get_string(const char *val, const char *sep, u_int8_t *buf, int *lenp)
{
    int len = *lenp, hexstr;
    u_int8_t *p = buf;
    
    hexstr = (val[0] == '0' && tolower((u_char)val[1]) == 'x');
    if (hexstr)
        val += 2;
    for (;;) {
        if (*val == '\0')
            break;
        if (sep != NULL && strchr(sep, *val) != NULL) {
            val++;
            break;
        }
        if (hexstr) {
            if (!isxdigit((u_char)val[0]) ||
                !isxdigit((u_char)val[1])) {
                warnx("bad hexadecimal digits");
                return NULL;
            }
        }
        if (p > buf + len) {
            if (hexstr)
                warnx("hexadecimal digits too long");
            else
                warnx("strings too long");
            return NULL;
        }
        if (hexstr) {
#define    tohex(x)    (isdigit(x) ? (x) - '0' : tolower(x) - 'a' + 10)
            *p++ = (tohex((u_char)val[0]) << 4) |
            tohex((u_char)val[1]);
#undef tohex
            val += 2;
        } else {
            if (*val == '\\' &&
                sep != NULL && strchr(sep, *(val + 1)) != NULL)
                val++;
            *p++ = *val++;
        }
    }
    len = p - buf;
    if (len < *lenp)
        memset(p, 0, *lenp - len);
    *lenp = len;
    return val;
}

int    shownet80211chans = 0;
int    shownet80211nodes = 0;

struct    ifreq        ifr = {}, ridreq;

char    ifname[IFNAMSIZ];
int    flags = 0, xflags, setaddr, setipdst, doalias;


struct ieee80211_join join = {};

char joinname[IEEE80211_NWID_LEN];
char nwidname[IEEE80211_NWID_LEN];

/*
 * Media stuff.  Whenever a media command is first performed, the
 * currently select media is grabbed for this interface.  If `media'
 * is given, the current media word is modified.  `mediaopt' commands
 * only modify the set and clear words.  They then operate on the
 * current media word later.
 */
uint64_t    media_current;
uint64_t    mediaopt_set;
uint64_t    mediaopt_clear;

int    actions;            /* Actions performed */

#define    A_MEDIA        0x0001        /* media command */
#define    A_MEDIAOPTSET    0x0002        /* mediaopt command */
#define    A_MEDIAOPTCLR    0x0004        /* -mediaopt command */
#define    A_MEDIAOPT    (A_MEDIAOPTSET|A_MEDIAOPTCLR)
#define    A_MEDIAINST    0x0008        /* instance or inst command */
#define    A_MEDIAMODE    0x0010        /* mode command */
#define    A_JOIN        0x0020        /* join */
#define A_SILENT    0x8000000    /* doing operation, do not print */

#define    NEXTARG0    0xffffff
#define NEXTARG        0xfffffe
#define    NEXTARG2    0xfffffd

const struct    cmd {
    char    *c_name;
    int    c_parameter;        /* NEXTARG means next argv */
    int    c_action;        /* defered action */
    void    (*c_func)(const char *val, int d);
    void    (*c_func2)(const char *, const char *);
} cmds[] = {
    { "up",        IFF_UP,        0,        setifflags } ,
    { "down",    -IFF_UP,    0,        setifflags },
//    { "arp",    -IFF_NOARP,    0,        setifflags },
//    { "-arp",    IFF_NOARP,    0,        setifflags },
    { "debug",    IFF_DEBUG,    0,        setifflags },
    { "-debug",    -IFF_DEBUG,    0,        setifflags },
//    { "alias",    IFF_UP,        0,        notealias },
//    { "-alias",    -IFF_UP,    0,        notealias },
//    { "delete",    -IFF_UP,    0,        notealias },
#ifdef notdef
#define    EN_SWABIPS    0x1000
    { "swabips",    EN_SWABIPS,    0,        setifflags },
    { "-swabips",    -EN_SWABIPS,    0,        setifflags },
#endif /* notdef */
//    { "netmask",    NEXTARG,    0,        setifnetmask },
//    { "mtu",    NEXTARG,    0,        setifmtu },
    { "nwid",    NEXTARG,    0,        setifnwid },
    { "-nwid",    -1,        0,        setifnwid },
    { "join",    NEXTARG,    A_JOIN,        setifjoin },
    { "-join",    NEXTARG,    0,        delifjoin },
    { "joinlist",    NEXTARG0,    0,        showjoin },
//    { "-joinlist",    -1,        0,        delifjoinlist },
    { "bssid",    NEXTARG,    0,        setifbssid },
    { "-bssid",    -1,        0,        setifbssid },
    { "nwkey",    NEXTARG,    0,        setifnwkey },
    { "-nwkey",    -1,        0,        setifnwkey },
    { "wpa",    1,        0,        setifwpa },
    { "-wpa",    0,        0,        setifwpa },
    { "wpaakms",    NEXTARG,    0,        setifwpaakms },
    { "wpaciphers",    NEXTARG,    0,        setifwpaciphers },
    { "wpagroupcipher", NEXTARG,    0,        setifwpagroupcipher },
    { "wpaprotos",    NEXTARG,    0,        setifwpaprotos },
    { "wpakey",    NEXTARG,    0,        setifwpakey },
    { "-wpakey",    -1,        0,        setifwpakey },
    { "chan",    NEXTARG0,    0,        setifchan },
    { "-chan",    -1,        0,        setifchan },
//    { "scan",    NEXTARG0,    0,        setifscan },
//    { "broadcast",    NEXTARG,    0,        setifbroadaddr },
//    { "prefixlen",  NEXTARG,    0,        setifprefixlen},
//    { "vnetid",    NEXTARG,    0,        setvnetid },
//    { "-vnetid",    0,        0,        delvnetid },
//    { "parent",    NEXTARG,    0,        setifparent },
//    { "-parent",    1,        0,        delifparent },
//    { "vlan",    NEXTARG,    0,        setvnetid },
//    { "-vlan",    0,        0,        delvnetid },
//    { "vlandev",    NEXTARG,    0,        setifparent },
//    { "-vlandev",    1,        0,        delifparent },
//    { "group",    NEXTARG,    0,        setifgroup },
//    { "-group",    NEXTARG,    0,        unsetifgroup },
//    { "autoconf",    1,        0,        setautoconf },
//    { "-autoconf",    -1,        0,        setautoconf },
//    { "trunkport",    NEXTARG,    0,        settrunkport },
//    { "-trunkport",    NEXTARG,    0,        unsettrunkport },
//    { "trunkproto",    NEXTARG,    0,        settrunkproto },
//    { "lacpmode",    NEXTARG,    0,        settrunklacpmode },
//    { "lacptimeout", NEXTARG,    0,        settrunklacptimeout },
//    { "anycast",    IN6_IFF_ANYCAST,    0,    setia6flags },
//    { "-anycast",    -IN6_IFF_ANYCAST,    0,    setia6flags },
//    { "tentative",    IN6_IFF_TENTATIVE,    0,    setia6flags },
//    { "-tentative",    -IN6_IFF_TENTATIVE,    0,    setia6flags },
//    { "pltime",    NEXTARG,    0,        setia6pltime },
//    { "vltime",    NEXTARG,    0,        setia6vltime },
//    { "eui64",    0,        0,        setia6eui64 },
//    { "autoconfprivacy",    -IFXF_INET6_NOPRIVACY,    0,    setifxflags },
//    { "-autoconfprivacy",    IFXF_INET6_NOPRIVACY,    0,    setifxflags },
//    { "soii",    -IFXF_INET6_NOSOII,    0,    setifxflags },
//    { "-soii",    IFXF_INET6_NOSOII,    0,    setifxflags },
#ifndef SMALL
//    { "hwfeatures", NEXTARG0,    0,        printifhwfeatures },
//    { "metric",    NEXTARG,    0,        setifmetric },
//    { "powersave",    NEXTARG0,    0,        setifpowersave },
//    { "-powersave",    -1,        0,        setifpowersave },
//    { "priority",    NEXTARG,    0,        setifpriority },
//    { "rtlabel",    NEXTARG,    0,        setifrtlabel },
//    { "-rtlabel",    -1,        0,        setifrtlabel },
//    { "rdomain",    NEXTARG,    0,        setrdomain },
//    { "-rdomain",    0,        0,        unsetrdomain },
//    { "staticarp",    IFF_STATICARP,    0,        setifflags },
//    { "-staticarp",    -IFF_STATICARP,    0,        setifflags },
//    { "mpls",    IFXF_MPLS,    0,        setifxflags },
//    { "-mpls",    -IFXF_MPLS,    0,        setifxflags },
//    { "mplslabel",    NEXTARG,    0,        setmplslabel },
//    { "-mplslabel",    0,        0,        unsetmplslabel },
//    { "pwecw",    0,        0,        setpwe3cw },
//    { "-pwecw",    0,        0,        unsetpwe3cw },
//    { "pwefat",    0,        0,        setpwe3fat },
//    { "-pwefat",    0,        0,        unsetpwe3fat },
//    { "pweneighbor", NEXTARG2,    0,        NULL, setpwe3neighbor },
//    { "-pweneighbor", 0,        0,        unsetpwe3neighbor },
//    { "advbase",    NEXTARG,    0,        setcarp_advbase },
//    { "advskew",    NEXTARG,    0,        setcarp_advskew },
//    { "carppeer",    NEXTARG,    0,        setcarppeer },
//    { "-carppeer",    1,        0,        unsetcarppeer },
//    { "pass",    NEXTARG,    0,        setcarp_passwd },
//    { "vhid",    NEXTARG,    0,        setcarp_vhid },
//    { "state",    NEXTARG,    0,        setcarp_state },
//    { "carpdev",    NEXTARG,    0,        setcarpdev },
//    { "carpnodes",    NEXTARG,    0,        setcarp_nodes },
//    { "balancing",    NEXTARG,    0,        setcarp_balancing },
//    { "syncdev",    NEXTARG,    0,        setpfsync_syncdev },
//    { "-syncdev",    1,        0,        unsetpfsync_syncdev },
//    { "syncif",    NEXTARG,    0,        setpfsync_syncdev },
//    { "-syncif",    1,        0,        unsetpfsync_syncdev },
//    { "syncpeer",    NEXTARG,    0,        setpfsync_syncpeer },
//    { "-syncpeer",    1,        0,        unsetpfsync_syncpeer },
//    { "maxupd",    NEXTARG,    0,        setpfsync_maxupd },
//    { "defer",    1,        0,        setpfsync_defer },
//    { "-defer",    0,        0,        setpfsync_defer },
//    { "tunnel",    NEXTARG2,    0,        NULL, settunnel },
//    { "tunneladdr",    NEXTARG,    0,        settunneladdr },
//    { "-tunnel",    0,        0,        deletetunnel },
    /* deletetunnel is for backward compat, remove during 6.4-current */
//    { "deletetunnel",  0,        0,        deletetunnel },
//    { "tunneldomain", NEXTARG,    0,        settunnelinst },
//    { "-tunneldomain", 0,        0,        unsettunnelinst },
//    { "tunnelttl",    NEXTARG,    0,        settunnelttl },
//    { "tunneldf",    0,        0,        settunneldf },
//    { "-tunneldf",    0,        0,        settunnelnodf },
//    { "tunnelecn",    0,        0,        settunnelecn },
//    { "-tunnelecn",    0,        0,        settunnelnoecn },
//    { "vnetflowid",    0,        0,        setvnetflowid },
//    { "-vnetflowid", 0,        0,        delvnetflowid },
//    { "txprio",    NEXTARG,    0,        settxprio },
//    { "rxprio",    NEXTARG,    0,        setrxprio },
//    { "pppoedev",    NEXTARG,    0,        setpppoe_dev },
//    { "pppoesvc",    NEXTARG,    0,        setpppoe_svc },
//    { "-pppoesvc",    1,        0,        setpppoe_svc },
//    { "pppoeac",    NEXTARG,    0,        setpppoe_ac },
//    { "-pppoeac",    1,        0,        setpppoe_ac },
//    { "authproto",    NEXTARG,    0,        setspppproto },
//    { "authname",    NEXTARG,    0,        setspppname },
//    { "authkey",    NEXTARG,    0,        setspppkey },
//    { "peerproto",    NEXTARG,    0,        setsppppeerproto },
//    { "peername",    NEXTARG,    0,        setsppppeername },
//    { "peerkey",    NEXTARG,    0,        setsppppeerkey },
//    { "peerflag",    NEXTARG,    0,        setsppppeerflag },
//    { "-peerflag",    NEXTARG,    0,        unsetsppppeerflag },
//    { "nwflag",    NEXTARG,    0,        setifnwflag },
//    { "-nwflag",    NEXTARG,    0,        unsetifnwflag },
//    { "flowsrc",    NEXTARG,    0,        setpflow_sender },
//    { "-flowsrc",    1,        0,        unsetpflow_sender },
//    { "flowdst",    NEXTARG,    0,        setpflow_receiver },
//    { "-flowdst", 1,        0,        unsetpflow_receiver },
//    { "pflowproto", NEXTARG,    0,        setpflowproto },
//    { "-inet",    AF_INET,    0,        removeaf },
//    { "-inet6",    AF_INET6,    0,        removeaf },
//    { "keepalive",    NEXTARG2,    0,        NULL, setkeepalive },
//    { "-keepalive",    1,        0,        unsetkeepalive },
//    { "add",    NEXTARG,    0,        bridge_add },
//    { "del",    NEXTARG,    0,        bridge_delete },
//    { "addspan",    NEXTARG,    0,        bridge_addspan },
//    { "delspan",    NEXTARG,    0,        bridge_delspan },
//    { "discover",    NEXTARG,    0,        setdiscover },
//    { "-discover",    NEXTARG,    0,        unsetdiscover },
//    { "blocknonip", NEXTARG,    0,        setblocknonip },
//    { "-blocknonip",NEXTARG,    0,        unsetblocknonip },
//    { "learn",    NEXTARG,    0,        setlearn },
//    { "-learn",    NEXTARG,    0,        unsetlearn },
//    { "stp",    NEXTARG,    0,        setstp },
//    { "-stp",    NEXTARG,    0,        unsetstp },
//    { "edge",    NEXTARG,    0,        setedge },
//    { "-edge",    NEXTARG,    0,        unsetedge },
//    { "autoedge",    NEXTARG,    0,        setautoedge },
//    { "-autoedge",    NEXTARG,    0,        unsetautoedge },
//    { "protected",    NEXTARG2,    0,        NULL, bridge_protect },
//    { "-protected",    NEXTARG,    0,        bridge_unprotect },
//    { "ptp",    NEXTARG,    0,        setptp },
//    { "-ptp",    NEXTARG,    0,        unsetptp },
//    { "autoptp",    NEXTARG,    0,        setautoptp },
//    { "-autoptp",    NEXTARG,    0,        unsetautoptp },
//    { "flush",    0,        0,        bridge_flush },
//    { "flushall",    0,        0,        bridge_flushall },
//    { "static",    NEXTARG2,    0,        NULL, bridge_addaddr },
//    { "deladdr",    NEXTARG,    0,        bridge_deladdr },
//    { "maxaddr",    NEXTARG,    0,        bridge_maxaddr },
//    { "addr",    0,        0,        bridge_addrs },
//    { "hellotime",    NEXTARG,    0,        bridge_hellotime },
//    { "fwddelay",    NEXTARG,    0,        bridge_fwddelay },
//    { "maxage",    NEXTARG,    0,        bridge_maxage },
//    { "proto",    NEXTARG,    0,        bridge_proto },
//    { "ifpriority",    NEXTARG2,    0,        NULL, bridge_ifprio },
//    { "ifcost",    NEXTARG2,    0,        NULL, bridge_ifcost },
//    { "-ifcost",    NEXTARG,    0,        bridge_noifcost },
//    { "timeout",    NEXTARG,    0,        bridge_timeout },
//    { "holdcnt",    NEXTARG,    0,        bridge_holdcnt },
//    { "spanpriority", NEXTARG,    0,        bridge_priority },
//    { "ipdst",    NEXTARG,    0,        setifipdst },
#if 0
    /* XXX `rule` special-cased below */
    { "rule",    0,        0,        bridge_rule },
#endif
//    { "rules",    NEXTARG,    0,        bridge_rules },
//    { "rulefile",    NEXTARG,    0,        bridge_rulefile },
//    { "flushrule",    NEXTARG,    0,        bridge_flushrule },
//    { "description", NEXTARG,    0,        setifdesc },
//    { "descr",    NEXTARG,    0,        setifdesc },
//    { "-description", 1,        0,        unsetifdesc },
//    { "-descr",    1,        0,        unsetifdesc },
//    { "wol",    IFXF_WOL,    0,        setifxflags },
//    { "-wol",    -IFXF_WOL,    0,        setifxflags },
//    { "pin",    NEXTARG,    0,        umb_setpin },
//    { "chgpin",    NEXTARG2,    0,        NULL, umb_chgpin },
//    { "puk",    NEXTARG2,    0,        NULL, umb_puk },
//    { "apn",    NEXTARG,    0,        umb_apn },
//    { "-apn",    -1,        0,        umb_apn },
//    { "class",    NEXTARG0,    0,        umb_setclass },
//    { "-class",    -1,        0,        umb_setclass },
//    { "roaming",    1,        0,        umb_roaming },
//    { "-roaming",    0,        0,        umb_roaming },
//    { "patch",    NEXTARG,    0,        setpair },
//    { "-patch",    1,        0,        unsetpair },
//    { "datapath",    NEXTARG,    0,        switch_datapathid },
//    { "portno",    NEXTARG2,    0,        NULL, switch_portno },
//    { "addlocal",    NEXTARG,    0,        addlocal },
//    { "transceiver", NEXTARG0,    0,        transceiver },
//    { "sff",    NEXTARG0,    0,        transceiver },
//    { "sffdump",    0,        0,        transceiverdump },
#else /* SMALL */
    { "powersave",    NEXTARG0,    0,        setignore },
    { "priority",    NEXTARG,    0,        setignore },
    { "rtlabel",    NEXTARG,    0,        setignore },
    { "mpls",    IFXF_MPLS,    0,        setignore },
    { "nwflag",    NEXTARG,    0,        setignore },
    { "rdomain",    NEXTARG,    0,        setignore },
    { "-inet",    AF_INET,    0,        removeaf },
    { "-inet6",    AF_INET6,    0,        removeaf },
    { "description", NEXTARG,    0,        setignore },
    { "descr",    NEXTARG,    0,        setignore },
    { "wol",    IFXF_WOL,    0,        setignore },
    { "-wol",    -IFXF_WOL,    0,        setignore },
#endif /* SMALL */
#if 0
    /* XXX `create' special-cased below */
    { "create",    0,        0,        clone_create } ,
#endif
//    { "destroy",    0,        0,        clone_destroy } ,
//    { "link0",    IFF_LINK0,    0,        setifflags } ,
//    { "-link0",    -IFF_LINK0,    0,        setifflags } ,
//    { "link1",    IFF_LINK1,    0,        setifflags } ,
//    { "-link1",    -IFF_LINK1,    0,        setifflags } ,
//    { "link2",    IFF_LINK2,    0,        setifflags } ,
//    { "-link2",    -IFF_LINK2,    0,        setifflags } ,
//    { "media",    NEXTARG0,    A_MEDIA,    setmedia },
//    { "mediaopt",    NEXTARG,    A_MEDIAOPTSET,    setmediaopt },
//    { "-mediaopt",    NEXTARG,    A_MEDIAOPTCLR,    unsetmediaopt },
//    { "mode",    NEXTARG,    A_MEDIAMODE,    setmediamode },
//    { "-mode",    0,        A_MEDIAMODE,    unsetmediamode },
//    { "instance",    NEXTARG,    A_MEDIAINST,    setmediainst },
//    { "inst",    NEXTARG,    A_MEDIAINST,    setmediainst },
//    { "lladdr",    NEXTARG,    0,        setiflladdr },
//    { "llprio",    NEXTARG,    0,        setifllprio },
//    { NULL, /*src*/    0,        0,        setifaddr },
//    { NULL, /*dst*/    0,        0,        setifdstaddr },
    { NULL, /*illegal*/0,        0,        NULL },
};


char *strtok(char *s, const char *delim);
char *strdup(const char *s);

void split(const char *src, const char *separator,char **dest, int *num)
{
    char *optlist, *str;
    u_int i = 0;
    
    if ((optlist = strdup(src)) == NULL)
        errx(1, "strdup");
    str = strtok(optlist, separator);
    while (str != NULL) {
        *dest++ = str;
        i++;
        str = strtok(NULL, separator);
    }
    *num = i;
}


void ifconfig(const char **argv, int argc)
{
    while (argc > 0) {
        const struct cmd *p;
        
        for (p = cmds; p->c_name; p++)
            if (strcmp(*argv, p->c_name) == 0)
                break;
//#ifndef SMALL
//        if (strcmp(*argv, "rule") == 0) {
//            argc--, argv++;
//            return bridge_rule(argc, argv, -1);
//        }
//#endif
//        if (p->c_name == 0 && setaddr)
//            for (i = setaddr; i > 0; i--) {
//                p++;
//                if (p->c_func == NULL)
//                    errx(1, "%s: bad value", *argv);
//            }
        if (p->c_func || p->c_func2) {
            if (p->c_parameter == NEXTARG0) {
                const struct cmd *p0;
                int noarg = 1;
                
                if (argv[1]) {
                    for (p0 = cmds; p0->c_name; p0++)
                        if (strcmp(argv[1],
                                   p0->c_name) == 0) {
                            noarg = 0;
                            break;
                        }
                } else
                    noarg = 0;
                
                if (noarg == 0)
                    (*p->c_func)(NULL, 0);
                else
                    goto nextarg;
            } else if (p->c_parameter == NEXTARG) {
            nextarg:
                if (argv[1] == NULL)
                    errx(1, "'%s' requires argument",
                         p->c_name);
                (*p->c_func)(argv[1], 0);
                static_cast<void>(argc--), argv++;
                actions = actions | A_SILENT | p->c_action;
            } else if (p->c_parameter == NEXTARG2) {
                if ((argv[1] == NULL) ||
                    (argv[2] == NULL))
                    errx(1, "'%s' requires 2 arguments",
                         p->c_name);
                (*p->c_func2)(argv[1], argv[2]);
                argc -= 2;
                argv += 2;
                actions = actions | A_SILENT | p->c_action;
            } else {
                (*p->c_func)(*argv, p->c_parameter);
                actions = actions | A_SILENT | p->c_action;
            }
        }
        static_cast<void>(argc--), argv++;
    }
    
    process_join_commands();
}

void ifconfig(const char *config_str)
{
    struct device *dev = (struct device *)_ifp->if_softc;
    strlcpy(ifname, dev->dev->getName(), sizeof(ifname));
    
    char *config[100] = {0};
    
    int argc = 0;
    
    split(config_str, " ", config, &argc);
    const char **argv = (const char **)config ;
    
    while (argc > 0) {
        const struct cmd *p;
        
        for (p = cmds; p->c_name; p++)
            if (strcmp(*argv, p->c_name) == 0)
                break;
//#ifndef SMALL
//        if (strcmp(*argv, "rule") == 0) {
//            argc--, argv++;
//            return bridge_rule(argc, argv, -1);
//        }
//#endif
//        if (p->c_name == 0 && setaddr)
//            for (i = setaddr; i > 0; i--) {
//                p++;
//                if (p->c_func == NULL)
//                    errx(1, "%s: bad value", *argv);
//            }
        if (p->c_func || p->c_func2) {
            if (p->c_parameter == NEXTARG0) {
                const struct cmd *p0;
                int noarg = 1;
                
                if (argv[1]) {
                    for (p0 = cmds; p0->c_name; p0++)
                        if (strcmp(argv[1],
                                   p0->c_name) == 0) {
                            noarg = 0;
                            break;
                        }
                } else
                    noarg = 0;
                
                if (noarg == 0)
                    (*p->c_func)(NULL, 0);
                else
                    goto nextarg;
            } else if (p->c_parameter == NEXTARG) {
            nextarg:
                if (argv[1] == NULL)
                    errx(1, "'%s' requires argument",
                         p->c_name);
                (*p->c_func)(argv[1], 0);
                static_cast<void>(argc--), argv++;
                actions = actions | A_SILENT | p->c_action;
            } else if (p->c_parameter == NEXTARG2) {
                if ((argv[1] == NULL) ||
                    (argv[2] == NULL))
                    errx(1, "'%s' requires 2 arguments",
                         p->c_name);
                (*p->c_func2)(argv[1], argv[2]);
                argc -= 2;
                argv += 2;
                actions = actions | A_SILENT | p->c_action;
            } else {
                (*p->c_func)(*argv, p->c_parameter);
                actions = actions | A_SILENT | p->c_action;
            }
        }
        static_cast<void>(argc--), argv++;
    }
    
    process_join_commands();
}


/*
 * Note: doing an SIOCGIFFLAGS scribbles on the union portion
 * of the ifreq structure, which may confuse other parts of ifconfig.
 * Make a private copy so we can avoid that.
 */
/* ARGSUSED */
void
setifflags(const char *vname, int value)
{
    struct ifreq my_ifr;

    bcopy((char *)&ifr, (char *)&my_ifr, sizeof(struct ifreq));

//    if (_ifp->if_ioctl(_ifp, SIOCGIFFLAGS, (caddr_t)&my_ifr) == -1)
//        err(1, "SIOCGIFFLAGS");
//    (void) strlcpy(my_ifr.ifr_name, ifname, sizeof(my_ifr.ifr_name));
//    flags = my_ifr.ifr_flags;
    flags = _ifp->if_flags;

    if (value < 0) {
        value = -value;
        flags &= ~value;
    } else
        flags |= value;
    _ifp->if_flags = flags;
    if (_ifp->if_ioctl(_ifp, SIOCSIFFLAGS, (caddr_t)&my_ifr) == -1)
        err(1, "SIOCSIFFLAGS");
}


void    ieee80211_status();
void    join_status();
void    ieee80211_listchans();
void    ieee80211_listnodes();
void    ieee80211_printnode(struct ieee80211_nodereq *);

void
setifjoin(const char *val, int d)
{
    int len;
    
    if (strlen(nwidname) != 0) {
        errx(1, "nwid and join may not be used at the same time");
    }
    
    if (strlen(joinname) != 0) {
        errx(1, "join may not be specified twice");
    }
    
    if (d != 0) {
        /* no network id is especially desired */
        memset(&join, 0, sizeof(join));
        len = 0;
    } else {
        len = sizeof(join.i_nwid);
        if (get_string(val, NULL, join.i_nwid, &len) == NULL)
            return;
        if (len == 0)
            join.i_flags |= IEEE80211_JOIN_ANY;
    }
    join.i_len = len;
    (void)strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    (void)strlcpy(joinname, (char *)join.i_nwid, sizeof(joinname));
    
    actions |= A_JOIN;
}


int
isspace(int c) {
    switch (c) {
        case 0x20:
        case 0xD:
        case 0xA:
        case 0x9:
            return 1;
    }
    return 0;
}

static char *
_ether_aton(const char *s, struct ether_addr *e)
{
    int i;
    long l;
    char *pp = NULL;
    
    while (isspace((unsigned char)*s))
        s++;
    
    /* expect 6 hex octets separated by ':' or space/NUL if last octet */
    for (i = 0; i < 6; i++) {
        l = strtol(s, &pp, 16);
        if (pp == s || l > 0xFF || l < 0)
            return (NULL);
        if (!(*pp == ':' ||
              (i == 5 && (isspace((unsigned char)*pp) ||
                          *pp == '\0'))))
            return (NULL);
        e->ether_addr_octet[i] = (u_char)l;
        s = pp + 1;
    }
    
    /* return character after the octets ala strtol(3) */
    return (pp);
}

struct ether_addr *
ether_aton(const char *s)
{
    static struct ether_addr n;
    
    return (_ether_aton(s, &n) ? &n : NULL);
}

void
setifbssid(const char *val, int d)
{
    
    struct ieee80211_bssid bssid;
    struct ether_addr *ea;
    
    if (d != 0) {
        /* no BSSID is especially desired */
        memset(&bssid.i_bssid, 0, sizeof(bssid.i_bssid));
    } else {
        ea = ether_aton((char*)val);
        if (ea == NULL) {
            errx(1,"malformed BSSID: %s", val);
            return;
        }
        memcpy(&bssid.i_bssid, ea->ether_addr_octet,
               sizeof(bssid.i_bssid));
    }
    strlcpy(bssid.i_name, ifname, sizeof(bssid.i_name));
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211BSSID, (caddr_t)&bssid) == -1)
        warn("SIOCS80211BSSID");
}

void
setifnwkey(const char *val, int d)
{
    int i, len;
    struct ieee80211_nwkey nwkey;
    u_int8_t keybuf[IEEE80211_WEP_NKID][16];
    
    bzero(&nwkey, sizeof(nwkey));
    bzero(&keybuf, sizeof(keybuf));
    
    nwkey.i_wepon = IEEE80211_NWKEY_WEP;
    nwkey.i_defkid = 1;
    if (d == -1) {
        /* disable WEP encryption */
        nwkey.i_wepon = IEEE80211_NWKEY_OPEN;
        i = 0;
    } else if (strcasecmp("persist", val) == 0) {
        /* use all values from persistent memory */
        nwkey.i_wepon |= IEEE80211_NWKEY_PERSIST;
        nwkey.i_defkid = 0;
        for (i = 0; i < IEEE80211_WEP_NKID; i++)
            nwkey.i_key[i].i_keylen = -1;
    } else if (strncasecmp("persist:", val, 8) == 0) {
        val += 8;
        /* program keys in persistent memory */
        nwkey.i_wepon |= IEEE80211_NWKEY_PERSIST;
        goto set_nwkey;
    } else {
    set_nwkey:
        if (isdigit((unsigned char)val[0]) && val[1] == ':') {
            /* specifying a full set of four keys */
            nwkey.i_defkid = val[0] - '0';
            val += 2;
            for (i = 0; i < IEEE80211_WEP_NKID; i++) {
                len = sizeof(keybuf[i]);
                val = get_string(val, ",", keybuf[i], &len);
                if (val == NULL)
                    return;
                nwkey.i_key[i].i_keylen = len;
                nwkey.i_key[i].i_keydat = keybuf[i];
            }
            if (*val != '\0') {
                warnx("SIOCS80211NWKEY: too many keys.");
                return;
            }
        } else {
            /*
             * length of each key must be either a 5
             * character ASCII string or 10 hex digits for
             * 40 bit encryption, or 13 character ASCII
             * string or 26 hex digits for 128 bit
             * encryption.
             */
            int j;
            char *tmp = NULL;
            size_t vlen = strlen(val);
            switch(vlen) {
                case 10:
                case 26:
                    /* 0x must be missing for these lengths */
                    tmp = (char *)malloc(vlen + 3, M_DEVBUF, M_NOWAIT | M_ZERO);
                    j = snprintf(tmp, vlen + 3, "0x%s", val);
                    if (j == -1) {
                        warnx("malloc failed");
                        return;
                    }
                    val = tmp;
                    break;
                case 12:
                case 28:
                case 5:
                case 13:
                    /* 0xkey or string case - all is ok */
                    break;
                default:
                    warnx("Invalid WEP key length");
                    return;
            }
            len = sizeof(keybuf[0]);
            val = get_string(val, NULL, keybuf[0], &len);
            free(tmp, M_DEVBUF, vlen);
            if (val == NULL)
                return;
            nwkey.i_key[0].i_keylen = len;
            nwkey.i_key[0].i_keydat = keybuf[0];
            i = 1;
        }
    }
    (void)strlcpy(nwkey.i_name, ifname, sizeof(nwkey.i_name));
    
    if (actions & A_JOIN) {
        memcpy(&join.i_nwkey, &nwkey, sizeof(join.i_nwkey));
        join.i_flags |= IEEE80211_JOIN_NWKEY;
        return;
    }
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211NWKEY, (caddr_t)&nwkey) == -1)
        warn("SIOCS80211NWKEY");
}


char *
strdup(const char *s)
{
    size_t len = strlen (s) + 1;
    char *result = (char*)malloc(len, M_DEVBUF,
                                 M_NOWAIT | M_ZERO);

    if (result == (char*) 0)
        return (char*) 0;
    return (char*) memcpy (result, s, len);
}

char *
strtok_r(char *s, const char *delim, char **last)
{
    const char *spanp;
    int c, sc;
    char *tok;
    
    if (s == NULL && (s = *last) == NULL)
        return (NULL);
    
    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }
    
    if (c == 0) {        /* no non-delimiter characters */
        *last = NULL;
        return (NULL);
    }
    tok = s - 1;
    
    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = '\0';
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

char *
strtok(char *s, const char *delim)
{
    static char *last;
    
    return strtok_r(s, delim, &last);
}

/* ARGSUSED */
void
setifwpaakms(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    char *optlist, *str;
    u_int rval = 0;
    
    if ((optlist = strdup(val)) == NULL)
        errx(1, "strdup");
    str = strtok(optlist, ",");
    while (str != NULL) {
        if (strcasecmp(str, "psk") == 0)
            rval |= IEEE80211_WPA_AKM_PSK;
        else if (strcasecmp(str, "802.1x") == 0)
            rval |= IEEE80211_WPA_AKM_8021X;
        else
            errx(1, "wpaakms: unknown akm: %s", str);
        str = strtok(NULL, ",");
    }
    free(optlist, M_DEVBUF, strlen(optlist));
    
    if (actions & A_JOIN) {
        join.i_wpaparams.i_akms = rval;
        join.i_wpaparams.i_enabled =
        ((rval & IEEE80211_WPA_AKM_8021X) != 0);
        join.i_flags |= IEEE80211_JOIN_WPA;
        return;
    }
    
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    if (_ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCG80211WPAPARMS");
    wpa.i_akms = rval;
    /* Enable WPA for 802.1x here. PSK case is handled in setifwpakey(). */
    wpa.i_enabled = ((rval & IEEE80211_WPA_AKM_8021X) != 0);
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCS80211WPAPARMS");
}


/* ARGSUSED */
void
setifwpaprotos(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    char *optlist, *str;
    u_int rval = 0;
    
    if ((optlist = strdup(val)) == NULL)
        errx(1, "strdup");
    str = strtok(optlist, ",");
    while (str != NULL) {
        if (strcasecmp(str, "wpa1") == 0)
            rval |= IEEE80211_WPA_PROTO_WPA1;
        else if (strcasecmp(str, "wpa2") == 0)
            rval |= IEEE80211_WPA_PROTO_WPA2;
        else
            errx(1, "wpaprotos: unknown protocol: %s", str);
        str = strtok(NULL, ",");
    }
    free(optlist, M_DEVBUF, strlen(optlist));
    
    if (actions & A_JOIN) {
        join.i_wpaparams.i_protos = rval;
        join.i_flags |= IEEE80211_JOIN_WPA;
        return;
    }
    
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    if (_ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCG80211WPAPARMS");
    wpa.i_protos = rval;
    /* Let the kernel set up the appropriate default ciphers. */
    wpa.i_ciphers = 0;
    wpa.i_groupcipher = 0;
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCS80211WPAPARMS");
    
    struct ieee80211_txpower power = {NULL, 0, 100};
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211TXPOWER, (caddr_t)&power) == -1)
        errx(1, "SIOCS80211TXPOWER");
}


static const struct {
    const char    *name;
    u_int        cipher;
} ciphers[] = {
    { "usegroup",    IEEE80211_WPA_CIPHER_USEGROUP },
    { "wep40",    IEEE80211_WPA_CIPHER_WEP40 },
    { "tkip",    IEEE80211_WPA_CIPHER_TKIP },
    { "ccmp",    IEEE80211_WPA_CIPHER_CCMP },
    { "wep104",    IEEE80211_WPA_CIPHER_WEP104 }
};

u_int
getwpacipher(const char *name)
{
    int i;
    
    for (i = 0; i < sizeof(ciphers) / sizeof(ciphers[0]); i++)
        if (strcasecmp(name, ciphers[i].name) == 0)
            return ciphers[i].cipher;
    return IEEE80211_WPA_CIPHER_NONE;
}

/* ARGSUSED */
void
setifwpaciphers(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    char *optlist, *str;
    u_int rval = 0;
    
    if ((optlist = strdup(val)) == NULL)
        errx(1, "strdup");
    str = strtok(optlist, ",");
    while (str != NULL) {
        u_int cipher = getwpacipher(str);
        if (cipher == IEEE80211_WPA_CIPHER_NONE)
            errx(1, "wpaciphers: unknown cipher: %s", str);
        
        rval |= cipher;
        str = strtok(NULL, ",");
    }
    free(optlist, M_DEVBUF, strlen(optlist));
    
    if (actions & A_JOIN) {
        join.i_wpaparams.i_ciphers = rval;
        join.i_flags |= IEEE80211_JOIN_WPA;
        return;
    }
    
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    if (_ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCG80211WPAPARMS");
    wpa.i_ciphers = rval;
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCS80211WPAPARMS");
}


/* ARGSUSED */
void
setifwpagroupcipher(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    u_int cipher;
    
    cipher = getwpacipher(val);
    if (cipher == IEEE80211_WPA_CIPHER_NONE)
        errx(1, "wpagroupcipher: unknown cipher: %s", val);
    
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    if (_ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCG80211WPAPARMS");
    wpa.i_groupcipher = cipher;
    
    if (actions & A_JOIN) {
        join.i_wpaparams.i_groupcipher = cipher;
        join.i_flags |= IEEE80211_JOIN_WPA;
        return;
    }
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCS80211WPAPARMS");
}


void
setifnwid(const char *val, int d)
{
    struct ieee80211_nwid nwid;
    int len;
    
    if (strlen(joinname) != 0) {
        errx(1, "nwid and join may not be used at the same time");
    }
    
    if (d != 0) {
        /* no network id is especially desired */
        memset(&nwid, 0, sizeof(nwid));
        len = 0;
    } else {
        len = sizeof(nwid.i_nwid);
        if (get_string(val, NULL, nwid.i_nwid, &len) == NULL)
            return;
    }
    nwid.i_len = len;
    (void)strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    (void)strlcpy(nwidname, (char *)nwid.i_nwid, sizeof(nwidname));
    ifr.ifr_data = (caddr_t)&nwid;
    if (_ifp->if_ioctl(_ifp, SIOCS80211NWID, (caddr_t)&ifr) < 0)
        warn("SIOCS80211NWID");
}

void
process_join_commands()
{
    int len;
    
    if (!(actions & A_JOIN))
        return;

    ifr.ifr_data = (caddr_t)&join;
    _ifp->if_ioctl(_ifp, SIOCS80211JOIN, (caddr_t)&ifr);
}


/* ARGSUSED */
void
setifwpa(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    /* Don't read current values. The kernel will set defaults. */
    wpa.i_enabled = d;
    
    if (actions & A_JOIN) {
        join.i_wpaparams.i_enabled = d;
        join.i_flags |= IEEE80211_JOIN_WPA;
        return;
    }
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) < 0)
        errx(1, "SIOCS80211WPAPARMS");
}


void
setifwpakey(const char *val, int d)
{
    struct ieee80211_wpaparams wpa;
    struct ieee80211_wpapsk psk;
    struct ieee80211_nwid nwid;
    int passlen;
    
    memset(&psk, 0, sizeof(psk));
    if (d != -1) {
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_data = (caddr_t)&nwid;
        strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
        
        /* Use the value specified in 'join' or 'nwid' */
        if (strlen(joinname) != 0) {
            strlcpy((char *)nwid.i_nwid, joinname, sizeof(nwid.i_nwid));
            nwid.i_len = strlen(joinname);
        } else if (strlen(nwidname) != 0) {
            strlcpy((char *)nwid.i_nwid, nwidname, sizeof(nwid.i_nwid));
            nwid.i_len = strlen(nwidname);
        } else {
            warnx("no nwid or join command, guessing nwid to use");
            
            if (_ifp->if_ioctl(_ifp, SIOCG80211NWID, (caddr_t)&ifr) == -1)
                errx(1, "SIOCG80211NWID");
        }
        
        passlen = strlen(val);
        if (passlen == 2 + 2 * sizeof(psk.i_psk) &&
            val[0] == '0' && val[1] == 'x') {
            /* Parse a WPA hex key (must be full-length) */
            passlen = sizeof(psk.i_psk);
            val = get_string(val, NULL, psk.i_psk, &passlen);
            if (val == NULL || passlen != sizeof(psk.i_psk))
                errx(1, "wpakey: invalid pre-shared key");
        } else {
            /* Parse a WPA passphrase */
            if (passlen < 8 || passlen > 63)
                errx(1, "wpakey: passphrase must be between "
                     "8 and 63 characters");
            if (nwid.i_len == 0)
                errx(1, "wpakey: nwid not set");
            
            if (pkcs5_pbkdf2(val, passlen, nwid.i_nwid, nwid.i_len,
                             psk.i_psk, sizeof(psk.i_psk), 4096) != 0)
                errx(1, "wpakey: passphrase hashing failed");
        }
        
        kprintf("--%s: line = %d psk.i_psk = \n", __FUNCTION__, __LINE__);
        kprintf("%02x %02x %02x %02x %02x %02x %02x %02x \n",psk.i_psk[0], psk.i_psk[1], psk.i_psk[2], psk.i_psk[3], psk.i_psk[4],psk.i_psk[5], psk.i_psk[6], psk.i_psk[7]);
        kprintf("%02x %02x %02x %02x %02x %02x %02x %02x \n", psk.i_psk[8], psk.i_psk[9],psk.i_psk[10], psk.i_psk[11], psk.i_psk[12], psk.i_psk[13], psk.i_psk[14],psk.i_psk[15]);
        kprintf("%02x %02x %02x %02x %02x %02x %02x %02x \n", psk.i_psk[16], psk.i_psk[17], psk.i_psk[18], psk.i_psk[19],psk.i_psk[20], psk.i_psk[21], psk.i_psk[22], psk.i_psk[23]);
        kprintf("%02x %02x %02x %02x %02x %02x %02x %02x \n\n", psk.i_psk[24],psk.i_psk[25], psk.i_psk[26], psk.i_psk[27], psk.i_psk[28], psk.i_psk[29], psk.i_psk[30], psk.i_psk[31]);
        
        psk.i_enabled = 1;
    } else
        psk.i_enabled = 0;
    
    (void)strlcpy(psk.i_name, ifname, sizeof(psk.i_name));
    
    if (actions & A_JOIN) {
        memcpy(&join.i_wpapsk, &psk, sizeof(join.i_wpapsk));
        join.i_flags |= IEEE80211_JOIN_WPAPSK;
        if (!join.i_wpaparams.i_enabled)
            setifwpa( NULL, join.i_wpapsk.i_enabled);
        return;
    }
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPSK, (caddr_t)&psk) == -1)
        errx(1, "SIOCS80211WPAPSK");
    
    /* And ... automatically enable or disable WPA */
    memset(&wpa, 0, sizeof(wpa));
    (void)strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    if (_ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCG80211WPAPARMS");
    wpa.i_enabled = psk.i_enabled;
    if (_ifp->if_ioctl(_ifp, SIOCS80211WPAPARMS, (caddr_t)&wpa) == -1)
        errx(1, "SIOCS80211WPAPARMS");
}

__dead void
usage(void)
{
//    fprintf(stderr,
//        "usage: ifconfig [-AaC] [interface] [address_family] "
//        "[address [dest_address]]\n"
//        "\t\t[parameters]\n");
//    exit(1);
}

const char    *_ctype_;

#define    _U    0x01
#define    _L    0x02
#define    _N    0x04
#define    _S    0x08
#define    _P    0x10
#define    _C    0x20
#define    _X    0x40
#define    _B    0x80

#define EOF (-1)

int
isalpha(int c)
{
    return (c == EOF ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_U|_L)));
}


int
isupper(int c)
{
    return (c == EOF ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _U));
}

/*
 * Convert a string to a long long.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long long
strtoll(const char *nptr, char **endptr, int base)
{
    const char *s;
    long long acc, cutoff;
    int c;
    int neg, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = (unsigned char) *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for long longs is
     * [-9223372036854775808..9223372036854775807] and the input base
     * is 10, cutoff will be set to 922337203685477580 and cutlim to
     * either 7 (neg==0) or 8 (neg==1), meaning that if we have
     * accumulated a value > 922337203685477580, or equal but the
     * next digit is > 7 (or 8), the number is too big, and we will
     * return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? LLONG_MIN : LLONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    if (neg) {
        if (cutlim > 0) {
            cutlim -= base;
            cutoff += 1;
        }
        cutlim = -cutlim;
    }
    for (acc = 0, any = 0;; c = (unsigned char) *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0)
            continue;
        if (neg) {
            if (acc < cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LLONG_MIN;
            } else {
                any = 1;
                acc *= base;
                acc -= c;
            }
        } else {
            if (acc > cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LLONG_MAX;
            } else {
                any = 1;
                acc *= base;
                acc += c;
            }
        }
    }
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (acc);
}


#define    INVALID        1
#define    TOOSMALL    2
#define    TOOLARGE    3

long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp)
{
    int errno = 0;
    long long ll = 0;
    int error = 0;
    char *ep;
    struct errval {
        const char *errstr;
        int err;
    } ev[4] = {
        { NULL,        0 },
        { "invalid",    EINVAL },
        { "too small",    ERANGE },
        { "too large",    ERANGE },
    };

    ev[0].err = errno;
    errno = 0;
    if (minval > maxval) {
        error = INVALID;
    } else {
        ll = strtoll(numstr, &ep, 10);
        if (numstr == ep || *ep != '\0')
            error = INVALID;
        else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
            error = TOOSMALL;
        else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
            error = TOOLARGE;
    }
    if (errstrp != NULL)
        *errstrp = ev[error].errstr;
    errno = ev[error].err;
    if (error)
        ll = 0;

    return (ll);
}

void
setifchan(const char *val, int d)
{
    struct ieee80211chanreq channel;
    const char *errstr;
    int chan;

    if (val == NULL) {
        if (shownet80211chans || shownet80211nodes)
            usage();
        shownet80211chans = 1;
        return;
    }
    if (d != 0)
        chan = IEEE80211_CHAN_ANY;
    else {
        chan = strtonum(val, 1, 256, &errstr);
        if (errstr) {
//            warnx("invalid channel %s: %s", val, errstr);
            return;
        }
    }

    strlcpy(channel.i_name, ifname, sizeof(channel.i_name));
    channel.i_channel = (u_int16_t)chan;
    if (_ifp->if_ioctl(_ifp, SIOCS80211CHANNEL, (caddr_t)&channel) == -1)
        warn("SIOCS80211CHANNEL");
}


char *
ether_ntoa(struct ether_addr *e)
{
    static char a[] = "xx:xx:xx:xx:xx:xx";
    
    (void)snprintf(a, sizeof a, "%02x:%02x:%02x:%02x:%02x:%02x",
                   e->octet[0], e->octet[1],
                   e->octet[2], e->octet[3],
                   e->octet[4], e->octet[5]);
    
    return (a);
}


void
print_cipherset(u_int32_t cipherset)
{
    const char *sep = "";
    int i;
    
    if (cipherset == IEEE80211_WPA_CIPHER_NONE) {
        printf("none");
        return;
    }
    for (i = 0; i < sizeof(ciphers) / sizeof(ciphers[0]); i++) {
        if (cipherset & ciphers[i].cipher) {
            printf("%s%s", sep, ciphers[i].name);
            sep = ",";
        }
    }
}


/*
 * A simple version of printb for status output
 */
void
printb_status(unsigned short v, unsigned char *bits)
{
    int i, any = 0;
    unsigned char c;
    
    if (bits) {
        bits++;
        while ((i = *bits++)) {
            if (v & (1 << (i-1))) {
                if (any)
                    printf("%c", ',');
                any = 1;
                for (; (c = *bits) > 32; bits++)
                    printf("%c", tolower(c));
            } else
                for (; *bits > 32; bits++)
                    ;
        }
    }
}


#define isprint(c)      ((c) >= 0x20 && (c) < 0x7f)

#define isspace(c)    ((c) == ' ' || (c) == '\t')


int
len_string(const u_int8_t *buf, int len)
{
    int i = 0, hasspc = 0;
    
    if (len < 2 || buf[0] != '0' || tolower(buf[1]) != 'x') {
        for (; i < len; i++) {
            /* Only print 7-bit ASCII keys */
            if (buf[i] & 0x80 || !isprint(buf[i]))
                break;
            if (isspace(buf[i]))
                hasspc++;
        }
    }
    if (i == len) {
        if (hasspc || len == 0)
            return len + 2;
        else
            return len;
    } else
        return (len * 2) + 2;
}


int
print_string(const u_int8_t *buf, int len)
{
    int i = 0, hasspc = 0;
    
    if (len < 2 || buf[0] != '0' || tolower(buf[1]) != 'x') {
        for (; i < len; i++) {
            /* Only print 7-bit ASCII keys */
            if (buf[i] & 0x80 || !isprint(buf[i]))
                break;
            if (isspace(buf[i]))
                hasspc++;
        }
    }
    if (i == len) {
        if (hasspc || len == 0) {
            printf("\"%.*s\"", len, buf);
            return len + 2;
        } else {
            printf("%.*s", len, buf);
            return len;
        }
    } else {
        printf("0x");
        for (i = 0; i < len; i++)
            printf("%02x", buf[i]);
        return (len * 2) + 2;
    }
}


int    show_join = 0;

void
showjoin(const char *cmd, int val)
{
    show_join = 1;
    return;
}


void
delifjoin(const char *val, int d)
{
    struct ieee80211_join join;
    int len;
    
    memset(&join, 0, sizeof(join));
    len = 0;
    join.i_flags |= IEEE80211_JOIN_DEL;
    
    if (d == -1) {
        ifr.ifr_data = (caddr_t)&join;
        if (_ifp->if_ioctl(_ifp, SIOCS80211JOIN, (caddr_t)&ifr) == -1)
            errx(1, "SIOCS80211JOIN");
    }
    
    len = sizeof(join.i_nwid);
    if (get_string(val, NULL, join.i_nwid, &len) == NULL)
        return;
    join.i_len = len;
    if (len == 0)
        join.i_flags |= IEEE80211_JOIN_ANY;
    (void)strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_data = (caddr_t)&join;
    if (_ifp->if_ioctl(_ifp, SIOCS80211JOIN, (caddr_t)&ifr) == -1)
        errx(1, "SIOCS80211JOIN");
}

int errno = 0;

void
join_status()
{
    struct ieee80211_joinreq_all ja;
    struct ieee80211_join *jn = NULL;
    struct ieee80211_wpaparams *wpa;
    int jsz = 100;
    int ojsz;
    int i;
    int r;
    int maxlen, len;
    
    bzero(&ja, sizeof(ja));
    jn = IONew(struct ieee80211_join, jsz);
    if (jn == NULL)
        errx(1, "recallocarray");
    ojsz = jsz;
    while (1) {
        ja.ja_node = jn;
        ja.ja_size = jsz * sizeof(*jn);
        strlcpy(ja.ja_ifname, ifname, sizeof(ja.ja_ifname));
        
        if ((r = _ifp->if_ioctl(_ifp, SIOCG80211JOINALL, (caddr_t)&ja)) != 0) {
            if (errno == E2BIG) {
                jsz += 100;
                jn = IONew(struct ieee80211_join, jsz);
                if (jn == NULL)
                    errx(1, "recallocarray");
                ojsz = jsz;
                continue;
            } else if (errno != ENOENT)
                warn("SIOCG80211JOINALL");
            return;
        }
        break;
    }
    
    if (!ja.ja_nodes)
        return;
    
    maxlen = 0;
    for (i = 0; i < ja.ja_nodes; i++) {
        len = len_string(jn[i].i_nwid, jn[i].i_len);
        if (len > maxlen)
            maxlen = len;
    }
    if (maxlen > IEEE80211_NWID_LEN)
        maxlen = IEEE80211_NWID_LEN - 1;
    
    for (i = 0; i < ja.ja_nodes; i++) {
        printf("\t      ");
        if (jn[i].i_len > IEEE80211_NWID_LEN)
            jn[i].i_len = IEEE80211_NWID_LEN;
        len = print_string(jn[i].i_nwid, jn[i].i_len);
        printf("%-*s", maxlen - len, " ");
        if (jn[i].i_flags) {
            const char *sep;
            printf(" ");
            
            if (jn[i].i_flags & IEEE80211_JOIN_NWKEY)
                printf("nwkey");
            
            if (jn[i].i_flags & IEEE80211_JOIN_WPA) {
                wpa = &jn[i].i_wpaparams;
                
                printf("wpaprotos "); sep = "";
                if (wpa->i_protos & IEEE80211_WPA_PROTO_WPA1) {
                    printf("wpa1");
                    sep = ",";
                }
                if (wpa->i_protos & IEEE80211_WPA_PROTO_WPA2)
                    printf("%swpa2", sep);
                
                printf(" wpaakms ");
                sep = "";
                if (wpa->i_akms & IEEE80211_WPA_AKM_PSK) {
                    printf("psk");
                    sep = ",";
                }
                if (wpa->i_akms & IEEE80211_WPA_AKM_8021X)
                    printf("%s802.1x", sep);
                
                printf(" wpaciphers ");
                print_cipherset(wpa->i_ciphers);
                
                printf(" wpagroupcipher ");
                print_cipherset(wpa->i_groupcipher);
            }
        }
        putchar('\n');
    }
}


void
ieee80211_listchans()
{
    static struct ieee80211_channel chans[256+1];
    struct ieee80211_chanreq_all ca;
    int i;
    
    bzero(&ca, sizeof(ca));
    bzero(chans, sizeof(chans));
    ca.i_chans = chans;
    strlcpy(ca.i_name, ifname, sizeof(ca.i_name));
    
    if (_ifp->if_ioctl(_ifp, SIOCG80211ALLCHANS, (caddr_t)&ca) != 0) {
        warn("SIOCG80211ALLCHANS");
        return;
    }
    printf("\t\t%4s  %-8s  %s\n", "chan", "freq", "properties");
    for (i = 1; i <= 256; i++) {
        if (chans[i].ic_flags == 0)
            continue;
        printf("\t\t%4d  %4d MHz  ", i, chans[i].ic_freq);
        if (chans[i].ic_flags & IEEE80211_CHAN_PASSIVE)
            printf("passive scan");
        else
            putchar('-');
        putchar('\n');
    }
}


void
ieee80211_listnodes()
{
    struct ieee80211_nodereq_all na;
    struct ieee80211_nodereq nr[512];
    struct ifreq ifr;
    int i;
    
    if ((flags & IFF_UP) == 0) {
        printf("\t\tcannot scan, interface is down\n");
        return;
    }
    
    bzero(&ifr, sizeof(ifr));
    strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    
    if (_ifp->if_ioctl(_ifp, SIOCS80211SCAN, (caddr_t)&ifr) != 0) {
        if (errno == EPERM)
            printf("\t\tno permission to scan\n");
        return;
    }
    
    bzero(&na, sizeof(na));
    bzero(&nr, sizeof(nr));
    na.na_node = nr;
    na.na_size = sizeof(nr);
    strlcpy(na.na_ifname, ifname, sizeof(na.na_ifname));
    
    if (_ifp->if_ioctl(_ifp, SIOCG80211ALLNODES, (caddr_t)&na) != 0) {
        warn("SIOCG80211ALLNODES");
        return;
    }
    
    if (!na.na_nodes)
        printf("\t\tnone\n");
//    else
//        qsort(nr, na.na_nodes, sizeof(*nr), rssicmp);
    
    for (i = 0; i < na.na_nodes; i++) {
        printf("\t\t");
        ieee80211_printnode(&nr[i]);
        putchar('\n');
    }
}

void
ieee80211_printnode(struct ieee80211_nodereq *nr)
{
    int len, i;
    
    if (nr->nr_flags & IEEE80211_NODEREQ_AP ||
        nr->nr_capinfo & IEEE80211_CAPINFO_IBSS) {
        len = nr->nr_nwid_len;
        if (len > IEEE80211_NWID_LEN)
            len = IEEE80211_NWID_LEN;
        printf("nwid ");
        print_string(nr->nr_nwid, len);
        putchar(' ');
        
        printf("chan %u ", nr->nr_channel);
        
        printf("bssid %s ",
               ether_ntoa((struct ether_addr*)nr->nr_bssid));
    }
    
    if ((nr->nr_flags & IEEE80211_NODEREQ_AP) == 0)
        printf("lladdr %s ",
               ether_ntoa((struct ether_addr*)nr->nr_macaddr));
    
    if (nr->nr_max_rssi)
        printf("%u%% ", IEEE80211_NODEREQ_RSSI(nr));
    else
        printf("%ddBm ", nr->nr_rssi);
    
    if (nr->nr_pwrsave)
        printf("powersave ");
    /*
     * Print our current Tx rate for associated nodes.
     * Print the fastest supported rate for APs.
     */
    if ((nr->nr_flags & (IEEE80211_NODEREQ_AP)) == 0) {
        if (nr->nr_flags & IEEE80211_NODEREQ_HT) {
            printf("HT-MCS%d ", nr->nr_txmcs);
        } else if (nr->nr_nrates) {
            printf("%uM ",
                   (nr->nr_rates[nr->nr_txrate] & IEEE80211_RATE_VAL)
                   / 2);
        }
    } else if (nr->nr_max_rxrate) {
        printf("%uM HT ", nr->nr_max_rxrate);
    } else if (nr->nr_rxmcs[0] != 0) {
        for (i = IEEE80211_HT_NUM_MCS - 1; i >= 0; i--) {
            if (nr->nr_rxmcs[i / 8] & (1 << (i / 10)))
                break;
        }
        printf("HT-MCS%d ", i);
    } else if (nr->nr_nrates) {
        printf("%uM ",
               (nr->nr_rates[nr->nr_nrates - 1] & IEEE80211_RATE_VAL) / 2);
    }
    /* ESS is the default, skip it */
    nr->nr_capinfo &= ~IEEE80211_CAPINFO_ESS;
    if (nr->nr_capinfo) {
        printb_status(nr->nr_capinfo, (unsigned char *)IEEE80211_CAPINFO_BITS);
        if (nr->nr_capinfo & IEEE80211_CAPINFO_PRIVACY) {
            if (nr->nr_rsnprotos) {
                if (nr->nr_rsnprotos & IEEE80211_WPA_PROTO_WPA2)
//                    fputs(",wpa2", stdout);
                    printf(",wpa2");
                if (nr->nr_rsnprotos & IEEE80211_WPA_PROTO_WPA1)
//                    fputs(",wpa1", stdout);
                    printf(",wpa1");
            } else
//                fputs(",wep", stdout);
            printf(",wep");
            
            if (nr->nr_rsnakms & IEEE80211_WPA_AKM_8021X ||
                nr->nr_rsnakms & IEEE80211_WPA_AKM_SHA256_8021X)
//                fputs(",802.1x", stdout);
            printf(",802.1x");
        }
        putchar(' ');
    }
    
    if ((nr->nr_flags & IEEE80211_NODEREQ_AP) == 0)
        printb_status(IEEE80211_NODEREQ_STATE(nr->nr_state),
                      (unsigned char *)IEEE80211_NODEREQ_STATE_BITS);
}

void
ieee80211_status()
{
    int len, inwid, ijoin, inwkey, ipsk, ichan, ipwr;
    int ibssid, iwpa;
    struct ieee80211_nwid nwid;
    struct ieee80211_join join;
    struct ieee80211_nwkey nwkey;
    struct ieee80211_wpapsk psk;
    struct ieee80211_power power;
    struct ieee80211chanreq channel;
    struct ieee80211_bssid bssid;
    struct ieee80211_wpaparams wpa;
    struct ieee80211_nodereq nr;
    u_int8_t zero_bssid[IEEE80211_ADDR_LEN];
    struct ether_addr ea;
    
    /* get current status via ioctls */
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_data = (caddr_t)&nwid;
    strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    inwid = _ifp->if_ioctl(_ifp, SIOCG80211NWID, (caddr_t)&ifr);
    
    ifr.ifr_data = (caddr_t)&join;
    strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ijoin = _ifp->if_ioctl(_ifp, SIOCG80211JOIN, (caddr_t)&ifr);
    
    memset(&nwkey, 0, sizeof(nwkey));
    strlcpy(nwkey.i_name, ifname, sizeof(nwkey.i_name));
    inwkey = _ifp->if_ioctl(_ifp, SIOCG80211NWKEY, (caddr_t)&nwkey);
    
    memset(&psk, 0, sizeof(psk));
    strlcpy(psk.i_name, ifname, sizeof(psk.i_name));
    ipsk = _ifp->if_ioctl(_ifp, SIOCG80211WPAPSK, (caddr_t)&psk);
    
    memset(&power, 0, sizeof(power));
    strlcpy(power.i_name, ifname, sizeof(power.i_name));
    ipwr = _ifp->if_ioctl(_ifp, SIOCG80211POWER, (caddr_t)&power);
    
    memset(&channel, 0, sizeof(channel));
    strlcpy(channel.i_name, ifname, sizeof(channel.i_name));
    ichan = _ifp->if_ioctl(_ifp, SIOCG80211CHANNEL, (caddr_t)&channel);
    
    memset(&bssid, 0, sizeof(bssid));
    strlcpy(bssid.i_name, ifname, sizeof(bssid.i_name));
    ibssid = _ifp->if_ioctl(_ifp, SIOCG80211BSSID, (caddr_t)&bssid);
    
    memset(&wpa, 0, sizeof(wpa));
    strlcpy(wpa.i_name, ifname, sizeof(wpa.i_name));
    iwpa = _ifp->if_ioctl(_ifp, SIOCG80211WPAPARMS, (caddr_t)&wpa);
    
    /* check if any ieee80211 option is active */
    if (inwid == 0 || ijoin == 0 || inwkey == 0 || ipsk == 0 ||
        ipwr == 0 || ichan == 0 || ibssid == 0 || iwpa == 0)
//        fputs("\tieee80211:", stdout);
        printf("---------\n");
    else
        return;
    
//    if (inwid == 0) {
//        /* nwid.i_nwid is not NUL terminated. */
//        len = nwid.i_len;
//        if (len > IEEE80211_NWID_LEN)
//            len = IEEE80211_NWID_LEN;
//        if (ijoin == 0 && join.i_flags & IEEE80211_JOIN_FOUND)
//            fputs(" join ", stdout);
//        else
//            fputs(" nwid ", stdout);
//        print_string(nwid.i_nwid, len);
//    }
    
    if (ichan == 0 && channel.i_channel != 0 &&
        channel.i_channel != IEEE80211_CHAN_ANY)
        printf(" chan %u", channel.i_channel);
    
    memset(&zero_bssid, 0, sizeof(zero_bssid));
    if (ibssid == 0 &&
        memcmp(bssid.i_bssid, zero_bssid, IEEE80211_ADDR_LEN) != 0) {
        memcpy(&ea.ether_addr_octet, bssid.i_bssid,
               sizeof(ea.ether_addr_octet));
        printf(" bssid %s", ether_ntoa(&ea));
        
        bzero(&nr, sizeof(nr));
        bcopy(bssid.i_bssid, &nr.nr_macaddr, sizeof(nr.nr_macaddr));
        strlcpy(nr.nr_ifname, ifname, sizeof(nr.nr_ifname));
        if (_ifp->if_ioctl(_ifp, SIOCG80211NODE, (caddr_t)&nr) == 0 && nr.nr_rssi) {
            if (nr.nr_max_rssi)
                printf(" %u%%", IEEE80211_NODEREQ_RSSI(&nr));
            else
                printf(" %ddBm", nr.nr_rssi);
        }
    }
    
//    if (inwkey == 0 && nwkey.i_wepon > IEEE80211_NWKEY_OPEN)
//        fputs(" nwkey", stdout);
    
//    if (ipsk == 0 && psk.i_enabled)
//        fputs(" wpakey", stdout);
    if (iwpa == 0 && wpa.i_enabled) {
        const char *sep;
        
//        fputs(" wpaprotos ", stdout); sep = "";
        if (wpa.i_protos & IEEE80211_WPA_PROTO_WPA1) {
//            fputs("wpa1", stdout);
            sep = ",";
        }
        if (wpa.i_protos & IEEE80211_WPA_PROTO_WPA2)
            printf("%swpa2", sep);
        
//        fputs(" wpaakms ", stdout); sep = "";
        if (wpa.i_akms & IEEE80211_WPA_AKM_PSK) {
//            fputs("psk", stdout);
            sep = ",";
        }
        if (wpa.i_akms & IEEE80211_WPA_AKM_8021X)
            printf("%s802.1x", sep);
        
//        fputs(" wpaciphers ", stdout);
        print_cipherset(wpa.i_ciphers);
        
//        fputs(" wpagroupcipher ", stdout);
        print_cipherset(wpa.i_groupcipher);
    }
    
    if (ipwr == 0 && power.i_enabled)
        printf(" powersave on (%dms sleep)", power.i_maxsleep);
    
    if (_ifp->if_ioctl(_ifp, SIOCG80211FLAGS, (caddr_t)&ifr) == 0 &&
        ifr.ifr_flags) {
//        putchar(' ');
        printb_status(ifr.ifr_flags, (unsigned char *)IEEE80211_F_USERBITS);
    }
    printf("%c", '\n');
    if (show_join)
        join_status();
    if (shownet80211chans)
        ieee80211_listchans();
    else if (shownet80211nodes)
        ieee80211_listnodes();
}

