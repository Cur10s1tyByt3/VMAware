#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for hypervisor brand string length (would be around 2 characters in a host machine)
 * @category x86
 * @implements VM::HYPERVISOR_STR
 */
[[nodiscard]] static bool hypervisor_str() {
#if (!x86)
    return false;
#else
    if (util::hyper_x() == HYPERV_ARTIFACT_VM) {
        return false;
    }

    char out[sizeof(i32) * 4 + 1] = { 0 }; // e*x size + number of e*x registers + null terminator
    cpu::cpuid((int*)out, cpu::leaf::hypervisor);

    debug("HYPERVISOR_STR: \neax: ", static_cast<u32>(out[0]),
        "\nebx: ", static_cast<u32>(out[1]),
        "\necx: ", static_cast<u32>(out[2]),
        "\nedx: ", static_cast<u32>(out[3])
    );

    return (std::strlen(out + 4) >= 4);
#endif
}