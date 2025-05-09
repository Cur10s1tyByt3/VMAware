#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if dmidecode output matches a VM brand
 * @category Linux
 * @implements VM::DMIDECODE
 */
[[nodiscard]] static bool dmidecode() {
#if (!LINUX)
    return false;
#else
    if (!util::is_admin()) {
        debug("DMIDECODE: ", "precondition return called (root = ", util::is_admin(), ")");
        return false;
    }

    if (!(util::exists("/bin/dmidecode") || util::exists("/usr/bin/dmidecode"))) {
        debug("DMIDECODE: ", "binary doesn't exist");
        return false;
    }

    const std::unique_ptr<std::string> result = util::sys_result("dmidecode -t system | grep 'Manufacturer|Product' | grep -c \"QEMU|VirtualBox|KVM\"");

    if (*result == "" || result == nullptr) {
        debug("DMIDECODE: ", "invalid output");
        return false;
    } else if (*result == "QEMU") {
        return core::add(brands::QEMU);
    } else if (*result == "VirtualBox") {
        return core::add(brands::VBOX);
    } else if (*result == "KVM") {
        return core::add(brands::KVM);
    } else if (std::atoi(result->c_str()) >= 1) {
        return true;
    } else {
        debug("DMIDECODE: ", "output = ", *result);
    }

    return false;
#endif
}