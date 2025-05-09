#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for odd CPU threads, usually a sign of modification through VM setting because 99% of CPUs have even numbers of threads
 * @category x86
 * @implements VM::ODD_CPU_THREADS
 */
[[nodiscard]] static bool odd_cpu_threads() {
#if (!x86)
    return false;
#else
    const u32 threads = std::thread::hardware_concurrency();

    struct cpu::stepping_struct steps = cpu::fetch_steppings();

    debug("ODD_CPU_THREADS: model    = ", static_cast<u32>(steps.model));
    debug("ODD_CPU_THREADS: family   = ", static_cast<u32>(steps.family));
    debug("ODD_CPU_THREADS: extmodel = ", static_cast<u32>(steps.extmodel));

    // check if the microarchitecture was made before 2006, which was around the time multi-core processors were implemented
    auto old_microarchitecture = [&steps]() -> bool {
        constexpr std::array<std::array<u8, 3>, 32> old_archs = {{
            // 80486
            {{ 0x4, 0x0, 0x1 }},
            {{ 0x4, 0x0, 0x2 }},
            {{ 0x4, 0x0, 0x3 }},
            {{ 0x4, 0x0, 0x4 }},
            {{ 0x4, 0x0, 0x5 }},
            {{ 0x4, 0x0, 0x7 }},
            {{ 0x4, 0x0, 0x8 }},
            {{ 0x4, 0x0, 0x9 }},

            // P5
            {{ 0x5, 0x0, 0x1 }},
            {{ 0x5, 0x0, 0x2 }},
            {{ 0x5, 0x0, 0x4 }},
            {{ 0x5, 0x0, 0x7 }},
            {{ 0x5, 0x0, 0x8 }},

            // P6
            {{ 0x6, 0x0, 0x1 }},
            {{ 0x6, 0x0, 0x3 }},
            {{ 0x6, 0x0, 0x5 }},
            {{ 0x6, 0x0, 0x6 }},
            {{ 0x6, 0x0, 0x7 }},
            {{ 0x6, 0x0, 0x8 }},
            {{ 0x6, 0x0, 0xA }},
            {{ 0x6, 0x0, 0xB }},

            // Netburst
            {{ 0xF, 0x0, 0x6 }},
            {{ 0xF, 0x0, 0x4 }},
            {{ 0xF, 0x0, 0x3 }},
            {{ 0xF, 0x0, 0x2 }},
            {{ 0xF, 0x0, 0x10 }},

            {{ 0x6, 0x1, 0x5 }}, // Pentium M (Talopai)
            {{ 0x6, 0x1, 0x6 }}, // Core (Client)
            {{ 0x6, 0x0, 0x9 }}, // Pentium M
            {{ 0x6, 0x0, 0xD }}, // Pentium M
            {{ 0x6, 0x0, 0xE }}, // Modified Pentium M
            {{ 0x6, 0x0, 0xF }}  // Core (Client)
        }};

        constexpr u8 FAMILY = 0;
        constexpr u8 EXTMODEL = 1;
        constexpr u8 MODEL = 2;

        for (const auto& arch : old_archs) {
            if (
                steps.family == arch.at(FAMILY) &&
                steps.extmodel == arch.at(EXTMODEL) &&
                steps.model == arch.at(MODEL)
            ) {
                return true;
            }
        }

        return false;
    };

    // self-explanatory
    if (!(cpu::is_intel() || cpu::is_amd())) {
        return false;
    }

    // intel celeron CPUs are relatively modern, but they can contain a single or odd thread count
    if (cpu::is_celeron(steps)) {
        return false;
    }

    // CPUs before 2006 had no official multi-core processors
    if (old_microarchitecture()) {
        return false;
    }

    // is the count odd?
    return (threads & 1);
#endif
}