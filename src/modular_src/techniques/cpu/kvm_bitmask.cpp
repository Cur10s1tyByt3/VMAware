#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for KVM CPUID bitmask range for reserved values
 * @category x86
 * @implements VM::KVM_BITMASK
 */
[[nodiscard]] static bool kvm_bitmask() {
#if (!x86)
    return false;
#else
    u32 eax, ebx, ecx, edx = 0;
    cpu::cpuid(eax, ebx, ecx, edx, 0x40000000);

    // KVM brand and max leaf check
    if (!(
        (eax == 0x40000001) &&
        (ebx == 0x4b4d564b) &&
        (ecx == 0x564b4d56) &&
        (edx == 0x4d)
    )) {
        return false;
    }

    cpu::cpuid(eax, ebx, ecx, edx, 0x40000001);

    if (
        (eax & (1 << 8)) &&
        (((eax >> 13) & 0b1111111111) == 0) &&
        ((eax >> 24) == 0)
    ) {
        return core::add(brands::KVM);
    }

    return false;
#endif
}
