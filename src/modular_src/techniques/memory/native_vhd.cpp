#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
* @brief Checks if the OS was booted from a VHD container
* @category Windows
* @implements VM::NATIVE_VHD
*/
[[nodiscard]] static bool native_vhd() {
#if (!WINDOWS)
    return false;
#else
    #if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
        return false;
    #else
        BOOL isNativeVhdBoot = 0;
        if (IsNativeVhdBoot(&isNativeVhdBoot)) {
            return (isNativeVhdBoot == TRUE);
        }
        return false;
    #endif
#endif
}