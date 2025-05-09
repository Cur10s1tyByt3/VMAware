#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for signatures in leaf 0x40000001 in CPUID
 * @link https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/shared/hvgdk_mini/hv_hypervisor_interface.htm
 * @link https://github.com/ionescu007/SimpleVisor/blob/master/shvvp.c
 * @category x86
 * @implements VM::CPUID_SIGNATURE
 */
[[nodiscard]] static bool cpuid_signature() {
#if (!x86)
    return false;
#else
    u32 eax, unused = 0;
    cpu::cpuid(eax, unused, unused, unused, 0x40000001);
    UNUSED(unused);

    constexpr u32 nanovisor = 0x766E6258; // "Xbnv" 
    constexpr u32 simplevisor = 0x00766853; // " vhS"

    debug("CPUID_SIGNATURE: eax = ", eax);

    if (eax == nanovisor) 
        return core::add(brands::NANOVISOR);
    else if (eax == simplevisor)
        return core::add(brands::SIMPLEVISOR);

    return false;
#endif
}