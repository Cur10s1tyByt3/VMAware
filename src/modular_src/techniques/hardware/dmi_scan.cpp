#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for string matches of VM brands in the linux DMI
 * @category Linux
 * @implements VM::DMI_SCAN
 */
[[nodiscard]] static bool dmi_scan() {
#if (!LINUX)
    return false;
#else
    /*
    cat: /sys/class/dmi/id/board_serial: Permission denied
    cat: /sys/class/dmi/id/chassis_serial: Permission denied
    cat: /sys/class/dmi/id/product_serial: Permission denied
    cat: /sys/class/dmi/id/product_uuid: Permission denied
    */

    constexpr std::array<const char*, 7> dmi_array {
        "/sys/class/dmi/id/bios_vendor",
        "/sys/class/dmi/id/board_name",
        "/sys/class/dmi/id/board_vendor",
        "/sys/class/dmi/id/chassis_asset_tag",
        "/sys/class/dmi/id/product_family",
        "/sys/class/dmi/id/product_sku",
        "/sys/class/dmi/id/sys_vendor"
    };

    constexpr std::array<std::pair<const char*, const char*>, 15> vm_table {{
        { "kvm", brands::KVM },
        { "openstack", brands::OPENSTACK },
        { "kubevirt", brands::KUBEVIRT },
        { "amazon ec2", brands::AWS_NITRO },
        { "qemu", brands::QEMU },
        { "vmware", brands::VMWARE },
        { "innotek gmbh", brands::VBOX },
        { "virtualbox", brands::VBOX },
        { "oracle corporation", brands::VBOX },
        //{ "xen", XEN },
        { "bochs", brands::BOCHS },
        { "parallels", brands::PARALLELS },
        { "bhyve", brands::BHYVE },
        { "hyper-v", brands::HYPERV },
        { "apple virtualization", brands::APPLE_VZ },
        { "google compute engine", brands::GCE }
    }};

    auto to_lower = [](std::string &str) {
        for (auto& c : str) {
            if (c == ' ') {
                continue;
            }

            c = static_cast<char>(tolower(c));
        }
    };

    for (const auto &vm_string : vm_table) {
        for (const auto file : dmi_array) {
            if (!util::exists(file)) {
                continue;
            }

            std::string content = util::read_file(file);

            to_lower(content);

            if (std::regex_search(content, std::regex(vm_string.first))) {
                debug("DMI_SCAN: content = ", content);
                if (std::strcmp(vm_string.second, brands::AWS_NITRO) == 0) {
                    if (techniques::smbios_vm_bit()) {
                        return core::add(brands::AWS_NITRO);
                    }
                } else {
                    return core::add(vm_string.second);
                }
            }
        }
    }

    return false;
#endif
} 