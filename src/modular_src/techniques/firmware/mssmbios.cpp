#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check MSSMBIOS registry for VM-specific signatures
 * @category Windows
 * @implements VM::MSSMBIOS
 */
[[nodiscard]] static bool mssmbios() {
#if (!WINDOWS)
    return false;
#else
    struct PatternMeta {
        const char* str;
        size_t length;
    };

    struct CheckConfig {
        const char* value_name;
        std::array<PatternMeta, 7> patterns;  // Size for largest pattern set
        const char* debug_prefix;
        size_t actual_pattern_count;
    };

    constexpr CheckConfig checks[] = {
        {   // SMBiosData check (must come first)
            "SMBiosData",
            {{
                {"INNOTEK GMBH", 12},
                {"VIRTUALBOX", 10},
                {"SUN MICROSYSTEMS", 16},
                {"VBOXVER", 7},
                {"VIRTUAL MACHINE", 15},
                {"VMWARE", 6},
                {"GOOGLE COMPUTE ENGINE", 20}
            }},
            "SMBIOS",
            7  // Actual number of patterns
        },
        {   // AcpiData check
            "AcpiData",
            {{
                {"INNOTEK GMBH", 12},
                {"VBOXAPIC", 8},
                {"SUN MICROSYSTEMS", 16},
                {"VBOXVER", 7},
                {"VIRTUAL MACHINE", 15},
                {"VMWARE", 6},
                {}  // Padding
            }},
            "ACPI",
            6  // Actual number of patterns
        }
    };

    HKEY hk = nullptr;
    bool detected = false;

    for (const auto& config : checks) {
        LSTATUS ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data"),
            0, KEY_READ, &hk);

        if (ret != ERROR_SUCCESS) {
            debug(config.debug_prefix, ": Registry open failed");
            continue;
        }

        DWORD type = 0;
        DWORD length = 0;

        ret = RegQueryValueEx(hk, _T(config.value_name), nullptr, &type, nullptr, &length);
        if (ret != ERROR_SUCCESS || length == 0) {
            RegCloseKey(hk);
            debug(config.debug_prefix, ": Data size query failed");
            continue;
        }

        constexpr DWORD kStackThreshold = 4096;
        char stack_buffer[kStackThreshold] = { 0 };
        bool heap_allocated = false;
        char* buffer = nullptr;

        if (length <= kStackThreshold) {
            buffer = stack_buffer;
        }
        else {
            buffer = static_cast<char*>(LocalAlloc(LPTR, length));
            if (!buffer) {
                RegCloseKey(hk);
                debug(config.debug_prefix, ": Memory allocation failed");
                continue;
            }
            heap_allocated = true;
        }

        ret = RegQueryValueEx(hk, _T(config.value_name), nullptr, &type,
            reinterpret_cast<LPBYTE>(buffer), &length);

        if (ret != ERROR_SUCCESS) {
            if (heap_allocated) LocalFree(buffer);
            RegCloseKey(hk);
            debug(config.debug_prefix, ": Data read failed");
            continue;
        }

        for (DWORD i = 0; i < length; ++i) {
            buffer[i] = (buffer[i] >= 'a' && buffer[i] <= 'z')
                ? static_cast<char>(buffer[i] - 32)
                : buffer[i];
        }

        for (DWORD i = 0; i < length; ++i) {
            const char current = buffer[i];

            for (size_t p = 0; p < config.actual_pattern_count; ++p) {
                const auto& pattern = config.patterns[p];

                if (current != pattern.str[0]) continue;
                if (i + pattern.length > length) continue;

                if (memcmp(buffer + i, pattern.str, pattern.length) == 0) {
                    detected = true;
                    goto cleanup;
                }
            }
        }

    cleanup:
        if (heap_allocated) LocalFree(buffer);
        RegCloseKey(hk);

        if (detected && (&config == &checks[0])) break;
        if (detected) return true;
    }

    return detected;
#endif
}