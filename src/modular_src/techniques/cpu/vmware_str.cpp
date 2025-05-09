#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check str assembly instruction method for VMware
 * @note Alfredo Omella's (S21sec) STR technique
 * @note paper describing this technique is located at /papers/www.s21sec.com_vmware-eng.pdf (2006)
 * @category Windows
 * @implements VM::VMWARE_STR
 */
[[nodiscard]] static bool vmware_str() {
#if (WINDOWS && x86_32)
    unsigned short tr = 0;
    __asm {
        str ax
        mov tr, ax
    }
    if ((tr & 0xFF) == 0x00 && ((tr >> 8) & 0xFF) == 0x40) {
        return core::add(brands::VMWARE);
    }
#endif
    return false;
}
