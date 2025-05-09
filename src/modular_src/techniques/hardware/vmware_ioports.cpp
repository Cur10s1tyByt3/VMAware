#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for VMware string in /proc/ioports
 * @category Windows
 * @note idea from ScoopyNG by Tobias Klein
 * @implements VM::VMWARE_IOPORTS
 */
[[nodiscard]] static bool vmware_ioports() {
#if (!LINUX)
    return false;
#else
    const std::string ioports_file = util::read_file("/proc/ioports");

    if (util::find(ioports_file, "VMware")) {
        return core::add(brands::VMWARE);
    }

    return false;
#endif
}