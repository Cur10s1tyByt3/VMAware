#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check CPUID output of manufacturer ID for known VMs/hypervisors at leaf 0 and 0x40000000-0x40000100
 * @category x86
 * @implements VM::VMID
 */
[[nodiscard]] bool techniques::vmid() {
#if (!x86)
    return false;
#else
    return (
        cpu::vmid_template(0) ||
        cpu::vmid_template(cpu::leaf::hypervisor) || // 0x40000000
        cpu::vmid_template(cpu::leaf::hypervisor + 0x100) // 0x40000100
    );
#endif
}