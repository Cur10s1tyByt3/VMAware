#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VMware memory using IO port backdoor
 * @category Windows, x86_32
 * @note Code from ScoopyNG by Tobias Klein
 * @copyright BSD clause 2
 * @implements VM::VMWARE_PORT_MEM
 */
[[nodiscard]] static bool vmware_port_memory() {
#if (!WINDOWS || !x86_64)
    return false;
#elif (x86_32)
    unsigned int a = 0;

    __try {
        __asm {
            push eax
            push ebx
            push ecx
            push edx

            mov eax, 'VMXh'
            mov ecx, 14h
            mov dx, 'VX'
            in eax, dx
            mov a, eax

            pop edx
            pop ecx
            pop ebx
            pop eax
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    if (a > 0) {
        return core::add(brands::VMWARE);
    }

    return false;
#else
    return false;
#endif
}