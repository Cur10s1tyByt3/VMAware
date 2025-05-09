#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if CPU brand model contains any VM-specific string snippets
 * @category x86
 * @implements VM::CPU_BRAND
 */
[[nodiscard]] bool techniques::cpu_brand() {
#if (!x86)
    return false;
#else
    const std::string& brand = cpu::get_brand();

    struct KeywordCheck {
        const char* str;
        std::regex reg;
    };

    static const std::array<KeywordCheck, 10> checks = { {
        {"qemu",      std::regex("qemu",      std::regex::icase | std::regex::optimize)},
        {"kvm",       std::regex("kvm",       std::regex::icase | std::regex::optimize)},
        {"vbox",      std::regex("vbox",      std::regex::icase | std::regex::optimize)},
        {"virtualbox",std::regex("virtualbox",std::regex::icase | std::regex::optimize)},
        {"monitor",   std::regex("monitor",   std::regex::icase | std::regex::optimize)},
        {"bhyve",     std::regex("bhyve",     std::regex::icase | std::regex::optimize)},
        {"hypervisor",std::regex("hypervisor",std::regex::icase | std::regex::optimize)},
        {"hvisor",    std::regex("hvisor",    std::regex::icase | std::regex::optimize)},
        {"parallels", std::regex("parallels", std::regex::icase | std::regex::optimize)},
        {"vmware",    std::regex("vmware",    std::regex::icase | std::regex::optimize)}
    } };

    u8 match_count = 0;
    bool qemu_found = false;

    for (const auto& check : checks) {
        if (std::regex_search(brand, check.reg)) {
            util::debug("BRAND_KEYWORDS: match = ", check.str);
            match_count++;

            if (!qemu_found && (check.str[0] == 'q' || check.str[0] == 'Q')) {
                qemu_found = (std::strcmp(check.str, "qemu") == 0);
            }
        }
    }

    debug("BRAND_KEYWORDS: matches: ", static_cast<u32>(match_count));

    if (qemu_found) {
        return core::add(brands::QEMU);
    }

    return (match_count > 0);
#endif
}