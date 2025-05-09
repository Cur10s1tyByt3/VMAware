#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for official VMware io port backdoor technique
 * @category Windows, x86_32
 * @note Code from ScoopyNG by Tobias Klein
 * @note Technique founded by Ken Kato
 * @copyright BSD clause 2
 * @implements VM::VMWARE_BACKDOOR
 */
[[nodiscard]] static bool vmware_backdoor() {
#if (WINDOWS && x86_32 && !CLANG)
    u32 a = 0;
    u32 b = 0;

    constexpr std::array<i16, 2> ioports = { { 'VX' , 'VY' } };
    i16 ioport;
    bool is_vm = false;

    for (u8 i = 0; i < ioports.size(); ++i) {
        ioport = ioports[i];
        for (u8 cmd = 0; cmd < 0x2c; ++cmd) {
            __try {
                __asm {
                    push eax
                    push ebx
                    push ecx
                    push edx

                    mov eax, 'VMXh'
                    movzx ecx, cmd
                    mov dx, ioport
                    in eax, dx      // <- key point is here

                    mov a, ebx
                    mov b, ecx

                    pop edx
                    pop ecx
                    pop ebx
                    pop eax
                }

                is_vm = true;
                break;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    if (is_vm) {
        switch (b) {
            case 1:  return core::add(brands::VMWARE_EXPRESS);
            case 2:  return core::add(brands::VMWARE_ESX);
            case 3:  return core::add(brands::VMWARE_GSX);
            case 4:  return core::add(brands::VMWARE_WORKSTATION);
            default: return core::add(brands::VMWARE);
        }
    }

    return false;
#else
    return false;
#endif
}