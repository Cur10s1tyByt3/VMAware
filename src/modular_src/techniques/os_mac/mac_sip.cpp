#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if System Integrity Protection is disabled (likely a VM if it is)
 * @category MacOS
 * @link https://evasions.checkpoint.com/techniques/macos.html
 * @implements VM::MAC_SIP
 */
[[nodiscard]] static bool mac_sip() {
#if (!APPLE)
    return false;
#else
    std::unique_ptr<std::string> result = util::sys_result("csrutil status");
    const std::string tmp = *result;

    debug("MAC_SIP: ", "result = ", tmp);

    return (util::find(tmp, "disabled") || (!util::find(tmp, "enabled")));
#endif
}