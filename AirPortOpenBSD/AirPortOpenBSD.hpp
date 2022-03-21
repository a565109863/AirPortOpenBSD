/* add your code here */
#ifndef AirPortOpenBSD_h
#define AirPortOpenBSD_h

#define __PRIVATE_SPI__

typedef unsigned int ifnet_ctl_cmd_t;
#include "IONetworkInterface.h"
#include "IONetworkController.h"

#include "apple80211.h"

#include "compat.h"
#include "iwlwifi.h"
#include "firmware/firmware.h"

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

class AirPortOpenBSD : public IOController
{
    OSDeclareDefaultStructors(AirPortOpenBSD)
    
public:
    bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;
    void free() APPLE_KEXT_OVERRIDE;
    IOService* probe(IOService* provider, SInt32* score) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    IOReturn getHardwareAddress(IOEthernetAddress* addrP) APPLE_KEXT_OVERRIDE;
    
    /* Power Management Support */
    IOReturn registerWithPolicyMaker(IOService *policyMaker) APPLE_KEXT_OVERRIDE;
    IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker) APPLE_KEXT_OVERRIDE;
    
    /* IOController methods. */
    IOReturn enable(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    IOReturn disable(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    UInt32 outputPacket(mbuf_t, void * param) APPLE_KEXT_OVERRIDE;
    IOReturn outputStart(IONetworkInterface *interface, IOOptionBits options) APPLE_KEXT_OVERRIDE;
    IOReturn setPromiscuousMode(IOEnetPromiscuousMode mode) APPLE_KEXT_OVERRIDE;
    IOReturn setMulticastMode(IOEnetMulticastMode mode) APPLE_KEXT_OVERRIDE;
    IOReturn setMulticastList(IOEthernetAddress* addr, UInt32 len) APPLE_KEXT_OVERRIDE;
    bool configureInterface(IONetworkInterface *netif) APPLE_KEXT_OVERRIDE;
    
    IOReturn getMaxPacketSize(UInt32* maxSize) const APPLE_KEXT_OVERRIDE;
    const OSString* newVendorString() const APPLE_KEXT_OVERRIDE;
    const OSString* newModelString() const APPLE_KEXT_OVERRIDE;
    
public:
    IOReturn apple80211Request(UInt32 request_type, int request_number, IOInterface* interface, void* data) APPLE_KEXT_OVERRIDE;
    int bpfOutputPacket(IOInterface *interface,UInt,mbuf_t m) APPLE_KEXT_OVERRIDE;
    int outputRaw80211Packet(IOInterface *interface, mbuf_t m) APPLE_KEXT_OVERRIDE;
    int outputActionFrame(IOInterface *interface, mbuf_t m) APPLE_KEXT_OVERRIDE;
    int bpfOutput80211Radio(IOInterface *interface, mbuf_t m) APPLE_KEXT_OVERRIDE;
    
    virtual bool useAppleRSNSupplicant(IOInterface* interface) APPLE_KEXT_OVERRIDE;
//    virtual bool useAppleRSNSupplicant(IO80211VirtualInterface* interface) APPLE_KEXT_OVERRIDE;
private:
    // 1 - SSID
    IOReturn getSSID(IOInterface* interface, struct apple80211_ssid_data* sd);
    IOReturn setSSID(IOInterface* interface, struct apple80211_ssid_data* sd);
    // 2 - AUTH_TYPE
    IOReturn getAUTH_TYPE(IOInterface* interface, struct apple80211_authtype_data* ad);
    IOReturn setAUTH_TYPE(IOInterface* interface, struct apple80211_authtype_data* ad);
    // 3 - CIPHER_KEY
    IOReturn setCIPHER_KEY(IOInterface*, apple80211_key*);
    IOReturn getCIPHER_KEY(IOInterface*, apple80211_key*);
    // 4 - CHANNEL
    IOReturn getCHANNEL(IOInterface* interface, struct apple80211_channel_data* cd);
    IOReturn setCHANNEL(IOInterface* interface, struct apple80211_channel_data* cd);
    // 5 - POWERSAVE
    IOReturn getPOWERSAVE(IOInterface* interface, struct apple80211_powersave_data* cd);
//    IOReturn setPOWERSAVE(IOInterface* interface, struct apple80211_powersave_data* cd);
    // 6 - PROTMODE
    IOReturn getPROTMODE(IOInterface* interface, struct apple80211_protmode_data* ret);
    // 7 - TXPOWER
    IOReturn getTXPOWER(IOInterface* interface, struct apple80211_txpower_data* txd);
//    IOReturn setTXPOWER(IOInterface* interface, struct apple80211_txpower_data* txd);
    // 8 - RATE
    IOReturn getRATE(IOInterface* interface, struct apple80211_rate_data* rd);
    // 9 - BSSID
    IOReturn getBSSID(IOInterface* interface, struct apple80211_bssid_data* bd);
    IOReturn setBSSID(IOInterface* interface, struct apple80211_bssid_data* bd);
    // 10 - SCAN_REQ
    IOReturn setSCAN_REQ(IOInterface* interface, struct apple80211_scan_data* sd);
    // 11 - SCAN_RESULT
    IOReturn getSCAN_RESULT(IOInterface* interface, apple80211_scan_result* *sr);
    // 12 - CARD_CAPABILITIES
    IOReturn getCARD_CAPABILITIES(IOInterface* interface, struct apple80211_capability_data* cd);
    // 13 - STATE
    IOReturn getSTATE(IOInterface* interface, struct apple80211_state_data* sd);
    // 14 - PHY_MODE
    IOReturn getPHY_MODE(IOInterface* interface, struct apple80211_phymode_data* pd);
    // 15 - OP_MODE
    IOReturn getOP_MODE(IOInterface* interface, struct apple80211_opmode_data* od);
    // 16 - RSSI
    IOReturn getRSSI(IOInterface* interface, struct apple80211_rssi_data* rd);
    // 17 - NOISE
    IOReturn getNOISE(IOInterface* interface,struct apple80211_noise_data* nd);
    // 18 - INT_MIT
    IOReturn getINT_MIT(IOInterface* interface, struct apple80211_intmit_data* imd);
    // 19 - POWER
    IOReturn getPOWER(IOInterface* interface, struct apple80211_power_data* pd);
    IOReturn setPOWER(IOInterface* interface, struct apple80211_power_data* pd);
    // 20 - ASSOCIATE
    IOReturn setASSOCIATE(IOInterface* interface, struct apple80211_assoc_data* ad);
    // 21 - ASSOCIATE_RESULT
    IOReturn getASSOCIATE_RESULT(IOInterface* interface,struct apple80211_assoc_result_data* ard);
    // 22 - DISASSOCIATE
    IOReturn setDISASSOCIATE(IOInterface* interface);
//    // 23 - STATUS_DEV_NAME
//    IOReturn getSTATUS_DEV_NAME(IOInterface *interface, struct apple80211_status_dev_data *hv);
    // 27 - SUPPORTED_CHANNELS
    IOReturn getSUPPORTED_CHANNELS(IOInterface* interface, struct apple80211_sup_channel_data* ad);
    // 28 - LOCALE
    IOReturn getLOCALE(IOInterface* interface, struct apple80211_locale_data* ld);
    // 29 - DEAUTH
    IOReturn getDEAUTH(IOInterface *interface, struct apple80211_deauth_data *ret);
//    // 31 - FRAG_THRESHOLD
//    IOReturn getFRAG_THRESHOLD(IOInterface* interface, struct apple80211_frag_threshold_data* ld);
    // 32 - RATE_SET
    IOReturn getRATE_SET(IOInterface* interface, struct apple80211_rate_set_data* ret);
    // 37 - TX_ANTENNA
    IOReturn getTX_ANTENNA(IOInterface* interface, apple80211_antenna_data* ad);
    // 39 - ANTENNA_DIVERSITY
    IOReturn getANTENNA_DIVERSITY(IOInterface* interface, apple80211_antenna_data* ad);
//    // 40 - ROM
//    IOReturn getROM(IOInterface* interface, apple80211_rom_data* rd);
//    // 42 - STATION_LIST
//    IOReturn getSTATION_LIST(IOInterface *interface, apple80211_sta_data *sd);
    // 43 - DRIVER_VERSION
    IOReturn getDRIVER_VERSION(IOInterface* interface, struct apple80211_version_data* hv);
    // 44 - HARDWARE_VERSION
    IOReturn getHARDWARE_VERSION(IOInterface* interface, struct apple80211_version_data* hv);
    // 46 - RSN_IE
    IOReturn getRSN_IE(IOInterface* interface, struct apple80211_rsn_ie_data* rid);
//    IOReturn setRSN_IE(IOInterface *interface, struct apple80211_rsn_ie_data *rid);
    // 48 AP_IE_LIST
    IOReturn getAP_IE_LIST(IOInterface *interface, struct apple80211_ap_ie_data *data);
    // 50 - ASSOCIATION_STATUS
    IOReturn getASSOCIATION_STATUS(IOInterface* interface, struct apple80211_assoc_status_data* ret);
    IOReturn setASSOCIATION_STATUS(IOInterface* interface, struct apple80211_assoc_status_data* ret);
    // 51 - COUNTRY_CODE
    IOReturn getCOUNTRY_CODE(IOInterface* interface, struct apple80211_country_code_data* cd);
    IOReturn setCOUNTRY_CODE(IOInterface* interface, struct apple80211_country_code_data* cd);
//    // 53 - LAST_RX_PKT_DATA
//    IOReturn getLAST_RX_PKT_DATA(IOInterface *interface,struct apple80211_last_rx_pkt_data *ret);
    // 54 - RADIO_INFO
    IOReturn getRADIO_INFO(IOInterface* interface, struct apple80211_radio_info_data* md);
    // 57 - MCS
    IOReturn getMCS(IOInterface* interface, struct apple80211_mcs_data* md);
    // 66 - MCS_INDEX_SET
    IOReturn getMCS_INDEX_SET(IOInterface* interface, struct apple80211_mcs_index_set_data* md);
    // 80 - ROAM_THRESH
    IOReturn getROAM_THRESH(IOInterface* interface, struct apple80211_roam_threshold_data* rtd);
//    // 85 - IE
//    IOReturn getIE(IOInterface* interface, struct apple80211_rsn_ie_data *ret);
    // 86 - SCAN_REQ_MULTIPLE
    IOReturn setSCAN_REQ_MULTIPLE(IOInterface *interface, struct apple80211_scan_multiple_data *sd);
    // 90 - SCANCACHE_CLEAR
    IOReturn setSCANCACHE_CLEAR(IOInterface* interface, device *);
//    // 112 - FACTORY_MODE
//    IOReturn getFACTORY_MODE(IOInterface* interface, struct apple80211_factory_mode_data* fmd);
    // 156 - LINK_CHANGED_EVENT_DATA
    IOReturn getLINK_CHANGED_EVENT_DATA(IOInterface* interface, struct apple80211_link_changed_event_data* ed);
    // 196 - TX_NSS
    IOReturn getTX_NSS(IOInterface *interface, struct apple80211_tx_nss_data *data);
    IOReturn setTX_NSS(IOInterface *interface, struct apple80211_tx_nss_data *data);
    // 216 - ROAM_PROFILE
    IOReturn getROAM_PROFILE(IOInterface *interface, struct apple80211_roam_profile_band_data *data);
    IOReturn setROAM_PROFILE(IOInterface *interface, struct apple80211_roam_profile_band_data *data);
    // 353
    IOReturn getNSS(IOInterface *interface, struct apple80211_nss_data *data);
    
private:
    static IOReturn setPowerStateAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);
    
public:
    
    int chanspec2applechannel(int ic_flags);
    struct ieee80211_nodereq* findScanResult(apple80211_assoc_data* ad);
    IOReturn scanConvertResult(struct ieee80211_nodereq *nr, struct apple80211_scan_result* oneResult);
    
    void setLinkState(int linkState);
    bool isConnected();
    bool isRun80211X();
    void scanComplete();
    void scanFreeResults();
    IOReturn changePowerState(IOInterface *interface, int powerState);
    void ether_ifattach();
    void ether_ifdetach();
    int enqueueInputPacket2(mbuf_t m);
    void flushInputQueue2();
    
    static void scanDone(OSObject *owner, ...);
    void if_input(struct ifnet* ifp, struct mbuf_list *ml);
    static IOReturn tsleepHandler(OSObject* owner, void* arg0, void* arg1, void* arg2, void* arg3);
    static void firmwareLoadComplete(OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context);
    
    void firmwareLoadComplete(const char* name);
    int    loadfirmware(const char *name, u_char **bufp, size_t *buflen);
    
    void if_watchdog(IOTimerEventSource *timer);
    
    bool addMediumType(UInt32 type, UInt32 speed, UInt32 code, char* name = 0);
    
    OSString *getNVRAMProperty(char *name);
    
public:
    IOPCIDevice *fPciDevice;
    WorkLoop *fWorkloop;
    IOCommandGate   *fCommandGate;
    IOTimerEventSource *fWatchdogTimer;
    IOTimerEventSource* fTimerEventSource;
    IONetworkMedium *autoMedium;
    IONetworkMedium *mediumTable[MEDIUM_TYPE_INVALID];
    
    IOLock *fwLoadLock;

    OSData *firmwareData;
    
    IONetworkStats *netStats;
//    IOOutputQueueStats *outputStats;
    IOEthernetStats *etherStats;

    /* power management data */
    unsigned long powerState;
//    bool power_state;
    
    OSDictionary* mediumDict;
    
    struct pci_attach_args *pa;
    struct cfdriver *cd;
    struct cfattach *ca;
    
    OSArray* scanResults;
    uint32_t scanIndex;

    void* if_softc;
    bool firstUp = true;
    
public:
    struct apple80211_rate_data rate_data;
    struct apple80211_rssi_data rssi_data;
    struct apple80211_noise_data noise_data;
    
    struct apple80211_authtype_data authtype_data;
    struct apple80211_bssid_data bssid_data;
    struct apple80211_rsn_ie_data rsn_ie_data;
    struct apple80211_assoc_data assoc_data;
    struct apple80211_assoc_status_data assoc_status_data;
    struct apple80211_key cipher_key;
    struct apple80211_country_code_data ccd;
    
    bool scanFlag = false;
    struct apple80211_scan_data scanRequest;
    struct apple80211_scan_multiple_data scanMultiRequest;
    
    int try_times;
    bool disassoc_times = false;
    
    char i_psk[256];
    char key_tmp[256];
    
    const char *configArr[256];
    int configArrCount = 0;
    
public:
    static void autoASSOC(OSObject *owner, ...);
    OSArray *assoc_data_Arr;
    int assoc_data_index = 0;
    int times = 0;
    

};


#endif
