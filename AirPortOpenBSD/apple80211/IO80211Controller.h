#ifndef _IO80211CONTROLLER_H
#define _IO80211CONTROLLER_H

#if defined(KERNEL) && defined(__cplusplus)

#include <libkern/version.h>

#if VERSION_MAJOR > 8
	#define _MODERN_BPF
#endif

#include <sys/kpi_mbuf.h>

#include <IOKit/network/IOEthernetController.h>

#include <sys/param.h>
#include <net/bpf.h>

#include "apple80211_ioctl.h"
#ifdef CATALINA
#include "IO80211SkywalkInterface.h"
#endif
#include "IO80211WorkLoop.h"

#define AUTH_TIMEOUT            15    // seconds

/*! @enum LinkSpeed.
 @abstract ???.
 @discussion ???.
 @constant LINK_SPEED_80211A 54 Mbps
 @constant LINK_SPEED_80211B 11 Mbps.
 @constant LINK_SPEED_80211G 54 Mbps.
 */
enum {
    LINK_SPEED_80211A    = 54000000ul,        // 54 Mbps
    LINK_SPEED_80211B    = 11000000ul,        // 11 Mbps
    LINK_SPEED_80211G    = 54000000ul,        // 54 Mbps
    LINK_SPEED_80211N    = 300000000ul,        // 300 Mbps (MCS index 15, 400ns GI, 40 MHz channel)
};

enum IO80211CountryCodeOp
{
    kIO80211CountryCodeReset,                // Reset country code to world wide default, and start
    // searching for 802.11d beacon
};
typedef enum IO80211CountryCodeOp IO80211CountryCodeOp;

enum IO80211SystemPowerState
{
    kIO80211SystemPowerStateUnknown,
    kIO80211SystemPowerStateAwake,
    kIO80211SystemPowerStateSleeping,
};
typedef enum IO80211SystemPowerState IO80211SystemPowerState;

enum IO80211FeatureCode
{
    kIO80211Feature80211n = 1,
};
typedef enum IO80211FeatureCode IO80211FeatureCode;

#ifdef CATALINA
class IOSkywalkInterface;
#endif
class IO80211ScanManager;
enum CCStreamLogLevel
{
    LEVEL_1,
};

enum scanSource
{
    SOURCE_1,
};

enum joinStatus
{
    STATUS_1,
};

class IO80211Controller;
class IO80211Interface;
class IO82110WorkLoop;
class IO80211VirtualInterface;
class IO80211ControllerMonitor;
class CCLogPipe;
class CCIOReporterLogStream;
class CCLogStream;
class IO80211VirtualInterface;
class IO80211RangingManager;
class IO80211FlowQueue;
class IO80211FlowQueueLegacy;
class FlowIdMetadata;
class IOReporter;
extern void IO80211VirtualInterfaceNamerRetain();


struct apple80211_hostap_state;

struct apple80211_awdl_sync_channel_sequence;
struct ieee80211_ht_capability_ie;
struct apple80211_channel_switch_announcement;
struct apple80211_beacon_period_data;
struct apple80211_power_debug_sub_info;
struct apple80211_stat_report;
struct apple80211_frame_counters;
struct apple80211_leaky_ap_event;
struct apple80211_chip_stats;
struct apple80211_extended_stats;
struct apple80211_ampdu_stat_report;
struct apple80211_btCoex_report;
struct apple80211_cca_report;
class CCPipe;
struct apple80211_lteCoex_report;

typedef IOReturn (*IOCTL_FUNC)(IO80211Controller*, IO80211Interface*, IO80211VirtualInterface*, apple80211req*, bool);
extern IOCTL_FUNC gGetHandlerTable[];
extern IOCTL_FUNC gSetHandlerTable[];

#define __int64 int
#define ulong unsigned long
#define _QWORD UInt64
#define uint UInt

class IO80211Controller : public IOEthernetController {
    OSDeclareAbstractStructors(IO80211Controller)
    
public:
    
    virtual bool init(OSDictionary *) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn configureReport(IOReportChannelList *,uint,void *,void *) APPLE_KEXT_OVERRIDE;
    virtual IOReturn updateReport(IOReportChannelList *,uint,void *,void *) APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService *) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService *) APPLE_KEXT_OVERRIDE;
    
    virtual IOService* getProvider(void) const APPLE_KEXT_OVERRIDE;
    virtual IOWorkLoop* getWorkLoop(void) const APPLE_KEXT_OVERRIDE;
    virtual const char* stringFromReturn(int) APPLE_KEXT_OVERRIDE;
    virtual int errnoFromReturn(int) APPLE_KEXT_OVERRIDE;
    virtual IOOutputQueue* getOutputQueue(void) const APPLE_KEXT_OVERRIDE;
    virtual bool createWorkLoop(void) APPLE_KEXT_OVERRIDE;
    virtual IOReturn enable(IONetworkInterface *) APPLE_KEXT_OVERRIDE;
    virtual IOReturn disable(IONetworkInterface *) APPLE_KEXT_OVERRIDE;
    virtual bool attachInterface(IONetworkInterface **, bool attach = true) APPLE_KEXT_OVERRIDE;
    virtual IONetworkInterface* createInterface(void) APPLE_KEXT_OVERRIDE;
    virtual bool configureInterface(IONetworkInterface *) APPLE_KEXT_OVERRIDE;
    virtual IOReturn outputStart(IONetworkInterface *,uint) APPLE_KEXT_OVERRIDE;
    virtual IOReturn getHardwareAddress(IOEthernetAddress *) APPLE_KEXT_OVERRIDE;
    virtual void requestPacketTx(void*, uint);
    virtual IOReturn getHardwareAddressForInterface(IO80211Interface *,IOEthernetAddress *);
    virtual void inputMonitorPacket(mbuf_t,uint,void *,ulong);
    virtual int outputRaw80211Packet(IO80211Interface *,mbuf_t);
    virtual int outputActionFrame(IO80211Interface *,mbuf_t);
    virtual int bpfOutputPacket(IO80211Interface *,uint,mbuf_t);
    virtual SInt32 monitorModeSetEnabled(IO80211Interface*, bool, uint) {
        return 0;
    }
    virtual IO80211Interface* getNetworkInterface(void);
#ifdef CATALINA
    virtual IO80211SkywalkInterface* getPrimarySkywalkInterface(void);
#endif
    virtual SInt32 apple80211_ioctl(IO80211Interface *, IO80211VirtualInterface*, ifnet_t,ulong,void *);
    virtual SInt32 apple80211_ioctl(IO80211Interface *, ifnet_t,ulong id,void *) {
        IOLog("Black80211: ioctl called with %lx", id);
        return 0;
    }
#ifdef CATALINA
    virtual SInt32 apple80211_ioctl(IO80211SkywalkInterface *,ulong,void *);
#endif
    
    virtual SInt32 apple80211Request(uint, int, IO80211Interface*, void*) = 0;
    virtual SInt32 apple80211VirtualRequest(uint,int,IO80211VirtualInterface *,void *);
#ifdef CATALINA
    virtual SInt32 apple80211SkywalkRequest(uint,int,IO80211SkywalkInterface *,void *);
#endif
    
    virtual SInt32 stopDMA() { return 0x66; };
    virtual UInt32 hardwareOutputQueueDepth(IO80211Interface*) { return 0; };
    virtual SInt32 performCountryCodeOperation(IO80211Interface*, IO80211CountryCodeOp) { return 0; };
    virtual bool useAppleRSNSupplicant(IO80211Interface *);
    virtual bool useAppleRSNSupplicant(IO80211VirtualInterface *);
    virtual void dataLinkLayerAttachComplete(IO80211Interface *);
    virtual SInt32 enableFeature(IO80211FeatureCode, void*) { return 0; };
    virtual SInt32 setVirtualHardwareAddress(IO80211VirtualInterface *,ether_addr *);
    virtual SInt32 enableVirtualInterface(IO80211VirtualInterface *);
    virtual SInt32 disableVirtualInterface(IO80211VirtualInterface *);
    virtual IOReturn requiresExplicitMBufRelease() { return 0; };
    virtual IOReturn flowIdSupported() { return 0; };
    virtual IO80211FlowQueueLegacy* requestFlowQueue(FlowIdMetadata const*);
    virtual void releaseFlowQueue(IO80211FlowQueue *);
#ifdef CATALINA
    virtual void getLogPipes(CCPipe**, CCPipe**, CCPipe**) {};
#endif
    virtual IOReturn enablePacketTimestamping(void) { return 0; };
    virtual IOReturn disablePacketTimestamping(void) { return 0; };
    virtual UInt32 selfDiagnosticsReport(int,char const*,uint);
    virtual UInt32 getDataQueueDepth(OSObject *);
#ifdef BIG_SUR
    virtual bool isAssociatedToMovingNetwork(void) { return false; }
#endif
    virtual mbuf_flags_t inputPacket(mbuf_t);

#ifdef CATALINA
    SInt32 apple80211_ioctl_get(IO80211Interface *,IO80211VirtualInterface *,IO80211SkywalkInterface *,void *);
#endif
    
    virtual SInt32 apple80211_ioctl_get(IO80211Interface *,IO80211VirtualInterface *,ifnet_t,void *);
    
#ifdef CATALINA
    SInt32 apple80211_ioctl_get(IO80211SkywalkInterface *,void *);
#endif
#ifdef CATALINA
    virtual SInt32 apple80211_ioctl_set(IO80211Interface *,IO80211VirtualInterface *,IO80211SkywalkInterface *,void *);
#endif
    
    virtual SInt32 apple80211_ioctl_set(IO80211Interface *,IO80211VirtualInterface *,ifnet_t,void *);
    
#ifdef CATALINA
    virtual SInt32 apple80211_ioctl_set(IO80211SkywalkInterface *,void*);
#endif
#ifdef CATALINA
    virtual bool attachInterface(IOSkywalkInterface *,IOService *);

#ifdef BIG_SUR
    virtual bool detachInterface(IOSkywalkInterface *, bool);
#endif
    
    virtual IOReturn enable(IO80211SkywalkInterface *);
    virtual IOReturn disable(IO80211SkywalkInterface *);
    IO80211SkywalkInterface* getInfraInterface(void);
    SInt32 getASSOCIATE_RESULT(IO80211Interface *,IO80211VirtualInterface *,IO80211SkywalkInterface *,apple80211_assoc_result_data *);
#endif
    
    virtual IO80211VirtualInterface* createVirtualInterface(ether_addr *,uint);
    virtual bool attachVirtualInterface(IO80211VirtualInterface **,ether_addr *,uint,bool);
    virtual bool detachVirtualInterface(IO80211VirtualInterface *,bool);
    
    
    virtual void detachInterface(IONetworkInterface *, bool sync = false) APPLE_KEXT_OVERRIDE;
    
    IO80211ScanManager* getPrimaryInterfaceScanManager(void);
    
    IO80211ControllerMonitor* getInterfaceMonitor(void);
    
    
    IOReturn addReporterLegend(IOService *,IOReporter *,char const*,char const*);
    IOReturn removeReporterFromLegend(IOService *,IOReporter *,char const*,char const*);
    IOReturn unlockIOReporterLegend(void);
    void lockIOReporterLegend(void);//怀疑对象，之前是返回int
    IOReturn logIOReportLogStreamSubscription(ulong long);
    IOReturn addIOReportLogStreamForProvider(IOService *,ulong long *);
    IOReturn addSubscriptionForThisReporterFetchedOnTimer(IOReporter *,char const*,char const*,IOService *) ;
    IOReturn addSubscriptionForProviderFetchedOnTimer(IOService *);
    void handleIOReporterTimer(IOTimerEventSource *);
    void setIOReportersStreamFlags(ulong long);
    void updateIOReportersStreamFrequency(void); //怀疑对象，之前是返回int
    void setIOReportersStreamLevel(CCStreamLogLevel);


    void powerChangeGated(OSObject *,void *,void *,void *,void *) {};
    int copyOut(void const*,ulong long,ulong);
    
    SInt32 getASSOCIATE_RESULT(IO80211Interface*, IO80211VirtualInterface*, apple80211_assoc_result_data*);

    IOReturn copyIn(ulong long,void *,ulong);
    void logIOCTL(apple80211req *) {};
    bool isIOCTLLoggingRestricted(apple80211req *);
    
    bool getBeaconPeriod(apple80211_beacon_period_data *);
    
    SInt32 apple80211VirtualRequestIoctl(uint,int,IO80211VirtualInterface *,void *);
    bool getBSSIDData(OSObject *,apple80211_bssid_data *);
    bool getSSIDData(apple80211_ssid_data *);
    bool inputInfraPacket(mbuf_t);
    void notifyHostapState(apple80211_hostap_state *) {};
    bool isAwdlAssistedDiscoveryEnabled(void);
    void joinDone(scanSource,joinStatus) {};
    void joinStarted(scanSource,joinStatus) {};
    void handleChannelSwitchAnnouncement(apple80211_channel_switch_announcement *) {};
    void scanDone(scanSource,int) {};
    void scanStarted(scanSource,apple80211_scan_data *) {};
    void printChannels(void) {};
    void updateInterfaceCoexRiskPct(ulong long) {};
    SInt32 getInfraChannel(apple80211_channel_data *);
    void calculateInterfacesAvaiability(void);//怀疑对象，之前是返回int
    void setChannelSequenceList(apple80211_awdl_sync_channel_sequence *) {};//怀疑对象，之前是返回int
    void setPrimaryInterfaceDatapathState(bool) {};
    UInt32 getPrimaryInterfaceLinkState(void);
    void setCurrentChannel(apple80211_channel *);//怀疑对象，之前是返回int
    void setHtCapability(ieee80211_ht_capability_ie *) {};
    UInt32 getHtCapability(void);
    UInt32 getHtCapabilityLength(void);
    bool io80211isDebuggable(bool* enable);
    void logDebug(ulong long,char const*,...);//怀疑对象，之前是返回int
    void vlogDebug(ulong long,char const*,va_list);//怀疑对象，之前是返回char
    void logDebug(char const*,...);//怀疑对象，之前是返回int

    bool calculateInterfacesCoex(void);
    void setInfraChannel(apple80211_channel *);

    void configureAntennae(void) {};
    SInt32 apple80211RequestIoctl(uint,int,IO80211Interface *,void *);
    UInt32 radioCountForInterface(IO80211Interface *);
    void releaseIOReporters(void);
    bool findAndAttachToFaultReporter(void) {
        return false;
    }
    UInt32 setupControlPathLogging(void);
    IOReturn createIOReporters(IOService *);
    IOReturn powerChangeHandler(void *,void *,uint,IOService *,void *,ulong);
    
        
    OSMetaClassDeclareReservedUnused( IO80211Controller,  0);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  1);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  2);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  3);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  4);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  5);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  6);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  7);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  8);
    OSMetaClassDeclareReservedUnused( IO80211Controller,  9);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 10);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 11);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 12);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 13);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 14);
    OSMetaClassDeclareReservedUnused( IO80211Controller, 15);
    
protected:
    static IORegistryPlane gIO80211Plane;
    static IORegistryEntry* kIO80211PlaneName;
    //0x118
    IOTimerEventSource * _report_gathering_timer; // 0x118
    OSArray * _reporter_num;                 // 0x120 OSArray of OSNumber
    UInt32 _var_128;                   // timeout ticks
    bool _wan_debug_enable;            // 0x12c
                                       // 3 bytes padding
    UInt32 _debug_value;               // 0x130
    IORecursiveLock * _recursive_lock; // 0x138
    UInt64 _ht_cap_0x0;                  // 0x140
    UInt64 _ht_cap_0x8;                // 0x148
    UInt64 _ht_cap_0x10;                // 0x150
    UInt32 _ht_cap_0x18;                // 0x158
    UInt32 _ht_cap_len;                // 0x15c
    IO80211ControllerMonitor * _fControllerMonitor; // 0x160
    CCLogPipe * _fControllerIOReporterPipe;             // 0x168
    CCIOReporterLogStream * _fControllerIOReporterStream; // 0x170
    CCLogPipe * _controlPathLogPipe;       // 0x180
    CCLogStream * _ioctlLogStream;        // 0x188
    CCLogStream *  _eventLogStream;      // 0x190
    IO82110WorkLoop * _workLoop;       // 0x198
    IO80211Interface * _interface;     // 0x1a0
    IO80211VirtualInterface * _v_interface; // 0x1a8
    IO80211VirtualInterface (* _vir_interface)[4]; // 0x1b0
    UInt64 _vlog_debug ;                  // 0x1d0 vlogDebug ?
    UInt32 _unknown;                    // 0x1d8
    UInt32 _infra_channel;  // 0x1dc compared with offet 8 of apple80211_stat_report  IO80211Controller::setChanCCA(apple80211_stat_report*, int)
    UInt32 _infra_channel_flags;  // 0x1e0 compared with offet 8 of apple80211_channel
    UInt32 _current_channel;   // 0x1e8 loaded with offet 04 of apple80211_channel
    UInt32 _current_channel_fags;   // 0x1ec loaded with offet 08 of apple80211_channel
    UInt8 _awdl_sync[0x190]; // 0x1f0, 0x190 bytes apple80211_awdl_sync_channel_sequence
    IONotifier * _powerDownNotifier;          // 0x380
    IOService  * _provider;            // 0x388
    IO80211RangingManager * _ranger_manager; // 0x390
    bool       _var_398;               // 0x398 checked in IO80211Controller::disable(IONetworkInterface*)
                                       // 7 byte padding
    IONotifier * _notifier1;           // 0x3a0
    bool         _var_3a8;             // 0x3a8
                                       // 7 byte padding
    UInt64       _last_pointer;        // 0x3b0  unused
    uint8_t filler[0x2B2];
    //0x3CA
};

#endif /* defined(KERNEL) && defined(__cplusplus) */

#endif /* !_IO80211CONTROLLER_H */
