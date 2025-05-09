#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if the sysctl for the hwmodel does not contain the "Mac" string
 * @author MacRansom ransomware
 * @category MacOS
 * @implements VM::HWMODEL
 */
[[nodiscard]] static bool hwmodel() {
#if (!APPLE)
    return false;
#else
    auto result = util::sys_result("sysctl -n hw.model");

    std::smatch match;

    if (result == nullptr) {
        debug("HWMODEL: ", "null result received");
        return false;
    }

    debug("HWMODEL: ", "output = ", *result);

    // if string contains "Mac" anywhere in the string, assume it's baremetal
    if (std::regex_search(*result, match, std::regex("Mac"))) {
        return false;
    }

    // not sure about the other VMs, more could potentially be added
    if (std::regex_search(*result, match, std::regex("VMware"))) {
        return core::add(brands::VMWARE);
    }

    // assumed true since it doesn't contain "Mac" string
    return true;
#endif
}