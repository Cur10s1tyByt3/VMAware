#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for various Bochs-related emulation oversights through CPU checks
 * @category x86
 * @note Discovered by Peter Ferrie, Senior Principal Researcher, Symantec Advanced Threat Research peter_ferrie@symantec.com
 * @implements VM::BOCHS_CPU
 */
[[nodiscard]] static bool bochs_cpu() {
#if (!x86)
    return false;
#else
    const bool intel = cpu::is_intel();
    const bool amd = cpu::is_amd();

    // if neither amd or intel, return false
    if (!(intel || amd)) {
        debug("BOCHS_CPU: neither AMD or Intel detected, returned false");
        return false;
    }

    const std::string brand = cpu::get_brand();

    if (intel) {
        // technique 1: not a valid brand 
        if (brand == "              Intel(R) Pentium(R) 4 CPU        ") {
            debug("BOCHS_CPU: technique 1 found");
            return core::add(brands::BOCHS);
        }
    } else if (amd) {
        // technique 2: "processor" should have a capital P
        if (brand == "AMD Athlon(tm) processor") {
            debug("BOCHS_CPU: technique 2 found");
            return core::add(brands::BOCHS);
        }

        // technique 3: Check for absence of AMD easter egg for K7 and K8 CPUs
        constexpr u32 AMD_EASTER_EGG = 0x8fffffff; // this is the CPUID leaf of the AMD easter egg

        if (!cpu::is_leaf_supported(AMD_EASTER_EGG)) {
            return false;
        }

        u32 unused, eax = 0;
        cpu::cpuid(eax, unused, unused, unused, 1);

        auto is_k7 = [](const u32 eax) -> bool {
            const u32 family = (eax >> 8) & 0xF;
            const u32 model = (eax >> 4) & 0xF;
            const u32 extended_family = (eax >> 20) & 0xFF;

            if (family == 6 && extended_family == 0) {
                if (model == 1 || model == 2 || model == 3 || model == 4) {
                    return true;
                }
            }

            return false;
        };

        auto is_k8 = [](const u32 eax) -> bool {
            const u32 family = (eax >> 8) & 0xF;
            const u32 extended_family = (eax >> 20) & 0xFF;

            if (family == 0xF) {
                if (extended_family == 0x00 || extended_family == 0x01) {
                    return true;
                }
            }

            return false;
        };

        if (!(is_k7(eax) || is_k8(eax))) {
            return false;
        }

        u32 ecx_bochs = 0;
        cpu::cpuid(unused, unused, ecx_bochs, unused, AMD_EASTER_EGG);

        if (ecx_bochs == 0) {
            return true;
        }
    }

    return false;
#endif
}