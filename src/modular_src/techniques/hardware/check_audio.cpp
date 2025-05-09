#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if no waveform-audio output devices are present in the system
 * @category Windows
 * @implements VM::AUDIO
 */
[[nodiscard]] static bool check_audio() {
#if (!WINDOWS)
    return false;
#else
    HKEY hKey = nullptr;
    LONG err = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        R"(SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render)",
        0,
        KEY_READ | KEY_WOW64_64KEY,
        &hKey
    );

    if (err != ERROR_SUCCESS) {
        return true;  
    }

    DWORD subKeyCount = 0;
    RegQueryInfoKey(
        hKey,
        nullptr,   
        nullptr,    
        nullptr,    
        &subKeyCount,  
        nullptr,    
        nullptr,   
        nullptr,    
        nullptr,    
        nullptr,   
        nullptr,    
        nullptr     
    );

    RegCloseKey(hKey);

    return subKeyCount == 0;
#endif
}