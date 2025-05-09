#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"



/**
 * @brief Check for sgdt instruction method
 * @category Windows
 * @author Danny Quist (chamuco@gmail.com) (top-most byte signature)
 * @author Val Smith (mvalsmith@metasploit.com) (top-most byte signature)
 * @note code documentation paper in /papers/www.offensivecomputing.net_vm.pdf (top-most byte signature)
 * @implements VM::SGDT
 */
[[nodiscard]] static bool sgdt() {
#if (WINDOWS)
    #if defined(_M_X64) || defined(__x86_64__)
        unsigned char gdtr[10] = { 0 };
    #else
        unsigned char gdtr[6] = { 0 };
    #endif

    unsigned int  gdt = 0;

    __try {
        #if (CLANG || GCC)
            __asm__ volatile("sgdt %0" : "=m"(gdtr));
        #elif (MSVC && x86_32)
            __asm {
                sgdt gdtr
            }
        #elif (MSVC)
            #pragma pack(push, 1)
            struct {
                unsigned short limit;
                unsigned long long base;
            } _gdtr = {};
            #pragma pack(pop)

            _sgdt(&_gdtr);
            std::memcpy(gdtr, &_gdtr, sizeof(gdtr));
        #else
            return false;
        #endif
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false; // umip
    }

    // 32-bit base from bytes [2..5]
    std::memcpy(&gdt, &gdtr[2], sizeof(gdt));

    // On x64, gdtr[5] is the top byte of the 64-bit base; on x86 it's high byte of 32-bit base
    if (gdtr[5] > 0xD0) {
        debug("SGDT: top-most byte signature detected");
        return true;
    }

    // 0xFF signature in the high byte of the 32-bit base
    return ((gdt >> 24) == 0xFF);
#else
    return false;
#endif
    }