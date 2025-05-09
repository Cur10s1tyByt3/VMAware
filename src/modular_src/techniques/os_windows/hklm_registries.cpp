#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check HKLM registries for specific VM strings
 * @category Windows
 * @implements VM::HKLM_REGISTRIES
 */
[[nodiscard]] static bool hklm_registries() {
#if (!WINDOWS)
    return false;
#else
    u8 count = 0;

    std::unordered_set<std::string> failedKeys;

    auto check_key = [&failedKeys, &count](const char* p_brand, const char* subKey, const char* valueName, const char* comp_string) {
        if (failedKeys.find(subKey) != failedKeys.end()) {
            return;
        }

        HKEY hKey;
        DWORD dwType;
        char buffer[1024] = {};
        DWORD bufferSize = sizeof(buffer);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T(subKey), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueEx(hKey, _T(valueName), nullptr, &dwType, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
                if (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_MULTI_SZ) {
                    buffer[bufferSize - 1] = '\0';
                    if (strstr(buffer, comp_string) != nullptr) {
                        debug("HKLM_REGISTRIES: Found ", comp_string, " in ", subKey, " for brand ", p_brand);
                        core::add(p_brand);
                        count++;
                    }
                }
            }
            RegCloseKey(hKey);
        }
        else {
            failedKeys.insert(subKey);
        }
        };

    check_key(brands::BOCHS, "HARDWARE\\Description\\System", "SystemBiosVersion", "BOCHS");
    check_key(brands::BOCHS, "HARDWARE\\Description\\System", "VideoBiosVersion", "BOCHS");

    check_key(brands::ANUBIS, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "76487-337-8429955-22614");
    check_key(brands::ANUBIS, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "76487-337-8429955-22614");

    check_key(brands::CWSANDBOX, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "76487-644-3177037-23510");
    check_key(brands::CWSANDBOX, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "76487-644-3177037-23510");

    check_key(brands::JOEBOX, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "55274-640-2673064-23950");
    check_key(brands::JOEBOX, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "55274-640-2673064-23950");

    check_key(brands::PARALLELS, "HARDWARE\\Description\\System", "SystemBiosVersion", "PARALLELS");
    check_key(brands::PARALLELS, "HARDWARE\\Description\\System", "VideoBiosVersion", "PARALLELS");

    check_key(brands::QEMU, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "QEMU");
    check_key(brands::QEMU, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "QEMU");
    check_key(brands::QEMU, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "QEMU");
    check_key(brands::QEMU, "HARDWARE\\Description\\System", "SystemBiosVersion", "QEMU");
    check_key(brands::QEMU, "HARDWARE\\Description\\System", "VideoBiosVersion", "QEMU");
    check_key(brands::QEMU, "HARDWARE\\Description\\System\\BIOS", "SystemManufacturer", "QEMU");

    check_key(brands::VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
    check_key(brands::VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
    check_key(brands::VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
    check_key(brands::VBOX, "HARDWARE\\Description\\System", "SystemBiosVersion", "VBOX");
    check_key(brands::VBOX, "HARDWARE\\DESCRIPTION\\System", "SystemBiosDate", "06/23/99");
    check_key(brands::VBOX, "HARDWARE\\Description\\System", "VideoBiosVersion", "VIRTUALBOX");
    check_key(brands::VBOX, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "VIRTUAL");
    check_key(brands::VBOX, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
    check_key(brands::VBOX, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VIRTUAL");
    check_key(brands::VBOX, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VIRTUALBOX");

    check_key(brands::VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
    check_key(brands::VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
    check_key(brands::VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
    check_key(brands::VMWARE, "HARDWARE\\Description\\System", "SystemBiosVersion", "VMWARE");
    check_key(brands::VMWARE, "HARDWARE\\Description\\System", "SystemBiosVersion", "INTEL - 6040000");
    check_key(brands::VMWARE, "HARDWARE\\Description\\System", "VideoBiosVersion", "VMWARE");
    check_key(brands::VMWARE, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "VMware");
    check_key(brands::VMWARE, "HARDWARE\\Description\\System\\BIOS", "SystemManufacturer", "VMware, Inc.");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "0", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "1", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "FriendlyName", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "FriendlyName", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "FriendlyName", "VMware");
    check_key(brands::VMWARE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", "DisplayName", "vmware tools");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "CoInstallers32", "*vmx*");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "DriverDesc", "VMware*");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "InfSection", "vmx*");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "ProviderName", "VMware*");
    check_key(brands::VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000\\Settings", "Device Description", "VMware*");
    check_key(brands::VMWARE, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VMWARE");
    check_key(brands::VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\Video", "Service", "vm3dmp");
    check_key(brands::VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\Video", "Service", "vmx_svga");
    check_key(brands::VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\0000", "Device Description", "VMware SVGA*");
    check_key(brands::VMWARE, "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", "0", "VMWare");
    check_key(brands::VMWARE, "HARDWARE\\ACPI\\DSDT\\PTLTD_\\CUSTOM__\\00000000", "00000000", "VMWARE");

    check_key(brands::XEN, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "Xen");

    return (count > 0);
#endif
}