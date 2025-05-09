#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if memory is too low for MacOS system
 * @category MacOS
 * @link https://evasions.checkpoint.com/techniques/macos.html
 * @implements VM::MAC_MEMSIZE
 */
[[nodiscard]] static bool hw_memsize() {
#if (!APPLE)
    return false;
#else
    std::unique_ptr<std::string> result = util::sys_result("sysctl -n hw.memsize");
    const std::string ram = *result;

    if (ram == "0") {
        return false;
    }

    debug("MAC_MEMSIZE: ", "ram size = ", ram);

    for (const char c : ram) {
        if (!std::isdigit(c)) {
            debug("MAC_MEMSIZE: ", "found non-digit character, returned false");
            return false;
        }
    }

    const u64 ram_u64 = std::stoull(ram);

    debug("MAC_MEMSIZE: ", "ram size in u64 = ", ram_u64);

    constexpr u64 limit = 4000000000; // 4GB 

    return (ram_u64 <= limit);
#endif
}