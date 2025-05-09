#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if dmesg output matches a VM brand
 * @category Linux
 * @implements VM::DMESG
 */
[[nodiscard]] static bool dmesg() {
#if (!LINUX || CPP <= 11)
    return false;
#else
    if (!util::is_admin()) {
        return false;
    }

    if (!util::exists("/bin/dmesg") && !util::exists("/usr/bin/dmesg")) {
        debug("DMESG: ", "binary doesn't exist");
        return false;
    }

    const std::unique_ptr<std::string> result = util::sys_result("dmesg | grep -i hypervisor | grep -c \"KVM|QEMU\"");

    if (*result == "" || result == nullptr) {
        return false;
    } else if (*result == "KVM") {
        return core::add(brands::KVM);
    } else if (*result == "QEMU") {
        return core::add(brands::QEMU);
    } else if (std::atoi(result->c_str())) {
        return true;
    } else {
        debug("DMESG: ", "output = ", *result);
    }

    return false;
#endif
}