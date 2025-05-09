#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"


/**
 * @brief Check if hypervisor feature bit in CPUID eax bit 31 is enabled (always false for physical CPUs)
 * @category x86
 * @implements VM::HYPERVISOR_BIT
 */
[[nodiscard]] bool techniques::hypervisor_bit() {
#if (!x86)
    return false;
#else
    if (util::hyper_x() == HYPERV_ARTIFACT_VM) {
        return false;
    }

    u32 unused, ecx = 0;
    cpu::cpuid(unused, unused, ecx, unused, 1);

    return (ecx & (1 << 31));
#endif
}