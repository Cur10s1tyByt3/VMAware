#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for default Azure hostname format regex (Azure uses Hyper-V as their base VM brand)
 * @category Windows, Linux
 * @implements VM::HYPERV_HOSTNAME
 */
[[nodiscard]] static bool hyperv_hostname() {
#if (!(WINDOWS || LINUX))
    return false;
#else
    const std::string hostname = util::get_hostname();
    const size_t len = hostname.size();

    // most Hyper-V hostnames under Azure have the hostname format of fv-azXXX-XXX where the X is a digit
    if (len < 8) return false;  // "fv-az0-0" is minimum length

    // Check prefix "fv-az"
    if (!(hostname[0] == 'f' &&
        hostname[1] == 'v' &&
        hostname[2] == '-' &&
        hostname[3] == 'a' &&
        hostname[4] == 'z')) {
        return false;
    }

    const size_t hyphen_pos = hostname.find('-', 5);

    if (hyphen_pos == std::string::npos || 
        hyphen_pos <= 5 ||                   
        hyphen_pos >= len - 1) {             
        return false;
    }

    for (size_t i = 5; i < hyphen_pos; ++i) {
        if (hostname[i] < '0' || hostname[i] > '9') return false;
    }

    for (size_t i = hyphen_pos + 1; i < len; ++i) {
        if (hostname[i] < '0' || hostname[i] > '9') return false;
    }

    return core::add(brands::AZURE_HYPERV);
#endif
}
