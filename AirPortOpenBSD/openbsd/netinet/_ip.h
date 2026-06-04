//
//  _ip.h
//  AirPortOpenBSD
//
//  Created by Zhice on 2026/6/4.
//


#ifndef _ip_h
#define _ip_h
#include <netinet/ip.h>


/*
 * Definitions for DiffServ Codepoints as per RFCs 2474, 3246, 4594, 5865, 8622.
 * These are the 6 most significant bits as they appear on the wire, so the
 * two least significant bits must be zero.
 */
#define    IPTOS_DSCP_CS0        0x00
#define    IPTOS_DSCP_LE        0x04
#define    IPTOS_DSCP_CS1        0x20
#define    IPTOS_DSCP_AF11        0x28
#define    IPTOS_DSCP_AF12        0x30
#define    IPTOS_DSCP_AF13        0x38
#define    IPTOS_DSCP_CS2        0x40
#define    IPTOS_DSCP_AF21        0x48
#define    IPTOS_DSCP_AF22        0x50
#define    IPTOS_DSCP_AF23        0x58
#define    IPTOS_DSCP_CS3        0x60
#define    IPTOS_DSCP_AF31        0x68
#define    IPTOS_DSCP_AF32        0x70
#define    IPTOS_DSCP_AF33        0x78
#define    IPTOS_DSCP_CS4        0x80
#define    IPTOS_DSCP_AF41        0x88
#define    IPTOS_DSCP_AF42        0x90
#define    IPTOS_DSCP_AF43        0x98
#define    IPTOS_DSCP_CS5        0xa0
#define    IPTOS_DSCP_VA        0xb0
#define    IPTOS_DSCP_EF        0xb8
#define    IPTOS_DSCP_CS6        0xc0
#define    IPTOS_DSCP_CS7        0xe0


#endif /* _ip_h */
