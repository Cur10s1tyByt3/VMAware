#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for presence of QEMU in the /sys/kernel/debug/usb/devices directory
 * @category Linux
 * @implements VM::QEMU_USB
 */
[[nodiscard]] static bool qemu_USB() {
#if (!LINUX)
    return false;
#else
    if (!util::is_admin()) {
        return false;
    }

    constexpr const char* usb_path = "/sys/kernel/debug/usb/devices";

    std::ifstream file(usb_path);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("QEMU") != std::string::npos) {
            return true;
        }
    }

    return false;
#endif
}
