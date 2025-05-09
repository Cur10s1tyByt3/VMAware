#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for mutex strings of VM brands
 * @category Windows
 * @note from VMDE project
 * @author hfiref0x
 * @implements VM::MUTEX
 */
[[nodiscard]] static bool mutex() {
#if (!WINDOWS)
    return false;
#else
    auto supMutexExist = [](const char* lpMutexName) -> bool {
        if (lpMutexName == NULL) {
            return false;
        }

        SetLastError(0);
        const HANDLE hObject = CreateMutexA(NULL, FALSE, lpMutexName);
        const DWORD dwError = GetLastError();

        if (hObject) {
            CloseHandle(hObject);
        }

        return (dwError == ERROR_ALREADY_EXISTS);
    };

    if (
        supMutexExist("Sandboxie_SingleInstanceMutex_Control") ||
        supMutexExist("SBIE_BOXED_ServiceInitComplete_Mutex1")
    ) {
        return core::add(brands::SANDBOXIE);
    }

    if (supMutexExist("MicrosoftVirtualPC7UserServiceMakeSureWe'reTheOnlyOneMutex")) {
        return core::add(brands::VPC);
    }

    if (supMutexExist("Frz_State")) { // DeepFreeze
        return true;
    }

    return false;
#endif
}
