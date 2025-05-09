#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VMware-specific device name in dmesg output
 * @category Windows
 * @note idea from ScoopyNG by Tobias Klein
 * @implements VM::VMWARE_DMESG
 */
[[nodiscard]] static bool vmware_dmesg() {
#if (!LINUX)
    return false;
#else
    if (!util::is_admin()) {
        return false;
    }

    if (!util::exists("/usr/bin/dmesg")) {
        return false;
    }

    auto dmesg_output = util::sys_result("dmesg");
    const std::string dmesg = *dmesg_output;

    if (dmesg.empty()) {
        return false;
    }

    if (util::find(dmesg, "BusLogic BT-958")) {
        return core::add(brands::VMWARE);
    }

    if (util::find(dmesg, "pcnet32")) {
        return core::add(brands::VMWARE);
    }

    return false;
#endif
}
