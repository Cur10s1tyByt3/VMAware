#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VirtualBox network provider string
 * @category Windows
 * @implements VM::VBOX_NETWORK
 */
[[nodiscard]] static bool vbox_network_share() {
#if (!WINDOWS)
    return false;
#else
    static const bool isVBoxSF = []() -> bool {
        // Try to open the VBoxSF service key
        HKEY hKey = nullptr;
        constexpr LPCSTR kVBoxSFKey =
            "SYSTEM\\CurrentControlSet\\Services\\VBoxSF";

        LONG result = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            kVBoxSFKey,
            0,
            KEY_READ,
            &hKey
        );

        if (result == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }

        return false;
    }();

    return isVBoxSF;
#endif
}