#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for SMSW assembly instruction technique
 * @category Windows, x86_32
 * @author Danny Quist from Offensive Computing
 * @implements VM::SMSW
 */
[[nodiscard]] static bool smsw() {
#if (!WINDOWS || !x86_64)
    return false;
#elif (x86_32)
    unsigned int reax = 0;

    __asm
    {
        mov eax, 0xCCCCCCCC;
        smsw eax;
        mov DWORD PTR[reax], eax;
    }

    return (
        (((reax >> 24) & 0xFF) == 0xCC) &&
        (((reax >> 16) & 0xFF) == 0xCC)
    );
#else
    return false;
#endif
}
