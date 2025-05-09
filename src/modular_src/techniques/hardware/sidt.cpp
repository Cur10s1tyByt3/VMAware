#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for uncommon IDT virtual addresses
 * @author Matteo Malvica (Linux)
 * @link https://www.matteomalvica.com/blog/2018/12/05/detecting-vmware-on-64-bit-systems/ (Linux)
 * @note Idea to check VPC's range from Tom Liston and Ed Skoudis' paper "On the Cutting Edge: Thwarting Virtual Machine Detection" (Windows)
 * @note Paper situated at /papers/ThwartingVMDetection_Liston_Skoudis.pdf (Windows)
 * @category Windows, Linux
 * @implements VM::SIDT
 */
[[nodiscard]] static bool sidt() {
#if (LINUX && (GCC || CLANG))
    u8 values[10] = { 0 };

    fflush(stdout);

    #if x86_64
        // 64-bit Linux: IDT descriptor is 10 bytes (2-byte limit + 8-byte base)
        __asm__ __volatile__("sidt %0" : "=m"(values));

    #ifdef __VMAWARE_DEBUG__
        debug("SIDT5: values = ");
        for (u8 i = 0; i < 10; ++i) {
            debug(std::hex, std::setw(2), std::setfill('0'), static_cast<unsigned>(values[i]));
            if (i < 9) debug(" ");
        }
    #endif

        return (values[9] == 0x00);  // 10th byte in x64 mode

    #elif x86_32
        // 32-bit Linux: IDT descriptor is 6 bytes (2-byte limit + 4-byte base)
        __asm__ __volatile__("sidt %0" : "=m"(values));

    #ifdef __VMAWARE_DEBUG__
        debug("SIDT5: values = ");
        for (u8 i = 0; i < 6; ++i) {
            debug(std::hex, std::setw(2), std::setfill('0'), static_cast<unsigned>(values[i]));
            if (i < 5) debug(" ");
        }
    #endif

        return (values[5] == 0x00);  // 6th byte in x86 mode

    #else
        return false;
    #endif

#elif (WINDOWS)
    // Clang/GCC on x64 emits a full 10-byte SIDT (16-bit limit + 64-bit base), on 32-bit it still only writes 6 bytes
    #if defined(_M_X64) || defined(__x86_64__)
        unsigned char m[10] = { 0 };
    #else
        unsigned char m[6] = { 0 };
    #endif

    u32 idt = 0;

    __try {
        #if (CLANG || GCC)
            __asm__ volatile ("sidt %0" : "=m"(m));
        #elif (MSVC && x86_32)
            __asm {
                sidt m
            }
        #elif (MSVC)
            #pragma pack(push, 1)
            struct {
                unsigned short limit;
                unsigned long long base;
            } idtr = {};
            #pragma pack(pop)

            __sidt(&idtr);
            std::memcpy(m, &idtr, sizeof(m));
        #else
            return false;
        #endif
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false; // umip
    }

    // Extract 32-bit base from bytes [2..5]
    idt = *((unsigned long*)&m[2]);

    if ((idt >> 24) == 0xE8) {
        return core::add(brands::VPC);
    }

    // On x64, m[5] is the top byte of the 64-bit base; on x86 it's high byte of 32-bit base
    return (m[5] > 0xD0);
#endif
}