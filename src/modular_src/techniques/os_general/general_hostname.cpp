#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"

/**
 * @brief Check for commonly set hostnames by certain VM brands
 * @category Windows, Linux
 * @note Idea from Thomas Roccia (fr0gger)
 * @link https://unprotect.it/technique/detecting-hostname-username/
 * @implements VM::GENERAL_HOSTNAME
 */
[[nodiscard]] static bool general_hostname() {
#if (!(WINDOWS || LINUX))
    return false;
#else
    std::string hostname = util::get_hostname();

    auto cmp = [&](const char* str2) -> bool {
        return (hostname == str2);
    };

    if (
        cmp("Sandbox") ||
        cmp("Maltest") ||
        cmp("Malware") ||
        cmp("malsand") ||
        cmp("ClonePC")
    ) {
        return true;
    }

    if (cmp("Cuckoo")) {
        return core::add(brands::CUCKOO);
    }

    return false;
#endif
}