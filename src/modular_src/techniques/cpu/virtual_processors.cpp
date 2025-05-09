#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if the number of virtual and logical processors are reported correctly by the system
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::VIRTUAL_PROCESSORS
 */
[[nodiscard]] static bool virtual_processors() {
#if (!WINDOWS)
    return false;
#else   
    if (!cpu::is_leaf_supported(0x40000005)) {
        return false;
    }
    
    struct Registers {
        int eax = 0;
        int ebx = 0;
        int ecx = 0;
        int edx = 0;
    };
    struct ImplementationLimits {
        unsigned int MaxVirtualProcessors = 0;
        unsigned int MaxLogicalProcessors = 0;
    };

    Registers registers;
    __cpuid(&registers.eax, 0x40000005);

    ImplementationLimits implementationLimits;
    implementationLimits.MaxVirtualProcessors = static_cast<unsigned int>(registers.eax);
    implementationLimits.MaxLogicalProcessors = static_cast<unsigned int>(registers.ebx);

    debug("VIRTUAL_PROCESSORS: MaxVirtualProcessors: ", implementationLimits.MaxVirtualProcessors,
        ", MaxLogicalProcessors: ", implementationLimits.MaxLogicalProcessors);

    if (implementationLimits.MaxVirtualProcessors == 0xffffffff || implementationLimits.MaxLogicalProcessors == 0) {
        return true;
    }

    return false;
#endif
}