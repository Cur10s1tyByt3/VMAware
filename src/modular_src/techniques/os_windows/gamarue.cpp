#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for Gamarue ransomware technique which compares VM-specific Window product IDs
 * @category Windows
 * @implements VM::GAMARUE
 */
[[nodiscard]] static bool gamarue() {
#if (!WINDOWS)
    return false;
#else
    HKEY hKey;
    char buffer[64] = { 0 };
    DWORD dwSize = sizeof(buffer);
    LONG lRes;

    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        _T("Software\\Microsoft\\Windows\\CurrentVersion"),
        0,
        KEY_QUERY_VALUE,
        &hKey);

    if (lRes != ERROR_SUCCESS) return false;

    lRes = RegQueryValueEx(hKey, _T("ProductId"),
        nullptr, nullptr,
        reinterpret_cast<LPBYTE>(buffer), &dwSize);

    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS) return false;

    struct TargetPattern {
        const char* product_id;
        const char* brand;
    };

    constexpr TargetPattern targets[] = {
        {"55274-640-2673064-23950", "JOEBOX"},
        {"76487-644-3177037-23510", "CWSANDBOX"},
        {"76487-337-8429955-22614", "ANUBIS"}
    };

    constexpr size_t target_len = 21;

    if (strlen(buffer) != target_len) return false;

    for (const auto& target : targets) {
        if (memcmp(buffer, target.product_id, target_len) == 0) {
            return core::add(target.brand);
        }
    }

    return false;
#endif
}