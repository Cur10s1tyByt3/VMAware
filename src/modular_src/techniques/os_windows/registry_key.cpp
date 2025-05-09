#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for VM-specific registry values
 * @category Windows
 * @implements VM::REGISTRY
 */
[[nodiscard]] static bool registry_key() {
#if (!WINDOWS)
    return false;
#else
    struct Entry { const char* brand; const char* regkey; };
    static constexpr Entry entries[] = {
        { nullptr, "HKLM\\Software\\Classes\\Folder\\shell\\sandbox" },

        { brands::PARALLELS, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_1AB8*" },

        { brands::SANDBOXIE,  "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Sandboxie" },

        { brands::VBOX, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_80EE*" },
        { brands::VBOX, "HKLM\\HARDWARE\\ACPI\\DSDT\\VBOX__" },
        { brands::VBOX, "HKLM\\HARDWARE\\ACPI\\FADT\\VBOX__" },
        { brands::VBOX, "HKLM\\HARDWARE\\ACPI\\RSDT\\VBOX__" },
        { brands::VBOX, "HKLM\\SOFTWARE\\Oracle\\VirtualBox Guest Additions" },
        { brands::VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxGuest" },
        { brands::VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxMouse" },
        { brands::VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxService" },
        { brands::VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxSF" },
        { brands::VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxVideo" },

        { brands::VPC, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_5333*" },
        { brands::VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpcbus" },
        { brands::VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpc-s3" },
        { brands::VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpcuhub" },
        { brands::VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\msvmmouf" },

        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_15AD*" },
        { brands::VMWARE, "HKCU\\SOFTWARE\\VMware, Inc.\\VMware Tools" },
        { brands::VMWARE, "HKLM\\SOFTWARE\\VMware, Inc.\\VMware Tools" },
        { brands::VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmdebug" },
        { brands::VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmmouse" },
        { brands::VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\VMTools" },
        { brands::VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\VMMEMCTL" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Services\\vmmouse" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Services\\vmusbmouse" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\CdRomNECVMWar_VMware_IDE_CD*" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\CdRomNECVMWar_VMware_SATA_CD*" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\DiskVMware_Virtual_IDE_Hard_Drive*" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\DiskVMware_Virtual_SATA_Hard_Drive*" },
        { brands::VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Enum\\ACPI\\VMW0003" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Services\\vmmouse" },
        { brands::VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Services\\vmusbmouse" },

        { brands::WINE,    "HKCU\\SOFTWARE\\Wine" },
        { brands::WINE,    "HKLM\\SOFTWARE\\Wine" },

        { brands::XEN, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_5853*" },
        { brands::XEN, "HKLM\\HARDWARE\\ACPI\\DSDT\\xen" },
        { brands::XEN, "HKLM\\HARDWARE\\ACPI\\FADT\\xen" },
        { brands::XEN, "HKLM\\HARDWARE\\ACPI\\RSDT\\xen" },
        { brands::XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xenevtchn" },
        { brands::XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xennet" },
        { brands::XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xennet6" },
        { brands::XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xensvc" },
        { brands::XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xenvdb" },

        { brands::KVM,  "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_1AF4*" },
        { brands::KVM,  "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_1B36*" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\vioscsi" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\viostor" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\VirtIO-FS Service" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\VirtioSerial" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\BALLOON" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\BalloonService" },
        { brands::KVM,  "HKLM\\SYSTEM\\ControlSet001\\Services\\netkvm" },
    };

    u8 score = 0;
    const bool wow64 = util::is_wow64();
    const REGSAM sam = wow64 ? (KEY_READ | KEY_WOW64_64KEY) : KEY_READ;

    for (auto const& e : entries) {
        const char* full = e.regkey;
        HKEY hRoot = nullptr;

        if (full[0] == 'H' && full[1] == 'K' && full[2] == 'L' && full[3] == 'M' && full[4] == '\\') {
            hRoot = HKEY_LOCAL_MACHINE; full += 5;
        }
        else if (full[0] == 'H' && full[1] == 'K' && full[2] == 'C' && full[3] == 'U' && full[4] == '\\') {
            hRoot = HKEY_CURRENT_USER;  full += 5;
        }
        else continue;

        const char* sub = full;
        bool wildcard = std::strchr(sub, '*') || std::strchr(sub, '?');

        if (wildcard) {
            const char* slash = std::strrchr(sub, '\\');
            const char* parent = slash ? sub : "";
            const char* pattern = slash ? slash + 1 : sub;

            HKEY hParent;
            if (RegOpenKeyExA(hRoot, parent, 0, sam, &hParent) != ERROR_SUCCESS)
                continue;

            DWORD idx = 0;
            char  name[MAX_PATH];
            DWORD nameLen = MAX_PATH;
            bool found = false;

            while (RegEnumKeyExA(hParent, idx, name, &nameLen,
                nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
            {
                if (PathMatchSpecA(name, pattern)) {
                    found = true;
                    break;
                }
                idx++;
                nameLen = MAX_PATH;
            }
            RegCloseKey(hParent);

            if (!found) continue;
        }
        else {
            HKEY hKey;
            if (RegOpenKeyExA(hRoot, sub, 0, sam, &hKey) != ERROR_SUCCESS)
                continue;
            RegCloseKey(hKey);
        }

        score++;
        if (e.brand && e.brand[0]) {
            debug("REGISTRY: ", "detected = ", e.brand);
            core::add(e.brand);
        }
    }

    debug("REGISTRY: ", "score = ", static_cast<u32>(score));
    return score >= 1;
#endif
}