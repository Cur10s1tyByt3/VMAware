#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
* @brief Check for VM-strings in ioreg commands for MacOS
* @category MacOS
* @link https://evasions.checkpoint.com/techniques/macos.html
* @implements VM::IOREG_GREP
*/
[[nodiscard]] static bool ioreg_grep() {
#if (!APPLE)
    return false;
#else
    auto check_usb = []() -> bool {
        std::unique_ptr<std::string> result = util::sys_result("ioreg -rd1 -c IOUSBHostDevice | grep \"USB Vendor Name\"");
        const std::string usb = *result;

        if (util::find(usb, "Apple")) {
            return false;
        }

        if (util::find(usb, "VirtualBox")) {
            return core::add(brands::VBOX);
        }

        return false;
        };

    auto check_general = []() -> bool {
        std::unique_ptr<std::string> sys_vbox = util::sys_result("ioreg -l | grep -i -c -e \"virtualbox\" -e \"oracle\"");

        if (std::stoi(*sys_vbox) > 0) {
            return core::add(brands::VBOX);
        }

        std::unique_ptr<std::string> sys_vmware = util::sys_result("ioreg -l | grep -i -c -e \"vmware\"");

        if (std::stoi(*sys_vmware) > 0) {
            return core::add(brands::VMWARE);
        }

        return false;
        };

    auto check_rom = []() -> bool {
        std::unique_ptr<std::string> sys_rom = util::sys_result("system_profiler SPHardwareDataType | grep \"Boot ROM Version\"");
        const std::string rom = *sys_rom;

        if (util::find(rom, "VirtualBox")) {
            return core::add(brands::VBOX);
        }

        return false;
    };

    return (
        check_usb() ||
        check_general() ||
        check_rom()
    );
#endif
}