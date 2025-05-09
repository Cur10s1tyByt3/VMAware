#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../modules/core.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for Intel KGT (Trusty branch) hypervisor signature in CPUID
 * @link https://github.com/intel/ikgt-core/blob/7dfd4d1614d788ec43b02602cce7a272ef8d5931/vmm/vmexit/vmexit_cpuid.c
 * @category x86
 * @implements VM::KGT_SIGNATURE
 */
[[nodiscard]] static bool intel_kgt_signature() {
#if (!x86)
    return false;
#else
    u32 unused, ecx, edx = 0;
    cpu::cpuid(unused, unused, ecx, edx, 3);

    /*
        * ECX = 0x4D4M5645 = "EVMM" (E=0x45, V=0x56, M=0x4D)
        * EDX = 0x43544E49 = "INTC" (I=0x49, N=0x4E, T=0x54, C=0x43)
        */
    if ((ecx == 0x4D4D5645) && (edx == 0x43544E49)) {
        return core::add(brands::INTEL_KGT);
    }

    return false;
#endif
}