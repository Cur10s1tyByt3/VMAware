#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check result from systemd-detect-virt tool
 * @category Linux
 * @implements VM::SYSTEMD
 */
[[nodiscard]] bool techniques::systemd_virt() {
#if (!LINUX)
    return false;
#else
    if (!(util::exists("/usr/bin/systemd-detect-virt") || util::exists("/bin/systemd-detect-virt"))) {
        debug("SYSTEMD: ", "binary doesn't exist");
        return false;
    }

    const std::unique_ptr<std::string> result = util::sys_result("systemd-detect-virt");

    if (result == nullptr) {
        debug("SYSTEMD: ", "invalid stdout output from systemd-detect-virt");
        return false;
    }

    debug("SYSTEMD: ", "output = ", *result);

    return (*result != "none");
#endif
}