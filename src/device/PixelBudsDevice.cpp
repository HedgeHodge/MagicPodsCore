#include "PixelBudsDevice.h"

#ifdef _WIN32
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <experimental/coroutine>
#include "Logger.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

const guid FAST_PAIR_SERVICE_UUID = guid("0000fe2c-0000-1000-8000-00805f9b34fb");
const guid BATTERY_SERVICE_UUID = guid("0000180f-0000-1000-8000-00805f9b34fb");
const guid BATTERY_LEVEL_CHARACTERISTIC_UUID = guid("00002a19-0000-1000-8000-00805f9b34fb");

struct PixelBudsDevice::impl {
    BluetoothLEDevice device = nullptr;
    GattDeviceServicesResult servicesResult = nullptr;
    GattCharacteristicsResult batteryCharacteristicResult = nullptr;

    fire_and_forget connect_async(PixelBudsDevice* self);
    IAsyncOperation<DeviceBattery> get_battery_async();
};

fire_and_forget PixelBudsDevice::impl::connect_async(PixelBudsDevice* self) {
    try {
        uint64_t bluetoothAddress = std::stoull(self->GetAddress(), nullptr, 16);
        device = co_await BluetoothLEDevice::FromBluetoothAddressAsync(bluetoothAddress);

        if (device) {
            servicesResult = co_await device.GetGattServicesForUuidAsync(FAST_PAIR_SERVICE_UUID);
            if (servicesResult.Status() == GattCommunicationStatus::Success && servicesResult.Services().Size() > 0) {
                self->SetConnected(true);
            }
        }
    }
    catch (const hresult_error& ex) {
        MagicPodsCore::Logger::Error("Connection failed: %ls", ex.message().c_str());
    }
}

IAsyncOperation<DeviceBattery> PixelBudsDevice::impl::get_battery_async() {
    DeviceBattery battery;
    try {
        if (servicesResult && servicesResult.Status() == GattCommunicationStatus::Success) {
            auto service = servicesResult.Services().GetAt(0);
            batteryCharacteristicResult = co_await service.GetCharacteristicsForUuidAsync(BATTERY_LEVEL_CHARACTERISTIC_UUID);

            if (batteryCharacteristicResult.Status() == GattCommunicationStatus::Success && batteryCharacteristicResult.Characteristics().Size() > 0) {
                auto characteristic = batteryCharacteristicResult.Characteristics().GetAt(0);
                auto readResult = co_await characteristic.ReadValueAsync();

                if (readResult.Status() == GattCommunicationStatus::Success) {
                    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(readResult.Value());
                    byte left_bud_battery = reader.ReadByte();
                    byte right_bud_battery = reader.ReadByte();
                    byte case_battery = reader.ReadByte();
                    battery.SetLeft(left_bud_battery);
                    battery.SetRight(right_bud_battery);
                    battery.SetCase(case_battery);
                }
            }
        }
    }
    catch (const hresult_error& ex) {
        MagicPodsCore::Logger::Error("Get battery failed: %ls", ex.message().c_str());
    }
    co_return battery;
}

PixelBudsDevice::PixelBudsDevice(const std::string& address, const std::string& name)
    : Device(address, name), pimpl(std::make_unique<impl>()) {}

PixelBudsDevice::~PixelBudsDevice() {}

void PixelBudsDevice::connect() {
    pimpl->connect_async(this);
}

void PixelBudsDevice::disconnect() {
    if (pimpl->device) {
        pimpl->device = nullptr;
        SetConnected(false);
    }
}

DeviceBattery PixelBudsDevice::get_battery() {
    if (pimpl->device) {
        return pimpl->get_battery_async().get();
    }
    return DeviceBattery();
}

#else

PixelBudsDevice::PixelBudsDevice(const std::string& address, const std::string& name)
    : Device(address, name) {}

PixelBudsDevice::~PixelBudsDevice() {}

void PixelBudsDevice::connect() {}

void PixelBudsDevice::disconnect() {}

DeviceBattery PixelBudsDevice::get_battery() {
    return DeviceBattery();
}

#endif