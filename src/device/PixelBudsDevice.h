
#pragma once

#include "Device.h"

class PixelBudsDevice : public Device {
public:
    PixelBudsDevice(const std::string& address, const std::string& name);
    ~PixelBudsDevice() override;

    void connect() override;
    void disconnect() override;
    
    DeviceBattery get_battery() override;
};
