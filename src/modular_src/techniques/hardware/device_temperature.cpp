#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for device's temperature
 * @category Windows
 * @implements VM::ACPI_TEMPERATURE
 */
[[nodiscard]] static bool acpi_temperature() {
#if (!WINDOWS)
    return false;
#else
    const GUID GUID_DEVCLASS_THERMALZONE = { 0x4AFA3D51, 0x74A7, 0x11d0, {0xBE, 0x5E, 0x00, 0xA0, 0xC9, 0x06, 0x28, 0x57} };
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_THERMALZONE, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return false;
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData{};
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    const bool exists = SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &GUID_DEVCLASS_THERMALZONE, 0, &deviceInterfaceData);
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return !exists;
#endif
}