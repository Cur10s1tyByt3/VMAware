#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for QEMU's hot-plug signature
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::QEMU_PASSTHROUGH
 */
[[nodiscard]] static bool qemu_passthrough() {
#if (!WINDOWS)
    return false;
#else
    // QEMU passthrough location paths
    static const std::wregex busRegex(L"PCIROOT\\(0\\)#PCI\\(0202\\)");
    static const std::wregex acpiSlotRegex(L"#ACPI\\(S[0-9]{2}_\\)");

    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        &GUID_DEVCLASS_DISPLAY,
        nullptr,
        nullptr,
        DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return false;

    SP_DEVINFO_DATA devInfo = {};
    devInfo.cbSize = sizeof(devInfo);
    const DEVPROPKEY key = DEVPKEY_Device_LocationPaths;

    for (DWORD idx = 0; SetupDiEnumDeviceInfo(hDevInfo, idx, &devInfo); ++idx)
    {
        DEVPROPTYPE propType = 0;
        DWORD requiredSize = 0;

        SetupDiGetDevicePropertyW(
            hDevInfo, &devInfo, &key, &propType,
            nullptr, 0, &requiredSize, 0);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || requiredSize == 0)
            continue;

        std::vector<BYTE> buffer(requiredSize);
        if (!SetupDiGetDevicePropertyW(
            hDevInfo, &devInfo, &key, &propType,
            buffer.data(), requiredSize, &requiredSize, 0))
            continue;

        const wchar_t* ptr = reinterpret_cast<const wchar_t*>(buffer.data());
        while (*ptr) {
            std::wstring path(ptr);
            if (std::regex_search(path, busRegex) ||
                std::regex_search(path, acpiSlotRegex))
            {
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return true;
            }
            ptr += path.size() + 1;
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return false;
#endif
}