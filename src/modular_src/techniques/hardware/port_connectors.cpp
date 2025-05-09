#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for physical connection ports
 * @category Windows
 * @note original idea of using physical ports to detect VMs was suggested by @unusual-aspect (https://github.com/unusual-aspect). 
 *       This technique is known to flag on devices like Surface Pro.
 * @implements VM::PORT_CONNECTORS
 */
[[nodiscard]] static bool port_connectors() {
#if (!WINDOWS)
    return false;
#else
    const GUID GUID_DEVCLASS_PORTCONNECTOR =
    { 0x4d36e978, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18} };

    HDEVINFO hDevInfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_PORTCONNECTOR,
        nullptr, nullptr, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return true;
    }

    SP_DEVINFO_DATA devInfoData = { sizeof(SP_DEVINFO_DATA) };
    // Check first device only - physical machines typically have multiple. Checking index 0 is sufficient to determine if ANY ports exist
    const bool hasPorts = SetupDiEnumDeviceInfo(hDevInfo, 0, &devInfoData);

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return !hasPorts;
#endif
}