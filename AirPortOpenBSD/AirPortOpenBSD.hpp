/* add your code here */
#ifndef AirPortOpenBSD_h
#define AirPortOpenBSD_h

#include <Availability.h>

#include "apple80211.h"

#include "compat.h"
#include "iwlwifi.h"

extern struct ifnet *_ifp;

#include "ifconfig.h"

#include "AirPortOpenBSDWLANBusInterfacePCIe.hpp"

enum {
    kIOMessageNetworkChanged,
    kIOMessageScanComplete
};

typedef enum {
    MEDIUM_TYPE_NONE = 0,
    MEDIUM_TYPE_AUTO,
    MEDIUM_TYPE_1MBIT,
    MEDIUM_TYPE_2MBIT,
    MEDIUM_TYPE_5MBIT,
    MEDIUM_TYPE_11MBIT,
    MEDIUM_TYPE_54MBIT,
    MEDIUM_TYPE_300MBIT,
    MEDIUM_TYPE_INVALID
} mediumType_t;

extern "C" {
const char *convertApple80211IOCTLToString(signed int cmd);
}


#define super IOController
#define kTimeoutMS 1000
#define RELEASE(x) if(x){(x)->release();(x)=NULL;}

struct apple80211_scan_result_list {
    SLIST_ENTRY(apple80211_scan_result_list)    list;
    uint64_t asr_age_ts;
    struct apple80211_scan_result scan_result;
};

struct apple80211_ssid_data_known_list {
    SLIST_ENTRY(apple80211_ssid_data_known_list)    list;
    struct apple80211_ssid_data ssid;
};

class AirPortOpenBSDWLANBusInterfacePCIe;

class AirPortOpenBSD : public IOController
{
    OSDeclareDefaultStructors(AirPortOpenBSD)
    
public:
    virtual bool init(OSDictionary *properties) override;
    virtual void free() override;
    virtual IOService* probe(IOService* provider, SInt32* score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual IOReturn enable(IO80211SkywalkInterface *netif) override;
    virtual IOReturn disable(IO80211SkywalkInterface *netif) override;
    virtual IOReturn setHardwareAddress(const void *addr, UInt32 addrBytes) override;
    virtual IOReturn getHardwareAddress(IOEthernetAddress* addrP) override;
    virtual IOReturn getPacketFilters(const OSSymbol *group, UInt32 *filters) const override;
    virtual IOReturn setPromiscuousMode(IOEnetPromiscuousMode mode) override;
    virtual IOReturn setMulticastMode(IOEnetMulticastMode mode) override;
    virtual IOReturn setMulticastList(IOEthernetAddress* addr, UInt32 len) override;
    virtual UInt32 getFeatures() const override;
    virtual const OSString * newVendorString() const override;
    virtual const OSString * newModelString() const override;
    virtual IOReturn selectMedium(const IONetworkMedium *medium) override;
    virtual bool createWorkQueue() override;
    virtual IONetworkInterface * createInterface() override;
    virtual bool configureInterface(IONetworkInterface *netif) override;
    virtual UInt32 outputPacket(mbuf_t, void * param) override;
    virtual IOReturn outputStart(IONetworkInterface *interface, IOOptionBits options) override;
    virtual IOReturn networkInterfaceNotification(
                        IONetworkInterface * interface,
                        uint32_t              type,
                        void *                  argument ) override;
    bool initCCLogs();
    
    virtual IO80211WorkQueue *getWorkQueue() override;
    virtual bool requiresExplicitMBufRelease() override {
        return false;
    }
    virtual bool flowIdSupported() override {
        return false;
    }
    virtual SInt32 monitorModeSetEnabled(bool, UInt) override {
        return kIOReturnSuccess;
    }
    virtual IOReturn requestQueueSizeAndTimeout(unsigned short *queue, unsigned short *timeout) override {
        return kIOReturnSuccess;
    }
    
    virtual bool getLogPipes(CCPipe**, CCPipe**, CCPipe**) override;
    
    virtual void *getFaultReporterFromDriver() override;
    
    virtual SInt32 apple80211_ioctl(IO80211SkywalkInterface *,unsigned long,void *, bool, bool) override;
    virtual SInt32 apple80211SkywalkRequest(UInt,int,IO80211SkywalkInterface *,void *) override;
    virtual SInt32 apple80211SkywalkRequest(UInt,int,IO80211SkywalkInterface *,void *,void *) override;

    virtual SInt32 enableFeature(IO80211FeatureCode, void*) override;
    virtual bool isCommandProhibited(int command) override {
//        if (!ml_at_interrupt_context())
//            DebugLog("%s %s\n", __FUNCTION__, convertApple80211IOCTLToString(command));
        return false;
    };
    virtual SInt32 handleCardSpecific(IO80211SkywalkInterface *,unsigned long,void *,bool) override {
        return 0;
    };
    virtual IOReturn getDRIVER_VERSION(IO80211SkywalkInterface *interface,apple80211_version_data *data) override {
        return getDRIVER_VERSION((OSObject *)interface, data);
    };
    virtual IOReturn getHARDWARE_VERSION(IO80211SkywalkInterface *interface,apple80211_version_data *data) override {
        return getHARDWARE_VERSION((OSObject *)interface, data);
    };
    virtual IOReturn getCARD_CAPABILITIES(IO80211SkywalkInterface *interface,apple80211_capability_data *data) override {
        return getCARD_CAPABILITIES((OSObject *)interface, data);
    }
    virtual IOReturn getPOWER(IO80211SkywalkInterface *interface,apple80211_power_data *data) override {
        return getPOWER((OSObject *)interface, data);
    }
    virtual IOReturn setPOWER(IO80211SkywalkInterface *interface,apple80211_power_data *data) override {
        return setPOWER((OSObject *)interface, data);
    }
    virtual IOReturn getCOUNTRY_CODE(IO80211SkywalkInterface *interface,apple80211_country_code_data *data) override {
        return getCOUNTRY_CODE((OSObject *)interface, data);
    }
    virtual IOReturn setCOUNTRY_CODE(IO80211SkywalkInterface *interface,apple80211_country_code_data *data) override {
        return setCOUNTRY_CODE((OSObject *)interface, data);
    }
    virtual IOReturn setGET_DEBUG_INFO(IO80211SkywalkInterface *interface,apple80211_debug_command *data) override {
        return kIOReturnSuccess;
    }
    
    //-----------------------------------------------------------------------
    // Power management support.
    //-----------------------------------------------------------------------
    virtual IOReturn registerWithPolicyMaker( IOService * policyMaker ) override;
    virtual IOReturn setPowerState( unsigned long powerStateOrdinal,
                                    IOService *   policyMaker) override;

private:
    
#define IOCTL_FUNC(REQ, DATA_TYPE) \
IOCTL_FUNC_GET(REQ, DATA_TYPE) \
IOCTL_FUNC_SET(REQ, DATA_TYPE)
#define IOCTL_FUNC_GET(REQ, DATA_TYPE) \
IOReturn get##REQ(OSObject *object, struct DATA_TYPE *data);
#define IOCTL_FUNC_SET(REQ, DATA_TYPE) \
IOReturn set##REQ(OSObject *object, struct DATA_TYPE *data);
    
    // 12 - CARD_CAPABILITIES
    IOCTL_FUNC_GET(CARD_CAPABILITIES, apple80211_capability_data)
    // 19 - POWER
    IOCTL_FUNC(POWER, apple80211_power_data)
    // 43 - DRIVER_VERSION
    IOCTL_FUNC_GET(DRIVER_VERSION, apple80211_version_data)
    // 44 - HARDWARE_VERSION
    IOCTL_FUNC_GET(HARDWARE_VERSION, apple80211_version_data)
    // 51 - COUNTRY_CODE
    IOCTL_FUNC(COUNTRY_CODE, apple80211_country_code_data)
    
    
public:
    // firmware data
    AirPortOpenBSDWLANBusInterfacePCIe *pciNub;
    int loadfirmware(const char *name, u_char **bufp, size_t *buflen);
    OSData *firmwareData;
    
    // WIFI
    OSString *getNVRAMProperty(char *name);
    static IOReturn tsleepHandler(OSObject* owner, void* arg0, void* arg1, void* arg2, void* arg3);
    bool addMediumType(UInt32 type, UInt32 speed, UInt32 code, char* name = 0);
    void ether_ifattach(struct ifnet *ifp);
    void ether_ifdetach(struct ifnet *ifp);
    
    void if_input(struct ifnet* ifp, struct mbuf_list *ml);
    void if_watchdog(IOTimerEventSource *timer);
    
    void setLinkState(int linkState);
    
    IOPCIDevice *fPciDevice;
    IOWorkLoop *fWorkloop;
    IOCommandGate   *fCommandGate;
    IOTimerEventSource *fWatchdogTimer;
    IOTimerEventSource* fScanSource;
    IONetworkMedium *autoMedium;
    IONetworkMedium *mediumTable[MEDIUM_TYPE_INVALID];
    
    IONetworkStats *netStats;
    IOEthernetStats *etherStats;
    
    OSDictionary* mediumDict;
    struct pci_attach_args *pa;
    struct cfdriver *cd;
    struct cfattach *ca;
    
    void* if_softc;
    struct ieee80211com *ic;
    const char *configArr[256];
    int configArrCount = 0;
    
    struct apple80211_rate_data rate_data;
    struct apple80211_rssi_data rssi_data;
    struct apple80211_noise_data noise_data;
    struct apple80211_country_code_data ccd;
    
    // PM
    unsigned long currentPMPowerLevel = 0;
    
    static IOReturn setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);
    IOReturn changePowerState(OSObject *object, u_int32_t powerState);
    u_int32_t   powerState;
    u_int32_t   powersave_level;
    struct apple80211_antenna_data tx_antenna;
    struct apple80211_antenna_data ad;
    
    bool firstUp = true;
    
    
    // SCAN
    int chanspec2applechannel(int ic_flags, int ic_xflags);
    IOReturn scanConvertResult(struct ieee80211_node *ni, struct apple80211_scan_result *oneResult);
    
    static void apple80211_scan_done(OSObject *owner, IOTimerEventSource *sender);
    IOReturn scanComplete();
    void scanFreeResults(uint64_t asr_age_ts);
    
    SLIST_HEAD(,apple80211_scan_result_list) scan_result_lists = SLIST_HEAD_INITIALIZER(scan_result_lists);
    struct apple80211_scan_result_list *scan_result_next;
    
    // ASSOC apple rsn
    void setPTK(const u_int8_t *key, size_t key_len);
    void setGTK(const u_int8_t *key, size_t key_len, u_int8_t kid, u_int8_t *rsc);
    struct apple80211_assoc_status_data assoc_status_data;
    struct apple80211_key cipher_key;
    char i_psk[256];
    char key_tmp[256];
    u_int32_t current_authtype_lower;
    u_int32_t current_authtype_upper;
    
    // openbsd rsn
    int try_times;
    bool disassoc_times = false;
    bool isConnected();
    bool isRun80211X();
    
public:
    // new
    AirPortOpenBSD_SkywalkInterface* getNetworkInterface(void);
    AirPortOpenBSD_SkywalkInterface *fNetIf;
    bool magicPacketEnabled = false;
    bool magicPacketSupported = false;
    CCPipe *driverLogPipe;
    CCPipe *driverDataPathPipe;
    CCPipe *driverSnapshotsPipe;
    
    CCStream *driverFaultReporter;
    
    IOReturn postMessage(unsigned int, void* data = NULL, unsigned long dataLen = 0) ;
    bool useAppleRSNSupplicant(void *interface);
    UInt32 currentStatus;
    
    int useAppleRSN = 0;
    
};


#endif
