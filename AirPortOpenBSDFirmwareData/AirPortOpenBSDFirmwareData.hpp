/* add your code here */
#ifndef AirPortOpenBSDFirmwareData_h
#define AirPortOpenBSDFirmwareData_h

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <libkern/OSKextLib.h>

#include "firmware.hpp"

#define kAirPortOpenBSDFirmwareDataService "AirPortOpenBSDFirmwareData"

class AirPortOpenBSDFirmwareData : public IOService
{
    typedef IOService super;
    OSDeclareDefaultStructors(AirPortOpenBSDFirmwareData)
    
public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    
    OSData* firmwareLoadComplete(const char* name);

};


#endif
