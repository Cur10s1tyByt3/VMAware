#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for AMD-SEV MSR running on the system
 * @category x86, Linux, MacOS
 * @note idea from virt-what
 * @implements VM::AMD_SEV
 */
[[nodiscard]] static bool amd_sev() {
#if (x86 && (LINUX || APPLE))
    if (!cpu::is_amd()) {
        return false;
    }
    
    if (!util::is_admin()) {
        return false;
    }

    constexpr u32 encrypted_memory_capability = 0x8000001f;
    constexpr u32 msr_index = 0xc0010131;	  
    
    if (!cpu::is_leaf_supported(encrypted_memory_capability)) {
        return false;
    }
    
    u32 eax, unused = 0;
    cpu::cpuid(eax, unused, unused, unused, encrypted_memory_capability);
        
    if (!(eax & (1 << 1))) {
        return false;
    }       

    u64 result = 0;
    
    const std::string msr_device = "/dev/cpu/0/msr";
    std::ifstream msr_file(msr_device, std::ios::binary);

    if (!msr_file.is_open()) {
        debug("AMD_SEV: unable to open MSR file");
        return false;
    }

    msr_file.seekg(msr_index);
    msr_file.read(reinterpret_cast<char*>(&result), sizeof(result));

    if (!msr_file) {
        debug("AMD_SEV: unable to open MSR file");
        return false;
    }

    if (result & (static_cast<unsigned long long>(1) << 2)) { return core::add(brands::AMD_SEV_SNP); }
    else if (result & (static_cast<unsigned long long>(1) << 1)) { return core::add(brands::AMD_SEV_ES); }
    else if (result & 1) { return core::add(brands::AMD_SEV); }

    return false;
#else
    return false;
#endif
}