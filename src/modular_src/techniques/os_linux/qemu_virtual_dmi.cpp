#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for presence of QEMU in the /sys/devices/virtual/dmi/id directory
 * @category Linux
 * @implements VM::QEMU_VIRTUAL_DMI
 */
[[nodiscard]] static bool qemu_virtual_dmi() {
#if (!LINUX)
    return false;
#else
    const char* sys_vendor = "/sys/devices/virtual/dmi/id/sys_vendor";
    const char* modalias = "/sys/devices/virtual/dmi/id/modalias";

    if (
        util::exists(sys_vendor) &&
        util::exists(modalias)
    ) {
        const std::string sys_vendor_str = util::read_file(sys_vendor);
        const std::string modalias_str = util::read_file(modalias);

        if (
            util::find(sys_vendor_str, "QEMU") &&
            util::find(modalias_str, "QEMU")
        ) {
            return core::add(brands::QEMU);
        }
    }

    return false;
#endif
} 