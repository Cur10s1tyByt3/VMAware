#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for number of logical processors
 * @category Windows
 * @implements VM::PROCESSOR_NUMBER
 */
[[nodiscard]] static bool processor_number()
{
#if (!WINDOWS)
    return false;
#else
    #if (x86_32)
        const PULONG ulNumberProcessors = (PULONG)(__readfsdword(0x30) + 0x64);
    #else
        const PULONG ulNumberProcessors = (PULONG)(__readgsqword(0x60) + 0xB8);
    #endif

    if (*ulNumberProcessors < 4)
        return true;
    else
        return false;
#endif
}