#include "cpu.hpp"
#include "memo.hpp"
#include "core.hpp"
#include "../brands.hpp"
#include "util.hpp"
#include <regex>

// cross-platform wrapper function for linux and MSVC cpuid
void cpu::cpuid (
    u32& a, u32& b, u32& c, u32& d,
    const u32 a_leaf,
    const u32 c_leaf
) {
#if (x86)
    // may be unmodified for older 32-bit processors, clearing just in case
    b = 0;
    c = 0;
#if (WINDOWS)
    i32 x[4]{};
    __cpuidex((i32*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
    a = static_cast<u32>(x[0]);
    b = static_cast<u32>(x[1]);
    c = static_cast<u32>(x[2]);
    d = static_cast<u32>(x[3]);
#elif (LINUX || APPLE)
    __cpuid_count(a_leaf, c_leaf, a, b, c, d);
#endif
#else
    return;
#endif
};

// same as above but for array type parameters (MSVC specific)
void cpu::cpuid(
    i32 x[4],
    const u32 a_leaf,
    const u32 c_leaf
) {
#if (x86)
    // may be unmodified for older 32-bit processors, clearing just in case
    x[1] = 0;
    x[2] = 0;
#if (WINDOWS)
    __cpuidex((i32*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
#elif (LINUX || APPLE)
    __cpuid_count(a_leaf, c_leaf, x[0], x[1], x[2], x[3]);
#endif
#else
    return;
#endif
};


// check for maximum function leaf
bool cpu::is_leaf_supported(const u32 p_leaf) {
    u32 eax = 0, unused = 0;

    if (p_leaf < 0x40000000) {
        // Standard range: 0x00000000 - 0x3FFFFFFF
        cpu::cpuid(eax, unused, unused, unused, 0x00000000);
        debug("CPUID: max standard leaf = ", eax);
        return (p_leaf <= eax);
    }
    else if (p_leaf < 0x80000000) {
        // Hypervisor range: 0x40000000 - 0x7FFFFFFF
        cpu::cpuid(eax, unused, unused, unused, cpu::leaf::hypervisor);
        debug("CPUID: max hypervisor leaf = ", eax);
        return (p_leaf <= eax);
    }
    else if (p_leaf < 0xC0000000) {
        // Extended range: 0x80000000 - 0xBFFFFFFF
        cpu::cpuid(eax, unused, unused, unused, cpu::leaf::func_ext);
        debug("CPUID: max extended leaf = ", eax);
        return (p_leaf <= eax);
    }

    debug("CPUID: unsupported leaf range: ", p_leaf);
    return false;
}


// check AMD
[[nodiscard]] bool cpu::is_amd() {
    constexpr u32 amd_ecx = 0x444d4163; // "cAMD"

    u32 unused, ecx = 0;
    cpuid(unused, unused, ecx, unused, 0);

    return (ecx == amd_ecx);
}


// check Intel
[[nodiscard]] bool cpu::is_intel() {
    constexpr u32 intel_ecx1 = 0x6c65746e; // "ntel"
    constexpr u32 intel_ecx2 = 0x6c65746f; // "otel", this is because some Intel CPUs have a rare manufacturer string of "GenuineIotel"

    u32 unused, ecx = 0;
    cpuid(unused, unused, ecx, unused, 0);

    return ((ecx == intel_ecx1) || (ecx == intel_ecx2));
}


// check for POSSIBILITY of hyperthreading, I don't think there's a 
// full-proof method to detect if you're actually hyperthreading imo.
[[nodiscard]] bool cpu::has_hyperthreading() {
    u32 unused, ebx, edx;

    cpuid(unused, ebx, unused, edx, 1);
    UNUSED(unused);

    bool htt_available = (edx & (1 << 28));

    if (!htt_available) {
        return false;
    }

    u32 logical_cores = ((ebx >> 16) & 0xFF);
    i32 physical_cores = 0;

#if (WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    physical_cores = static_cast<i32>(sysinfo.dwNumberOfProcessors);
#elif (LINUX)
    physical_cores = static_cast<i32>(sysconf(_SC_NPROCESSORS_CONF));
#elif (APPLE)
    // sysctlbyname("hw.physicalcpu", &physical_cores, sizeof(physical_cores), NULL, 0);
    // the code under this is the same as the one commented right above, removed due to non-backwards compatibility

    i32 mib[2];
    std::size_t size = sizeof(physical_cores);

    mib[0] = CTL_HW;         // hardware information
    mib[1] = HW_NCPU; // physical CPU count

    if (sysctl(mib, 2, &physical_cores, &size, NULL, 0) != 0) {
        debug("HAS_HYPERTHREADING(): sysctl failed, returned false");
        return false;
    }
#else
    return false;
#endif

    return (logical_cores > static_cast<u32>(physical_cores));
}


// get the CPU product
[[nodiscard]] std::string cpu::get_brand() {
    if (memo::cpu_brand::is_cached()) {
        return memo::cpu_brand::fetch();
    }

#if (!x86)
    return "Unknown";
#else
    if (!cpu::is_leaf_supported(cpu::leaf::brand3)) {
        return "Unknown";
    }

    std::array<u32, 4> buffer{};
    constexpr std::size_t buffer_size = sizeof(i32) * buffer.size();
    std::array<char, 64> charbuffer{};

    std::string brand(48, '\0'); // 3 leafs 16 each

    constexpr std::array<u32, 3> ids = { { cpu::leaf::brand1, cpu::leaf::brand2, cpu::leaf::brand3 } };
    for (const u32& id : ids) {
        cpu::cpuid(buffer.at(0), buffer.at(1), buffer.at(2), buffer.at(3), id);

        std::memcpy(charbuffer.data(), buffer.data(), buffer_size);

        brand.append(charbuffer.data(), 16); 
    }

    debug("BRAND: ", "cpu brand = ", brand);

    memo::cpu_brand::store(brand);

    return brand;
#endif
}


// cpu manufacturer id
[[nodiscard]] std::string cpu::cpu_manufacturer(const u32 p_leaf) {
    auto cpuid_thingy = [](const u32 p_leaf, u32* regs, std::size_t start = 0, std::size_t end = 4) -> bool {
        u32 x[4]{};
        cpu::cpuid(x[0], x[1], x[2], x[3], p_leaf);

        for (; start < end; start++) {
            *regs++ = x[start];
        }

        return true;
        };

    u32 sig_reg[3] = { 0 };

    // Start at index 1 to get EBX, ECX, EDX (x[1], x[2], x[3])
    if (!cpuid_thingy(p_leaf, sig_reg, 1, 4)) {
        return "";
    }

    if ((sig_reg[0] == 0) && (sig_reg[1] == 0) && (sig_reg[2] == 0)) {
        return "";
    }

    auto strconvert = [](u32 n) -> std::string {
        const char* bytes = reinterpret_cast<const char*>(&n);
        return std::string(bytes, 4);
        };

    std::stringstream ss;

    if (p_leaf >= 0x40000000) {
        // Hypervisor vendor string order: EBX, ECX, EDX
        ss << strconvert(sig_reg[0]) << strconvert(sig_reg[1]) << strconvert(sig_reg[2]);
    }
    else {
        // Standard vendor string (leaf 0x0) order: EBX, EDX, ECX
        ss << strconvert(sig_reg[0]) << strconvert(sig_reg[2]) << strconvert(sig_reg[1]);
    }

    return ss.str();
}


[[nodiscard]] cpu::stepping_struct cpu::fetch_steppings() {
    struct stepping_struct steps {};

    u32 unused, eax = 0;
    cpu::cpuid(eax, unused, unused, unused, 1);
    UNUSED(unused);

    steps.model = ((eax >> 4) & 0b1111);
    steps.family = ((eax >> 8) & 0b1111);
    steps.extmodel = ((eax >> 16) & 0b1111);

    return steps;
}


// check if the CPU is an intel celeron
[[nodiscard]] bool cpu::is_celeron(const stepping_struct steps) {
    if (!cpu::is_intel()) {
        return false;
    }

    constexpr u8 celeron_model = 0xA;
    constexpr u8 celeron_family = 0x6;
    constexpr u8 celeron_extmodel = 0x2;

    return (
        steps.model == celeron_model &&
        steps.family == celeron_family &&
        steps.extmodel == celeron_extmodel
    );
}


[[nodiscard]] cpu::model_struct cpu::get_model() {
    const std::string brand = get_brand();

    constexpr const char* intel_i_series_regex = "i[0-9]-[A-Z0-9]{1,7}";
    constexpr const char* intel_xeon_series_regex = "[DEW]-[A-Z0-9]{1,7}";
    constexpr const char* amd_ryzen_regex = "AMD Ryzen ^(PRO)?[A-Z0-9]{1,7}";

    std::string match_str = "";

    auto match = [&](const char* regex) -> bool {
        std::regex pattern(regex);

        auto words_begin = std::sregex_iterator(brand.begin(), brand.end(), pattern);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            match_str = match.str();
        }

        if (!match_str.empty()) {
            return true;
        }

        return false;
    };

    bool found = false;
    bool is_xeon = false;
    bool is_i_series = false;
    bool is_ryzen = false;

    if (cpu::is_intel()) {
        if (match(intel_i_series_regex)) {
            found = true;
            is_i_series = true;
        } else if (match(intel_xeon_series_regex)) {
            found = true;
            is_xeon = true;
        }
    }

    // example: AMD Ryzen 9 3950X 16-Core Processor
    if (cpu::is_amd()) {
        if (match(amd_ryzen_regex)) {
            found = true;
            is_ryzen = true;
        }
    }

    return model_struct{ found, is_xeon, is_i_series, is_ryzen, match_str };
};


[[nodiscard]] bool cpu::vmid_template(const u32 p_leaf) {
#if (CPP >= 17)
    constexpr std::string_view
#else
    const std::string
#endif
        bhyve = "bhyve bhyve ",
        bhyve2 = "BHyVE BHyVE ",
        kvm_hyperv = "Linux KVM Hv",
        qemu = "TCGTCGTCGTCG",
        hyperv = "Microsoft Hv",
        parallels = " prl hyperv ",
        parallels2 = " lrpepyh  vr",
        vmware = "VMwareVMware",
        vbox = "VBoxVBoxVBox",
        xen = "XenVMMXenVMM",
        acrn = "ACRNACRNACRN",
        qnx = " QNXQVMBSQG ",
        qnx2 = "QXNQSBMV",
        nvmm = "___ NVMM ___",
        openbsd_vmm = "OpenBSDVMM58",
        intel_haxm = "HAXMHAXMHAXM",
        unisys = "UnisysSpar64",
        lmhs = "SRESRESRESRE",
        jailhouse = "Jailhouse\0\0\0",
        apple_vz = "Apple VZ",
        intel_kgt = "EVMMEVMMEVMM",
        barevisor = "Barevisor!\0\0",
        hyperplatform = "PpyH",
        minivisor = "MiniVisor\0\0\0",
        intel_tdx = "IntelTDX    ",
        lkvm = "LKVMLKVMLKVM",
        neko = "Neko Project",
        noir = "NoirVisor ZT";

    const std::string brand_str = cpu_manufacturer(p_leaf);

#ifdef __VMAWARE_DEBUG__
    const char* technique_name;
    switch (p_leaf) {
    case 0x40000000:
        technique_name = "VMID_0x4: ";
        break;
    case 0x40000100:
        technique_name = "VMID_0x4 + 0x100: ";
        break;
    case 0x40000001:
        technique_name = "VMID_0x4 + 1: ";
        break;
    default:
        technique_name = "VMID: ";
        break;
    }
    debug(technique_name, brand_str);
#endif
    // both Hyper-V and VirtualPC have the same string value
    if (brand_str == hyperv) {
        if (util::hyper_x() == HYPERV_ARTIFACT_VM) {
            return false;
        }
        return core::add(brands::HYPERV, brands::VPC);
    }

    /**
        * this is added because there are inconsistent string
        * values for KVM's manufacturer ID. For example,
        * it gives me "KVMKMVMKV" when I run it under QEMU
        * but the Wikipedia article on CPUID says it's
        * "KVMKVMKVM\0\0\0", like wtf????
        */
    if (util::find(brand_str, "KVM")) {
        return core::add(brands::KVM);
    }

    if (brand_str == vmware) { return core::add(brands::VMWARE); }
    if (brand_str == vbox) { return core::add(brands::VBOX); }
    if (brand_str == qemu) { return core::add(brands::QEMU); }
    if (brand_str == xen) { return core::add(brands::XEN); }
    if (brand_str == kvm_hyperv) { return core::add(brands::KVM_HYPERV); }
    if (brand_str == parallels) { return core::add(brands::PARALLELS); }
    if (brand_str == parallels2) { return core::add(brands::PARALLELS); }
    if (brand_str == bhyve) { return core::add(brands::BHYVE); }
    if (brand_str == bhyve2) { return core::add(brands::BHYVE); }
    if (brand_str == acrn) { return core::add(brands::ACRN); }
    if (brand_str == qnx) { return core::add(brands::QNX); }
    if (brand_str == nvmm) { return core::add(brands::NVMM); }
    if (brand_str == openbsd_vmm) { return core::add(brands::BSD_VMM); }
    if (brand_str == intel_haxm) { return core::add(brands::INTEL_HAXM); }
    if (brand_str == unisys) { return core::add(brands::UNISYS); }
    if (brand_str == lmhs) { return core::add(brands::LMHS); }
    if (brand_str == jailhouse) { return core::add(brands::JAILHOUSE); }
    if (brand_str == intel_kgt) { return core::add(brands::INTEL_KGT); }
    if (brand_str == barevisor) { return core::add(brands::BAREVISOR); }
    if (brand_str == minivisor) { return core::add(brands::MINIVISOR); }
    if (brand_str == intel_tdx) { return core::add(brands::INTEL_TDX); }
    if (brand_str == lkvm) { return core::add(brands::LKVM); }
    if (brand_str == neko) { return core::add(brands::NEKO_PROJECT); }
    if (brand_str == noir) { return core::add(brands::NOIRVISOR); }

    /**
        * i'm honestly not sure about this one,
        * they're supposed to have 12 characters but
        * Wikipedia tells me it these brands have
        * less characters (both 8), so i'm just
        * going to scan for the entire string ig
        */
#if (CPP >= 17)
    const char* qnx_sample = qnx2.data();
    const char* applevz_sample = apple_vz.data();
#else
    const char* qnx_sample = qnx2.c_str();
    const char* applevz_sample = apple_vz.c_str();
#endif

    if (util::find(brand_str, qnx_sample)) {
        return core::add(brands::QNX);
    }

    if (util::find(brand_str, applevz_sample)) {
        return core::add(brands::APPLE_VZ);
    }

    if (util::find(brand_str, hyperplatform.data())) {
        return core::add(brands::HYPERPLATFORM);
    }

    return false;
}