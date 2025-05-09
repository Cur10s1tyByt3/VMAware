#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if the CPU manufacturer is not known
 * @category x86
 * @implements VM::UNKNOWN_MANUFACTURER
 */
[[nodiscard]] static bool unknown_manufacturer() {
    constexpr std::array<const char*, 21> known_ids = { {
        "AuthenticAMD", "CentaurHauls", "CyrixInstead",
        "GenuineIntel", "GenuineIotel", "TransmetaCPU",
        "GenuineTMx86", "Geode by NSC", "NexGenDriven",
        "RiseRiseRise", "SiS SiS SiS ", "UMC UMC UMC ",
        "Vortex86 SoC", "  Shanghai  ", "HygonGenuine",
        "Genuine  RDC", "E2K MACHINE", "VIA VIA VIA ",
        "AMD ISBETTER", "GenuineAO486", "MiSTer AO486"
    } };

    const std::string brand = cpu::cpu_manufacturer(0);
    for (const char* id : known_ids) {
        if (memcmp(brand.data(), id, 12) == 0) {
            return false;
        }
    }

    debug("UNKNOWN_MANUFACTURER: CPU brand '", brand, "' did not match known vendor IDs.");
    return true; // no known manufacturer matched, likely a VM
}