#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for vm-specific devices
 * @category Windows
 * @implements VM::VM_DEVICES
 */
[[nodiscard]] static bool vm_devices() {
#if (!WINDOWS)
    return false;
#else
    const HANDLE handle1 = CreateFileA(("\\\\.\\VBoxMiniRdrDN"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const HANDLE handle2 = CreateFileA(("\\\\.\\pipe\\VBoxMiniRdDN"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const HANDLE handle3 = CreateFileA(("\\\\.\\VBoxTrayIPC"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const HANDLE handle4 = CreateFileA(("\\\\.\\pipe\\VBoxTrayIPC"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const HANDLE handle5 = CreateFileA(("\\\\.\\HGFS"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const HANDLE handle6 = CreateFileA(("\\\\.\\pipe\\cuckoo"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    bool vbox = false;

    if (
        (handle1 != INVALID_HANDLE_VALUE) ||
        (handle2 != INVALID_HANDLE_VALUE) ||
        (handle3 != INVALID_HANDLE_VALUE) ||
        (handle4 != INVALID_HANDLE_VALUE)
        ) {
        vbox = true;
    }

    CloseHandle(handle1);
    CloseHandle(handle2);
    CloseHandle(handle3);
    CloseHandle(handle4);

    if (vbox) {
        debug("VM_DEVICES: Detected VBox related device handles");
        return core::add(brands::VBOX);
    }

    if (handle5 != INVALID_HANDLE_VALUE) {
        CloseHandle(handle5);
        debug("VM_DEVICES: Detected VMware related device (HGFS)");
        return core::add(brands::VMWARE);
    }
    if (handle6 != INVALID_HANDLE_VALUE) {
        CloseHandle(handle6);
        debug("VM_DEVICES: Detected Cuckoo related device (pipe)");
        return core::add(brands::CUCKOO);
    }

    CloseHandle(handle5);
    CloseHandle(handle6);

    return false;
#endif
}    