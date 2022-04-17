//
//  __types.h
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/19.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef __types_h
#define __types_h

#include <sys/systm.h>
#include <sys/types.h>
#include <sys/kpi_mbuf.h>
#include <sys/mbuf.h>
#include <sys/cdefs.h>
#include <sys/_endian.h>
#include <sys/param.h>
#include <sys/malloc.h>

#include <libkern/OSTypes.h>
#include <libkern/OSKextLib.h>
#include <libkern/c++/OSObject.h>

#include <IOKit/IONVRAM.h>
#include <IOKit/IOLib.h>

#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOGatedOutputQueue.h>
#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/network/IOPacketQueue.h>
#include <IOKit/network/IONetworkMedium.h>
#include <IOKit/network/IONetworkController.h>
#include <IOKit/network/IOOutputQueue.h>

#include <IOKit/IOService.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IODMACommand.h>
#include <IOKit/IODeviceMemory.h>
#include <IOKit/IODataQueue.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOLocks.h>

#include <IOKit/assert.h>
#include <IOKit/pci/IOPCIDevice.h>

typedef void*                 pci_chipset_tag_t;
typedef IOPCIDevice*        pcitag_t;
typedef uint32_t            pcireg_t;

typedef int paddr_t;
typedef unsigned long        vsize_t;


#define __letoh16(x)    OSSwapLittleToHostInt16(x)
#define __letoh32(x)    OSSwapLittleToHostInt32(x)
#define __letoh64(x)    OSSwapLittleToHostInt64(x)
#define __htole16(x)    OSSwapHostToLittleInt16(x)
#define __htole32(x)    OSSwapHostToLittleInt32(x)
#define __htole64(x)    OSSwapHostToLittleInt64(x)

/* POSIX names */
#define be16toh(x)    OSSwapBigToHostInt16(x)
#define be32toh(x)    OSSwapBigToHostInt32(x)
#define be64toh(x)    OSSwapBigToHostInt64(x)
#define htobe16(x)    OSSwapHostToBigInt16(x)
#define htobe32(x)    OSSwapHostToBigInt32(x)
#define htobe64(x)    OSSwapHostToBigInt64(x)

#define letoh16(x)    __letoh16(x)
#define letoh32(x)    __letoh32(x)
#define letoh64(x)    __letoh64(x)
#define htole16(x)    __htole16(x)
#define htole32(x)    __htole32(x)
#define htole64(x)    __htole64(x)

#define swap16(x)    OSSwapInt16(x)
#define swap32(x)    OSSwapInt32(x)
#define swap64(x)    OSSwapInt64(x)

#define le16toh(x)    letoh16(x)
#define le32toh(x)    letoh32(x)
#define le64toh(x)    letoh64(x)

/* original BSD names */
#define betoh16(x)    be16toh(x)
#define betoh32(x)    be32toh(x)
#define betoh64(x)    be64toh(x)


#define __htolem16(_x, _v)    (*(__uint16_t *)(_x) = __htole16(_v))
#define __htolem32(_x, _v)    (*(__uint32_t *)(_x) = __htole32(_v))
#define __htolem64(_x, _v)    (*(__uint64_t *)(_x) = __htole64(_v))

/* to/from memory conversions */
//#define bemtoh16    __bemtoh16
//#define bemtoh32    __bemtoh32
//#define bemtoh64    __bemtoh64
//#define htobem16    __htobem16
//#define htobem32    __htobem32
//#define htobem64    __htobem64
//#define lemtoh16    __lemtoh16
//#define lemtoh32    __lemtoh32
//#define lemtoh64    __lemtoh64
#define htolem16    __htolem16
#define htolem32    __htolem32
#define htolem64    __htolem64

#endif /* __types_h */
