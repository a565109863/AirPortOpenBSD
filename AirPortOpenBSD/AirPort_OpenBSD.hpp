/* add your code here */
#ifndef AirPort_OpenBSD_h
#define AirPort_OpenBSD_h

#include <Availability.h>

#include "apple80211.h"

#include "compat.h"
#include "iwlwifi.h"
#include <sys/firmware.h>

extern struct ifnet *_ifp;

#include "ifconfig.h"


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


extern int _stop(struct kmod_info*, void*);
extern int _start(struct kmod_info*, void*);

#define kTimeoutMS 1000
#define RELEASE(x) if(x){(x)->release();(x)=NULL;}

struct apple80211_scan_result_list {
    SLIST_ENTRY(apple80211_scan_result_list)    list;
    struct apple80211_scan_result scan_result;
};

class AirPort_OpenBSD : public IOController
{
    OSDeclareDefaultStructors(AirPort_OpenBSD)
    
public:
    bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;
    void free() APPLE_KEXT_OVERRIDE;
    IOService* probe(IOService* provider, SInt32* score) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    IOReturn getHardwareAddress(IOEthernetAddress* addrP) APPLE_KEXT_OVERRIDE;
    IOReturn setHardwareAddress(const IOEthernetAddress * addrP) APPLE_KEXT_OVERRIDE;
    
    /* Power Management Support */
    IOReturn registerWithPolicyMaker(IOService *policyMaker) APPLE_KEXT_OVERRIDE;
    IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker) APPLE_KEXT_OVERRIDE;
//    virtual IOReturn getPacketFilters(const OSSymbol *group, UInt32 *filters) const APPLE_KEXT_OVERRIDE;
//    virtual IOReturn selectMedium(const IONetworkMedium *medium) APPLE_KEXT_OVERRIDE;
    virtual UInt32 getFeatures() const APPLE_KEXT_OVERRIDE;
    
    /* IOController methods. */
    IOReturn enable(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    IOReturn disable(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    UInt32 outputPacket(mbuf_t, void * param) APPLE_KEXT_OVERRIDE;
    IOReturn outputStart(IONetworkInterface *interface, IOOptionBits options) APPLE_KEXT_OVERRIDE;
    IOReturn setPromiscuousMode(IOEnetPromiscuousMode mode) APPLE_KEXT_OVERRIDE;
    IOReturn setMulticastMode(IOEnetMulticastMode mode) APPLE_KEXT_OVERRIDE;
    IOReturn setMulticastList(IOEthernetAddress* addr, UInt32 len) APPLE_KEXT_OVERRIDE;
    bool configureInterface(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    
    virtual IONetworkInterface * createInterface() APPLE_KEXT_OVERRIDE;
    IOReturn getMaxPacketSize(UInt32* maxSize) const APPLE_KEXT_OVERRIDE;
    const OSString* newVendorString() const APPLE_KEXT_OVERRIDE;
    const OSString* newModelString() const APPLE_KEXT_OVERRIDE;
    
public:
    // IO80211
    virtual IOReturn getHardwareAddressForInterface(IO80211Interface* netif,
                                            IOEthernetAddress* addr) APPLE_KEXT_OVERRIDE;
    virtual SInt32 monitorModeSetEnabled(IO80211Interface* interface, bool enabled, UInt32 dlt) APPLE_KEXT_OVERRIDE;
    IOReturn postMessage(unsigned int, void* data = NULL, unsigned long dataLen = 0) ;
    IOReturn apple80211Request(UInt32 request_type, int request_number, IOInterface* interface, void* data) APPLE_KEXT_OVERRIDE;
    int bpfOutputPacket(OSObject *object,UInt,mbuf_t m) APPLE_KEXT_OVERRIDE;
    int outputActionFrame(OSObject *object, mbuf_t m) APPLE_KEXT_OVERRIDE;
    int bpfOutput80211Radio(OSObject *object, mbuf_t m) APPLE_KEXT_OVERRIDE;
    
    int outputRaw80211Packet(IO80211Interface *interface, mbuf_t m) APPLE_KEXT_OVERRIDE;
    virtual bool useAppleRSNSupplicant(IO80211Interface *interface) APPLE_KEXT_OVERRIDE;
    virtual bool useAppleRSNSupplicant(IO80211VirtualInterface* interface) APPLE_KEXT_OVERRIDE;
    
    
//    virtual bool setLinkStatus(UInt32 status, const IONetworkMedium *activeMedium = 0, UInt64 speed = 0, OSData *data = 0) APPLE_KEXT_OVERRIDE;
    
//    static IOReturn setLinkStateGated(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
    
    //virtual interface
    virtual SInt32 enableVirtualInterface(IO80211VirtualInterface *interface) APPLE_KEXT_OVERRIDE;
    virtual SInt32 disableVirtualInterface(IO80211VirtualInterface *interface) APPLE_KEXT_OVERRIDE;
    virtual IO80211VirtualInterface* createVirtualInterface(ether_addr *eth,uint role) APPLE_KEXT_OVERRIDE;
    virtual SInt32 apple80211VirtualRequest(uint request_type, int request_number,IO80211VirtualInterface *interface,void *data) APPLE_KEXT_OVERRIDE;
    
    virtual SInt32 stopDMA() APPLE_KEXT_OVERRIDE;
    virtual UInt32 hardwareOutputQueueDepth(IO80211Interface* interface) APPLE_KEXT_OVERRIDE;
    virtual SInt32 performCountryCodeOperation(IO80211Interface* interface, IO80211CountryCodeOp op) APPLE_KEXT_OVERRIDE;
    virtual SInt32 enableFeature(IO80211FeatureCode code, void* data) APPLE_KEXT_OVERRIDE;
    
    
//    virtual bool createWorkLoop() APPLE_KEXT_OVERRIDE;
//    virtual IOWorkLoop* getWorkLoop() const APPLE_KEXT_OVERRIDE;
//
//    virtual void requestPacketTx(void*, UInt) APPLE_KEXT_OVERRIDE;
    
//    void watchdogAction(IOTimerEventSource *timer);
//    bool initPCIPowerManagment(IOPCIDevice *provider);

private:
    
#define IOCTL_FUNC(REQ, DATA_TYPE) \
IOCTL_FUNC_GET(REQ, DATA_TYPE) \
IOCTL_FUNC_SET(REQ, DATA_TYPE)
#define IOCTL_FUNC_GET(REQ, DATA_TYPE) \
IOReturn get##REQ(OSObject *object, struct DATA_TYPE *data);
#define IOCTL_FUNC_SET(REQ, DATA_TYPE) \
IOReturn set##REQ(OSObject *object, struct DATA_TYPE *data);
    
    // 1 - SSID
    IOCTL_FUNC(SSID, apple80211_ssid_data)
    // 2 - AUTH_TYPE
    IOCTL_FUNC(AUTH_TYPE, apple80211_authtype_data)
    // 3 - CIPHER_KEY
    IOCTL_FUNC_SET(CIPHER_KEY, apple80211_key)
    // 4 - CHANNEL
    IOCTL_FUNC(CHANNEL, apple80211_channel_data)
    // 5 - POWERSAVE
    IOCTL_FUNC(POWERSAVE, apple80211_powersave_data)
    // 6 - PROTMODE
    IOCTL_FUNC_GET(PROTMODE, apple80211_protmode_data)
    // 7 - TXPOWER
    IOCTL_FUNC(TXPOWER, apple80211_txpower_data)
    // 8 - RATE
    IOCTL_FUNC_GET(RATE, apple80211_rate_data)
    // 9 - BSSID
    IOCTL_FUNC(BSSID, apple80211_bssid_data)
    // 10 - SCAN_REQ
    IOCTL_FUNC_SET(SCAN_REQ, apple80211_scan_data)
    // 11 - SCAN_RESULT
    IOCTL_FUNC_GET(SCAN_RESULT, apple80211_scan_result*)
    // 12 - CARD_CAPABILITIES
    IOCTL_FUNC_GET(CARD_CAPABILITIES, apple80211_capability_data)
    // 13 - STATE
    IOCTL_FUNC_GET(STATE, apple80211_state_data)
    // 14 - PHY_MODE
    IOCTL_FUNC_GET(PHY_MODE, apple80211_phymode_data)
    // 15 - OP_MODE
    IOCTL_FUNC_GET(OP_MODE, apple80211_opmode_data)
    // 16 - RSSI
    IOCTL_FUNC_GET(RSSI, apple80211_rssi_data)
    // 17 - NOISE
    IOCTL_FUNC_GET(NOISE, apple80211_noise_data)
    // 18 - INT_MIT
    IOCTL_FUNC_GET(INT_MIT, apple80211_intmit_data)
    // 19 - POWER
    IOCTL_FUNC(POWER, apple80211_power_data)
    // 20 - ASSOCIATE
    IOCTL_FUNC_SET(ASSOCIATE, apple80211_assoc_data)
    // 21 - ASSOCIATE_RESULT
    IOCTL_FUNC_GET(ASSOCIATE_RESULT, apple80211_assoc_result_data)
    // 22 - DISASSOCIATE
    IOReturn setDISASSOCIATE(OSObject *object);
//    // 23 - STATUS_DEV_NAME
//    IOCTL_FUNC_GET(STATUS_DEV_NAME, apple80211_status_dev_data)
    // 27 - SUPPORTED_CHANNELS
    IOCTL_FUNC_GET(SUPPORTED_CHANNELS, apple80211_sup_channel_data)
    // 28 - LOCALE
    IOCTL_FUNC_GET(LOCALE, apple80211_locale_data)
    // 29 - DEAUTH
    IOCTL_FUNC(DEAUTH, apple80211_deauth_data)
//    // 31 - FRAG_THRESHOLD
//    IOCTL_FUNC_GET(FRAG_THRESHOLD, apple80211_frag_threshold_data)
    // 32 - RATE_SET
    IOCTL_FUNC_GET(RATE_SET, apple80211_rate_set_data)
    // 37 - TX_ANTENNA
    IOCTL_FUNC(TX_ANTENNA, apple80211_antenna_data)
    // 39 - ANTENNA_DIVERSITY
    IOCTL_FUNC(ANTENNA_DIVERSITY, apple80211_antenna_data)
//    // 40 - ROM
//    IOCTL_FUNC_GET(ROM, apple80211_rom_data)
//    // 42 - STATION_LIST
//    IOCTL_FUNC_GET(STATION_LIST, apple80211_sta_data)
    // 43 - DRIVER_VERSION
    IOCTL_FUNC_GET(DRIVER_VERSION, apple80211_version_data)
    // 44 - HARDWARE_VERSION
    IOCTL_FUNC_GET(HARDWARE_VERSION, apple80211_version_data)
    // 46 - RSN_IE
    IOCTL_FUNC(RSN_IE, apple80211_rsn_ie_data)
    // 48 AP_IE_LIST
    IOCTL_FUNC_GET(AP_IE_LIST, apple80211_ap_ie_data)
    // 49 AP_IE_LIST
    IOCTL_FUNC_GET(STATS, apple80211_stats_data)
    // 50 - ASSOCIATION_STATUS
    IOCTL_FUNC(ASSOCIATION_STATUS, apple80211_assoc_status_data)
    // 51 - COUNTRY_CODE
    IOCTL_FUNC(COUNTRY_CODE, apple80211_country_code_data)
//    // 53 - LAST_RX_PKT_DATA
//    IOCTL_FUNC_GET(LAST_RX_PKT_DATA, apple80211_last_rx_pkt_data)
    // 54 - RADIO_INFO
    IOCTL_FUNC_GET(RADIO_INFO, apple80211_radio_info_data)
    // 57 - MCS
    IOCTL_FUNC_GET(MCS, apple80211_mcs_data)
    // 65 - PHY_SUB_MODE
    IOCTL_FUNC_GET(PHY_SUB_MODE, apple80211_physubmode_data)
    // 66 - MCS_INDEX_SET
    IOCTL_FUNC_GET(MCS_INDEX_SET, apple80211_mcs_index_set_data)
    // 80 - ROAM_THRESH
    IOCTL_FUNC_GET(ROAM_THRESH, apple80211_roam_threshold_data)
    // 85 - IE
    IOCTL_FUNC(IE, apple80211_rsn_ie_data)
    // 86 - SCAN_REQ_MULTIPLE
    IOCTL_FUNC_SET(SCAN_REQ_MULTIPLE, apple80211_scan_multiple_data)
    // 90 - SCANCACHE_CLEAR
    IOCTL_FUNC_SET(SCANCACHE_CLEAR, device)
    // 94 - VIRTUAL_IF_CREATE
    IOCTL_FUNC_SET(VIRTUAL_IF_CREATE, apple80211_virt_if_create_data)
    // 95 - VIRTUAL_IF_DELETE
    IOCTL_FUNC_SET(VIRTUAL_IF_DELETE, apple80211_virt_if_delete_data)
    // 107 - ROAM
    IOCTL_FUNC_SET(ROAM, apple80211_sta_roam_data)
//    // 112 - FACTORY_MODE
//    IOCTL_FUNC_GET(FACTORY_MODE, apple80211_factory_mode_data)
    // 156 - LINK_CHANGED_EVENT_DATA
    IOCTL_FUNC_GET(LINK_CHANGED_EVENT_DATA, apple80211_link_changed_event_data)
    // 181 - VHT_MCS_INDEX_SET
    IOCTL_FUNC_GET(VHT_MCS_INDEX_SET, apple80211_vht_mcs_index_set_data)
    // 195 - MCS_VHT
    IOCTL_FUNC(MCS_VHT, apple80211_mcs_vht_data)
    // 196 - TX_NSS
    IOCTL_FUNC(TX_NSS, apple80211_tx_nss_data)
    // 216 - ROAM_PROFILE
    IOCTL_FUNC(ROAM_PROFILE, apple80211_roam_profile_band_data)
    // 221 - BTCOEX_PROFILES
    IOCTL_FUNC(BTCOEX_PROFILES, apple80211_btc_profiles_data)
    // 222 - BTCOEX_CONFIG
    IOCTL_FUNC(BTCOEX_CONFIG, apple80211_btc_config_data)
    // 235 - BTCOEX_OPTIONS
    IOCTL_FUNC(BTCOEX_OPTIONS, apple80211_btc_options_data)
    // 87 - BTCOEX_MODE
    IOCTL_FUNC(BTCOEX_MODE, apple80211_btc_mode_data)
    // 353 - NSS
    IOCTL_FUNC_GET(NSS, apple80211_nss_data)
    
    //AirportVirtualIOCTL
    IOCTL_FUNC(AWDL_PEER_TRAFFIC_REGISTRATION, apple80211_awdl_peer_traffic_registration)
    IOCTL_FUNC(AWDL_ELECTION_METRIC, apple80211_awdl_election_metric)
    IOCTL_FUNC(SYNC_ENABLED, apple80211_awdl_sync_enabled)
    IOCTL_FUNC(SYNC_FRAME_TEMPLATE, apple80211_awdl_sync_frame_template)
    IOCTL_FUNC_GET(AWDL_HT_CAPABILITY, apple80211_ht_capability)
    IOCTL_FUNC_GET(AWDL_VHT_CAPABILITY, apple80211_vht_capability)
    
    //AWDL
    IOCTL_FUNC(AWDL_BSSID, apple80211_awdl_bssid)
    IOCTL_FUNC_GET(CHANNELS_INFO, apple80211_channels_info)
    IOCTL_FUNC(PEER_CACHE_MAXIMUM_SIZE, apple80211_peer_cache_maximum_size)
    IOCTL_FUNC(AWDL_ELECTION_ID, apple80211_awdl_election_id)
    IOCTL_FUNC(AWDL_MASTER_CHANNEL, apple80211_awdl_master_channel)
    IOCTL_FUNC(AWDL_SECONDARY_MASTER_CHANNEL, apple80211_awdl_secondary_master_channel)
    IOCTL_FUNC(AWDL_MIN_RATE, apple80211_awdl_min_rate)
    IOCTL_FUNC(AWDL_ELECTION_RSSI_THRESHOLDS, apple80211_awdl_election_rssi_thresholds)
    IOCTL_FUNC(AWDL_SYNCHRONIZATION_CHANNEL_SEQUENCE, apple80211_awdl_sync_channel_sequence)
    IOCTL_FUNC(AWDL_PRESENCE_MODE, apple80211_awdl_presence_mode)
    IOCTL_FUNC(AWDL_EXTENSION_STATE_MACHINE_PARAMETERS, apple80211_awdl_extension_state_machine_parameter)
    IOCTL_FUNC(AWDL_SYNC_STATE, apple80211_awdl_sync_state)
    IOCTL_FUNC(AWDL_SYNC_PARAMS, apple80211_awdl_sync_params)
    IOCTL_FUNC_GET(AWDL_CAPABILITIES, apple80211_awdl_cap)
    IOCTL_FUNC(AWDL_AF_TX_MODE, apple80211_awdl_af_tx_mode)
    IOCTL_FUNC_SET(AWDL_OOB_AUTO_REQUEST, apple80211_awdl_oob_request)
    IOCTL_FUNC(WOW_PARAMETERS, apple80211_wow_parameter_data)
    IOCTL_FUNC(IE, apple80211_ie_data)
    IOCTL_FUNC_SET(P2P_SCAN, apple80211_scan_data)
    IOCTL_FUNC_SET(P2P_LISTEN, apple80211_p2p_listen_data)
    IOCTL_FUNC_SET(P2P_GO_CONF, apple80211_p2p_go_conf_data)
    
    
public:
    // firmware
    void firmwareLoadComplete(const char* name);
    int loadfirmware(const char *name, u_char **bufp, size_t *buflen);
    static void firmwareLoadComplete(OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context);
    IOLock *fwLoadLock;
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
    IO80211WorkLoop *fWorkloop;
    IOWorkLoop *fWatchdogWorkLoop;
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
    static IOReturn setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);
    IOReturn changePowerState(OSObject *object, int powerState);
    unsigned long powerState;
    u_int32_t    powersave_level;
    struct apple80211_antenna_data tx_antenna;
    struct apple80211_antenna_data ad;
    
    bool firstUp = true;
    
    
    // SCAN
    int chanspec2applechannel(int ic_flags, int ic_xflags);
    IOReturn scanConvertResult(struct ieee80211_nodereq *nr, struct apple80211_scan_result* oneResult);
    
    static void apple80211_scan_done(OSObject *owner, IOTimerEventSource *sender);
    void scanComplete();
    void scanFreeResults();
    
    SLIST_HEAD(,apple80211_scan_result_list) scan_result_lists = SLIST_HEAD_INITIALIZER(scan_result_lists);
    struct apple80211_scan_result_list *scan_result_next;
    uint32_t scanResultsCount = 0;
    
    
    // ASSOC
    void setPTK(const u_int8_t *key, size_t key_len);
    void setGTK(const u_int8_t *key, size_t key_len, u_int8_t kid, u_int8_t *rsc);
    struct apple80211_authtype_data authtype_data;
    struct apple80211_bssid_data bssid_data;
    struct apple80211_rsn_ie_data rsn_ie_data;
//    struct apple80211_assoc_data assoc_data;
    struct apple80211_assoc_status_data assoc_status_data;
    struct apple80211_key cipher_key;
    char i_psk[256];
    char key_tmp[256];
    u_int32_t current_authtype_lower;
    u_int32_t current_authtype_upper;
    
public:
    
    // P2P
    IOReturn getVirtIf(OSObject *object);

    IO80211P2PInterface *fP2PDISCInterface;
    IO80211P2PInterface *fP2PGOInterface;
    IO80211P2PInterface *fAWDLInterface;

    //AWDL
    uint8_t *syncFrameTemplate;
    uint32_t syncFrameTemplateLength;
    uint8_t awdlBSSID[6];
    uint32_t awdlSyncState;
    uint32_t awdlElectionId;
    uint32_t awdlPresenceMode;
    uint16_t awdlMasterChannel;
    uint16_t awdlSecondaryMasterChannel;
    uint8_t *roamProfile;
    struct apple80211_btc_profiles_data *btcProfile;
    struct apple80211_btc_config_data btcConfig;
    uint32_t btcMode;
    uint32_t btcOptions;
    bool awdlSyncEnable;

};


#endif
