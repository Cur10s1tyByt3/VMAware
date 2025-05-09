#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for IVSHMEM device absense
 * @category Windows
 * @author dmfrpro (https://github.com/dmfrpro)
 * @implements VM::IVSHMEM
 */
[[nodiscard]] static bool ivshmem() {
#if (!WINDOWS)
    return false;
#else
    static const GUID GUID_IVSHMEM_IFACE =
    { 0xdf576976, 0x569d, 0x4672, {0x95,0xa0,0xf5,0x7e,0x4e,0xa0,0xb2,0x10} };

    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_IVSHMEM_IFACE,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return false;
    }

    SP_DEVICE_INTERFACE_DATA ifaceData;
    ifaceData.cbSize = sizeof(ifaceData);
    BOOL gotOne = SetupDiEnumDeviceInterfaces(
        hDevInfo,
        nullptr,
        &GUID_IVSHMEM_IFACE,
        0,
        &ifaceData
    );

    SetupDiDestroyDeviceInfoList(hDevInfo);

    if (gotOne == TRUE) {
        core::add(brands::HYPERVISOR_PHANTOM);
        return true;
    }
    return false;
#endif
}