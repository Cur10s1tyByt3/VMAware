#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for sldt instruction method
 * @category Windows, x86_32
 * @author Danny Quist (chamuco@gmail.com), ldtr_buf signature
 * @author Val Smith (mvalsmith@metasploit.com), ldtr_buf signature
 * @note code documentation paper in /papers/www.offensivecomputing.net_vm.pdf for ldtr_buf signature
 * @note code documentation paper in https://www.aldeid.com/wiki/X86-assembly/Instructions/sldt for ldt signature
 * @implements VM::SLDT
 */
[[nodiscard]] static bool sldt() {
#if (WINDOWS && x86_32)
    unsigned char ldtr_buf[4] = { 0xEF, 0xBE, 0xAD, 0xDE };
    unsigned long ldt = 0;

    __try {
#if (CLANG || GCC)
        __asm__ volatile("sldt %0" : "=m"(*(unsigned short*)ldtr_buf));
#elif (MSVC)
        __asm {
            sldt ax
            mov  word ptr[ldtr_buf], ax
        }
#endif
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false; // umip
    }

    std::memcpy(&ldt, ldtr_buf, sizeof(ldt));
    if (ldtr_buf[0] != 0x00 && ldtr_buf[1] != 0x00) {
        debug("SLDT: ldtr_buf signature detected");
        return true;
    }

    return (ldt != 0xDEAD0000);
#else
    return false;
#endif
}