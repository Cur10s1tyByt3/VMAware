#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if the chassis vendor is a VM vendor
 * @category Linux
 * @implements VM::CVENDOR
 */
[[nodiscard]] bool techniques::chassis_vendor() {
#if (!LINUX)
    return false;
#else
    const char* vendor_file = "/sys/devices/virtual/dmi/id/chassis_vendor";

    if (!util::exists(vendor_file)) {
        debug("CVENDOR: ", "file doesn't exist");
        return false;
    }

    const std::string vendor = util::read_file(vendor_file);

    // TODO: More can definitely be added, I only tried QEMU and VBox so far
    if (util::find(vendor, "QEMU")) { return core::add(brands::QEMU); }
    if (util::find(vendor, "Oracle Corporation")) { return core::add(brands::VBOX); }

    debug("CVENDOR: vendor = ", vendor);

    return false;
#endif
}