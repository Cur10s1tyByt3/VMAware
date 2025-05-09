#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VM-specific DLLs
 * @category Windows
 * @implements VM::DLL
 */
[[nodiscard]] static bool dll_check() {
#if (!WINDOWS)
    return false;
#else
    static constexpr struct {
        const char* dll_name;
        const char* brand;
    } dll_checks[] = {
        {"sbiedll.dll",   brands::SANDBOXIE},
        {"pstorec.dll",   brands::CWSANDBOX},
        {"vmcheck.dll",   brands::VPC},
        {"cmdvrt32.dll",  brands::COMODO},
        {"cmdvrt64.dll",  brands::COMODO},
        {"cuckoomon.dll", brands::CUCKOO},
        {"SxIn.dll",      brands::QIHOO},
        {"wpespy.dll",    brands::NULL_BRAND}
    };

    for (const auto& check : dll_checks) {
        if (GetModuleHandle(_T(check.dll_name)) != nullptr) {
            debug("DLL: Found ", check.dll_name, " (", check.brand, ")");
            return core::add(check.brand);
        }
    }

    return false;
#endif
}