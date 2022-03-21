/*    $OpenBSD: ifconfig.h,v 1.4 2021/11/11 09:39:16 claudio Exp $    */

/*
 * Copyright (c) 2009 Claudio Jeker <claudio@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef ifconfig_hpp
#define ifconfig_hpp

#include "compat.h"
#include "qsort.h"

void    ifconfig(const char **argv, int argc);
void    ifconfig(const char *);


extern int aflag;
extern int ifaliases;
extern int sock;
extern char ifname[IFNAMSIZ];

void printb(char *, unsigned int, unsigned char *);

//void setdiscover(const char *, int);
//void unsetdiscover(const char *, int);
//void setblocknonip(const char *, int);
//void unsetblocknonip(const char *, int);
//void setlearn(const char *, int);
//void unsetlearn(const char *, int);
//void setstp(const char *, int);
//void unsetstp(const char *, int);
//void setedge(const char *, int);
//void unsetedge(const char *, int);
//void setautoedge(const char *, int);
//void unsetautoedge(const char *, int);
//void setptp(const char *, int);
//void unsetptp(const char *, int);
//void setautoptp(const char *, int);
//void unsetautoptp(const char *, int);
//void addlocal(const char *, int);

//void bridge_add(const char *, int);
//void bridge_delete(const char *, int);
//void bridge_addspan(const char *, int);
//void bridge_delspan(const char *, int);
//void bridge_flush(const char *, int);
//void bridge_flushall(const char *, int);
//void bridge_addaddr(const char *, const char *);
//void bridge_deladdr(const char *, int);
//void bridge_maxaddr(const char *, int);
//void bridge_addrs(const char *, int);
//void bridge_hellotime(const char *, int);
//void bridge_fwddelay(const char *, int);
//void bridge_maxage(const char *, int);
//void bridge_protect(const char *, const char *);
//void bridge_unprotect(const char *, int);
//void bridge_proto(const char *, int);
//void bridge_ifprio(const char *, const char *);
//void bridge_ifcost(const char *, const char *);
//void bridge_noifcost(const char *, int);
//void bridge_timeout(const char *, int);
//void bridge_holdcnt(const char *, int);
//void bridge_priority(const char *, int);
//void bridge_rules(const char *, int);
//void bridge_rulefile(const char *, int);
//void bridge_flushrule(const char *, int);
//int is_bridge(void);
//void bridge_status(void);
//int bridge_rule(int, char **, int);

//int if_sff_info(int);




// -----------------------------

static int ioctl(int sock, u_long type, void *add)
{
    return _ifp->if_ioctl(_ifp, type, (caddr_t)add);
}


static char *
strdup(const char *s)
{
    size_t len = strlen (s) + 1;
    char *result = (char*)malloc(len, M_DEVBUF,
                                 M_NOWAIT | M_ZERO);

    if (result == (char*) 0)
        return (char*) 0;
    return (char*) memcpy (result, s, len);
}


#define tolower(c) (isupper(c) ? (c - 'A' + 'a') : c )
#define toupper(c) (islower(c) ? (x - 'a' + 'A') : c)
#define isdigit(c) (c >= '0' && c <= '9')

#define isxdigit(c)   (isdigit((c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))


#define isupper(c)    ((c) >= 'A' && (c) <= 'Z')
#define islower(c)    ((c) >= 'a' && (c) <= 'z')
#define isalpha(c)    (isupper(c)||islower(c))


#define isprint(c)      ((c) >= 0x20 && (c) < 0x7f)
#define isspace(c)    ((c) == ' ' || (c) == '\t')


//static int
//isspace(int c) {
//    switch (c) {
//        case 0x20:
//        case 0xD:
//        case 0xA:
//        case 0x9:
//            return 1;
//    }
//    return 0;
//}


/*
 * Convert a string to a long long.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static long long
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

static long long
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

static struct ether_addr *
ether_aton(const char *s)
{
    static struct ether_addr n;
    
    return (_ether_aton(s, &n) ? &n : NULL);
}

static char *
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

static char *
strtok(char *s, const char *delim)
{
    static char *last;
    
    return strtok_r(s, delim, &last);
}


/*
 * HMAC-SHA-1 (from RFC 2202).
 */
static void
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

#define MINIMUM(a, b)    (((a) < (b)) ? (a) : (b))

/*
 * Password-Based Key Derivation Function 2 (PKCS #5 v2.0).
 * Code based on IEEE Std 802.11-2007, Annex H.4.2.
 */
static int
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


static char *
ether_ntoa(struct ether_addr *e)
{
    static char a[] = "xx:xx:xx:xx:xx:xx";
    
    (void)snprintf(a, sizeof a, "%02x:%02x:%02x:%02x:%02x:%02x",
                   e->octet[0], e->octet[1],
                   e->octet[2], e->octet[3],
                   e->octet[4], e->octet[5]);
    
    return (a);
}


static void split(const char *src, const char *separator,char **dest, int *num)
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

/*
 * Returns an integer less than, equal to, or greater than zero if nr1's
 * RSSI is respectively greater than, equal to, or less than nr2's RSSI.
 */
int rssicmp(const void *nr1, const void *nr2);





#define    SIOCIFGCLONERS    _IOWR('i', 120, struct if_clonereq) /* get cloners */
#define    SIOCSIFDESCR     _IOW('i', 128, struct ifreq)    /* set ifnet descr */
#define    SIOCGIFDESCR    _IOWR('i', 129, struct ifreq)    /* get ifnet descr */
#define    SIOCSIFRTLABEL     _IOW('i', 130, struct ifreq)    /* set ifnet rtlabel */

#define SIOCDELLABEL     _IOW('i', 151, struct ifreq)    /* del MPLS label */
#define SIOCGPWE3     _IOWR('i', 152, struct ifreq)    /* get MPLS PWE3 cap */
#define SIOCSETLABEL     _IOW('i', 153, struct ifreq)    /* set MPLS label */
#define SIOCGETLABEL     _IOW('i', 154, struct ifreq)    /* get MPLS label */

#define SIOCSIFPRIORITY     _IOW('i', 155, struct ifreq)    /* set if priority */
#define SIOCGIFPRIORITY    _IOWR('i', 156, struct ifreq)    /* get if priority */

#define    SIOCSIFXFLAGS     _IOW('i', 157, struct ifreq)    /* set ifnet xflags */
#define    SIOCGIFXFLAGS    _IOWR('i', 158, struct ifreq)    /* get ifnet xflags */

#define SIOCSETKALIVE    _IOW('i', 163, struct ifkalivereq)
#define SIOCGETKALIVE    _IOWR('i', 164, struct ifkalivereq)

#define SIOCSVNETID    _IOW('i', 166, struct ifreq)    /* set virt net id */
#define SIOCGVNETID    _IOWR('i', 167, struct ifreq)    /* get virt net id */

#define SIOCDVNETID    _IOW('i', 175, struct ifreq)    /* del virt net id */

#define SIOCSIFPAIR    _IOW('i', 176, struct ifreq)    /* set paired if */
#define SIOCGIFPAIR    _IOWR('i', 177, struct ifreq)    /* get paired if */

#define SIOCSIFPARENT    _IOW('i', 178, struct if_parent) /* set parent if */
#define SIOCGIFPARENT    _IOWR('i', 179, struct if_parent) /* get parent if */
#define SIOCDIFPARENT    _IOW('i', 180, struct ifreq)    /* del parent if */

#define    SIOCSVNETFLOWID    _IOW('i', 195, struct ifreq)    /* set vnet flowid */
#define    SIOCGVNETFLOWID    _IOWR('i', 196, struct ifreq)    /* get vnet flowid */

#define    SIOCSLIFPHYECN    _IOW('i', 199, struct ifreq)    /* set ecn copying */
#define    SIOCGLIFPHYECN    _IOWR('i', 200, struct ifreq)    /* get ecn copying */

#define SIOCSPWE3CTRLWORD    _IOW('i', 220, struct ifreq)
#define SIOCGPWE3CTRLWORD    _IOWR('i',  220, struct ifreq)
#define SIOCSPWE3FAT        _IOW('i', 221, struct ifreq)
#define SIOCGPWE3FAT        _IOWR('i', 221, struct ifreq)
#define SIOCSPWE3NEIGHBOR    _IOW('i', 222, struct if_laddrreq)
#define SIOCGPWE3NEIGHBOR    _IOWR('i', 222, struct if_laddrreq)
#define SIOCDPWE3NEIGHBOR    _IOW('i', 222, struct ifreq)










#define    IFXF_MPSAFE        0x1    /* [I] if_start is mpsafe */
#define    IFXF_CLONED        0x2    /* [I] pseudo interface */
#define    IFXF_AUTOCONF6TEMP    0x4    /* [N] v6 temporary addrs enabled */
#define    IFXF_MPLS        0x8    /* [N] supports MPLS */
#define    IFXF_WOL        0x10    /* [N] wake on lan enabled */
#define    IFXF_AUTOCONF6        0x20    /* [N] v6 autoconf enabled */
#define IFXF_INET6_NOSOII    0x40    /* [N] don't do RFC 7217 */
#define    IFXF_AUTOCONF4        0x80    /* [N] v4 autoconf (aka dhcp) enabled */
#define    IFXF_MONITOR        0x100    /* [N] only used for bpf */

#define IF_HDRPRIO_PACKET    -1    /* use mbuf prio */
#define IF_HDRPRIO_PAYLOAD    -2    /* copy payload prio */
#define IF_HDRPRIO_OUTER    -3    /* use outer prio */

#define    IFF_STATICARP    0x20        /* [N] only static ARP */

#define    IFXF_MPSAFE        0x1    /* [I] if_start is mpsafe */
#define    IFXF_CLONED        0x2    /* [I] pseudo interface */
#define    IFXF_AUTOCONF6TEMP    0x4    /* [N] v6 temporary addrs enabled */
#define    IFXF_MPLS        0x8    /* [N] supports MPLS */
#define    IFXF_WOL        0x10    /* [N] wake on lan enabled */
#define    IFXF_AUTOCONF6        0x20    /* [N] v6 autoconf enabled */
#define IFXF_INET6_NOSOII    0x40    /* [N] don't do RFC 7217 */
#define    IFXF_AUTOCONF4        0x80    /* [N] v4 autoconf (aka dhcp) enabled */
#define    IFXF_MONITOR        0x100    /* [N] only used for bpf */


/*
 * Status bit descriptions for the various interface types.
 */
struct if_status_description {
    u_char    ifs_type;
    u_char    ifs_state;
    const char *ifs_string;
};

#define LINK_STATE_DESC_MATCH(_ifs, _t, _s)                \
    (((_ifs)->ifs_type == (_t) || (_ifs)->ifs_type == 0) &&        \
        (_ifs)->ifs_state == (_s))

#define    IFT_IEEE80211           0x47 /* radio spread spectrum    */

#define LINK_STATE_DESCRIPTIONS {                    \
    { IFT_ETHER, LINK_STATE_DOWN, "no carrier" },            \
                                    \
    { IFT_IEEE80211, LINK_STATE_DOWN, "no network" },        \
                                    \
    { IFT_PPP, LINK_STATE_DOWN, "no carrier" },            \
                                    \
    { IFT_CARP, LINK_STATE_DOWN, "backup" },            \
    { IFT_CARP, LINK_STATE_UP, "master" },                \
    { IFT_CARP, LINK_STATE_HALF_DUPLEX, "master" },            \
    { IFT_CARP, LINK_STATE_FULL_DUPLEX, "master" },            \
                                    \
    { 0, LINK_STATE_UP, "active" },                    \
    { 0, LINK_STATE_HALF_DUPLEX, "active" },            \
    { 0, LINK_STATE_FULL_DUPLEX, "active" },            \
                                    \
    { 0, LINK_STATE_UNKNOWN, "unknown" },                \
    { 0, LINK_STATE_INVALID, "invalid" },                \
    { 0, LINK_STATE_DOWN, "down" },                    \
    { 0, LINK_STATE_KALIVE_DOWN, "keepalive down" },        \
    { 0, 0, NULL }                            \
}

#endif /* ifconfig_hpp */
