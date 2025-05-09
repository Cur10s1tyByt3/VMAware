#pragma once

#include "enums.hpp"
#include "arg_handle.hpp"
#include "globals.hpp"
#include "brands.hpp"
#include "modules/memo.hpp"
#include "modules/core.hpp"
#include "modules/util.hpp"
#include <sstream>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <iostream>


/**
 * @brief Check for a specific technique based on flag argument
 * @param u8 (flags from VM wrapper)
 * @return bool
 * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmcheck
 */
static bool check(
    const enum_flags flag_bit, 
    const enum_flags memo_arg = NULL_ARG
    // clang doesn't support std::source_location for some reason
#if (CPP >= 20 && !CLANG)
    , const std::source_location& loc = std::source_location::current()
#endif
) {
    // lambda to manage exceptions
    auto throw_error = [&](const char* text) -> void {
        std::stringstream ss;
#if (CPP >= 20 && !CLANG)
        ss << ", error in " << loc.function_name() << " at " << loc.file_name() << ":" << loc.line() << ")";
#endif
        ss << ". Consult the documentation's flag handler for VM::check()";
        throw std::invalid_argument(std::string(text) + ss.str());
    };

    // check if flag is out of range
    if (flag_bit > enum_size) {
        throw_error("Flag argument must be a valid");
    }

    // check if the bit is a settings flag, which shouldn't be allowed
    if (
        (flag_bit == NO_MEMO) ||
        (flag_bit == HIGH_THRESHOLD) ||
        (flag_bit == DYNAMIC) ||
        (flag_bit == MULTIPLE)
    ) {
        throw_error("Flag argument must be a technique flag and not a settings flag");
    }

    if (
        (memo_arg != NO_MEMO) && 
        (memo_arg != NULL_ARG)
    ) {
        throw_error("Flag argument for memoization must be either VM::NO_MEMO or left empty");
    }

    const bool is_memoized = (memo_arg != NO_MEMO);

#if (CPP >= 23)
    [[assume(flag_bit < technique_end)]];
#endif

    // if the technique is already cached, return the cached value instead
    if (memo::is_cached(flag_bit) && is_memoized) {
        const memo::data_t data = memo::cache_fetch(flag_bit);
        return data.result;
    }

    // check if the flag even exists
    auto it = core::technique_table.find(flag_bit);
    if (it == core::technique_table.end()) {
        throw_error("Flag is not known");
    }

    // initialise and run the technique
    const core::technique& pair = it->second;
    const bool result = pair.run();

    if (result) {
        globals::detected_count_num++;
    }

#ifdef __VMAWARE_DEBUG__
    total_points += pair.points;
#endif

    // store the technique result in the cache table
    if (is_memoized) {
        memo::cache_store(flag_bit, result, pair.points);
    }

    return result;
}


/**
    * @brief Fetch the VM brand
    * @param any flag combination in VM structure or nothing (VM::MULTIPLE can be added)
    * @return std::string
    * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmbrand
    */
template <typename ...Args>
static std::string brand(Args ...args) {
    flagset flags = arg::arg_handler(args...);

    // is the multiple setting flag enabled? (meaning multiple 
    // brand strings will be outputted if there's a conflict)
    const bool is_multiple = core::is_enabled(flags, MULTIPLE);

    // used for later
    u16 score = 0;

    // are all the techiques already run? if not, run them 
    // to fetch the necessary info to determine the brand
    if (!memo::all_present() || core::is_enabled(flags, NO_MEMO)) {
        score = core::run_all(flags);
    }

    // check if the result is already cached and return that instead
    if (core::is_disabled(flags, NO_MEMO)) {
        if (is_multiple) {
            if (memo::multi_brand::is_cached()) {
                core_debug("VM::brand(): returned multi brand from cache");
                return memo::multi_brand::fetch();
            }
        } else {
            if (memo::brand::is_cached()) {
                core_debug("VM::brand(): returned brand from cache");
                return memo::brand::fetch();
            }
        }
    }

    // goofy ass C++11 and C++14 linker error workaround.
    // And yes, this does look stupid.
#if (CPP <= 14)
    constexpr const char* TMP_QEMU = "QEMU";
    constexpr const char* TMP_KVM = "KVM";
    constexpr const char* TMP_QEMU_KVM = "QEMU+KVM";
    constexpr const char* TMP_KVM_HYPERV = "KVM Hyper-V Enlightenment";
    constexpr const char* TMP_QEMU_KVM_HYPERV = "QEMU+KVM Hyper-V Enlightenment";

    constexpr const char* TMP_VMWARE = "VMware";
    constexpr const char* TMP_VMWARE_HARD = "VMware (with VmwareHardenedLoader)";
    constexpr const char* TMP_EXPRESS = "VMware Express";
    constexpr const char* TMP_ESX = "VMware ESX";
    constexpr const char* TMP_GSX = "VMware GSX";
    constexpr const char* TMP_WORKSTATION = "VMware Workstation";
    constexpr const char* TMP_FUSION = "VMware Fusion";

    constexpr const char* TMP_VPC = "Virtual PC";
    constexpr const char* TMP_HYPERV = "Microsoft Hyper-V";
    constexpr const char* TMP_HYPERV_VPC = "Microsoft Virtual PC/Hyper-V";
    constexpr const char* TMP_AZURE = "Microsoft Azure Hyper-V";
    constexpr const char* TMP_NANOVISOR = "Xbox NanoVisor (Hyper-V)";
    constexpr const char* TMP_HYPERV_ARTIFACT = "Hyper-V artifact (not an actual VM)";
#else
    constexpr const char* TMP_QEMU = brands::QEMU;
    constexpr const char* TMP_KVM = brands::KVM;
    constexpr const char* TMP_QEMU_KVM = brands::QEMU_KVM;
    constexpr const char* TMP_KVM_HYPERV = brands::KVM_HYPERV;
    constexpr const char* TMP_QEMU_KVM_HYPERV = brands::QEMU_KVM_HYPERV;

    constexpr const char* TMP_VMWARE = brands::VMWARE;
    constexpr const char* TMP_VMWARE_HARD = brands::VMWARE_HARD;
    constexpr const char* TMP_EXPRESS = brands::VMWARE_EXPRESS;
    constexpr const char* TMP_ESX = brands::VMWARE_ESX;
    constexpr const char* TMP_GSX = brands::VMWARE_GSX;
    constexpr const char* TMP_WORKSTATION = brands::VMWARE_WORKSTATION;
    constexpr const char* TMP_FUSION = brands::VMWARE_FUSION;

    constexpr const char* TMP_VPC = brands::VPC;
    constexpr const char* TMP_HYPERV = brands::HYPERV;
    constexpr const char* TMP_HYPERV_VPC = brands::HYPERV_VPC;
    constexpr const char* TMP_AZURE = brands::AZURE_HYPERV;
    constexpr const char* TMP_NANOVISOR = brands::NANOVISOR;
    constexpr const char* TMP_HYPERV_ARTIFACT = brands::HYPERV_ARTIFACT;
#endif

    // this is where all the RELEVANT brands are stored.
    // The ones with no points will be filtered out.
    std::map<const char*, brand_score_t> brands;

    // add the relevant brands with at least 1 point
    for (const auto &element : core::brand_scoreboard) {
        if (element.second > 0) {
            brands.insert(std::make_pair(element.first, element.second));
        }
    }

    // if all brands have a point of 0, return 
    // "Unknown" (no relevant brands were found)
    if (brands.empty()) {
        return brands::NULL_BRAND;
    }

    // if there's only a single brand, return it. 
    // This will skip the rest of the function
    // where it will process and merge certain
    // brands 
    if (brands.size() == 1) {
        return brands.begin()->first;
    }
    
    // remove Hyper-V artifacts if found with other 
    // brands, because that's not a VM. It's added 
    // only for the sake of information cuz of the 
    // fucky wucky Hyper-V problem (see Hyper-X)
    if (brands.size() > 1) {
        if (brands.find(TMP_HYPERV_ARTIFACT) != brands.end()) {
            brands.erase(TMP_HYPERV_ARTIFACT);
        }
    }

    // merge 2 brands, and make a single brand out of it.
    auto merge = [&](const char* a, const char* b, const char* result) -> void {
        if (
            (brands.count(a) > 0) &&
            (brands.count(b) > 0)
        ) {
            brands.erase(a);
            brands.erase(b);
            brands.emplace(std::make_pair(result, 2));
        }
    };

    // same as above, but for 3
    auto triple_merge = [&](const char* a, const char* b, const char* c, const char* result) -> void {
        if (
            (brands.count(a) > 0) &&
            (brands.count(b) > 0) &&
            (brands.count(c) > 0)
        ) {
            brands.erase(a);
            brands.erase(b);
            brands.erase(c);
            brands.emplace(std::make_pair(result, 2));
        }
    };


    // some edgecase handling for Hyper-V and VirtualPC since
    // they're very similar, and they're both from Microsoft (ew)
    if ((brands.count(TMP_HYPERV) > 0) && (brands.count(TMP_VPC) > 0)) {
        if (brands.count(TMP_HYPERV) == brands.count(TMP_VPC)) {
            merge(TMP_VPC, TMP_HYPERV, TMP_HYPERV_VPC);
        } else {
            brands.erase(TMP_VPC);
        }
    }
    

    // this is the section where brand post-processing will be done. 
    // The reason why this part is necessary is because it will
    // output a more accurate picture of the VM brand. For example, 
    // Azure's cloud is based on Hyper-V, but Hyper-V may have 
    // a higher score due to the prevalence of it in a practical 
    // setting, which will put Azure to the side. This is stupid 
    // because there should be an indication that Azure is involved
    // since it's a better idea to let the end-user know that the
    // brand is "Azure Hyper-V" instead of just "Hyper-V". So what
    // this section does is "merge" the brands together to form
    // a more accurate idea of the brand(s) involved.
    merge(TMP_AZURE, TMP_HYPERV,     TMP_AZURE);
    merge(TMP_AZURE, TMP_VPC,        TMP_AZURE);
    merge(TMP_AZURE, TMP_HYPERV_VPC, TMP_AZURE);

    merge(TMP_NANOVISOR, TMP_HYPERV,     TMP_NANOVISOR);
    merge(TMP_NANOVISOR, TMP_VPC,        TMP_NANOVISOR);
    merge(TMP_NANOVISOR, TMP_HYPERV_VPC, TMP_NANOVISOR);
    
    merge(TMP_QEMU,     TMP_KVM,        TMP_QEMU_KVM);
    merge(TMP_KVM,      TMP_HYPERV,     TMP_KVM_HYPERV);
    merge(TMP_QEMU,     TMP_HYPERV,     TMP_QEMU_KVM_HYPERV);
    merge(TMP_QEMU_KVM, TMP_HYPERV,     TMP_QEMU_KVM_HYPERV);
    merge(TMP_KVM,      TMP_KVM_HYPERV, TMP_KVM_HYPERV);
    merge(TMP_QEMU,     TMP_KVM_HYPERV, TMP_QEMU_KVM_HYPERV);
    merge(TMP_QEMU_KVM, TMP_KVM_HYPERV, TMP_QEMU_KVM_HYPERV);

    triple_merge(TMP_QEMU, TMP_KVM, TMP_KVM_HYPERV, TMP_QEMU_KVM_HYPERV);

    merge(TMP_VMWARE, TMP_FUSION,      TMP_FUSION);
    merge(TMP_VMWARE, TMP_EXPRESS,     TMP_EXPRESS);
    merge(TMP_VMWARE, TMP_ESX,         TMP_ESX);
    merge(TMP_VMWARE, TMP_GSX,         TMP_GSX);
    merge(TMP_VMWARE, TMP_WORKSTATION, TMP_WORKSTATION);

    merge(TMP_VMWARE_HARD, TMP_VMWARE,      TMP_VMWARE_HARD);
    merge(TMP_VMWARE_HARD, TMP_FUSION,      TMP_VMWARE_HARD);
    merge(TMP_VMWARE_HARD, TMP_EXPRESS,     TMP_VMWARE_HARD);
    merge(TMP_VMWARE_HARD, TMP_ESX,         TMP_VMWARE_HARD);
    merge(TMP_VMWARE_HARD, TMP_GSX,         TMP_VMWARE_HARD);
    merge(TMP_VMWARE_HARD, TMP_WORKSTATION, TMP_VMWARE_HARD);


    // this is added in case the lib detects a non-Hyper-X technique.
    // A Hyper-X affiliated technique should make the overall score
    // as 0, but this isn't the case if non-Hyper-X techniques were
    // found. There may be a conflict between an Unknown and Hyper-V
    // brand, which is exactly what this section is meant to handle.
    // It will remove the Hyper-V artifact brand string from the 
    // std::map to pave the way for other brands to take precedence.
    // One of the main reasons to do this is because it would look
    // incredibly awkward if the brand was "Hyper-V artifacts (not an
    // actual VM)", clearly stating that it's NOT a VM while the VM
    // confirmation is true and percentage is 100%, as if that makes
    // any sense whatsoever. That's what this part fixes.
    if (brands.count(TMP_HYPERV_ARTIFACT) > 0) {
        if (score > 0) {
            brands.erase(TMP_HYPERV_ARTIFACT);
        }
    }


    // the brand element, which stores the NAME (const char*) and the SCORE (u8)
    using brand_element_t = std::pair<const char*, brand_score_t>;

    // convert the std::map into a std::vector, easier to handle this way
    std::vector<brand_element_t> vec(brands.begin(), brands.end());

    // sort the relevant brands vector so that the brands with 
    // the highest score appears first in descending order
    std::sort(vec.begin(), vec.end(), [](
        const brand_element_t &a,
        const brand_element_t &b
    ) {
        return a.second > b.second;
    });

    std::string ret_str = brands::NULL_BRAND;




    // if the multiple setting flag is NOT set, return the
    // brand with the highest score. Else, return a std::string
    // of the brand message (i.e. "VirtualBox or VMware").
    // See VM::MULTIPLE flag in docs for more information.
    if (!is_multiple) {
        ret_str = vec.front().first;
    } else {
        std::stringstream ss;
        std::size_t i = 1;

        ss << vec.front().first;
        for (; i < vec.size(); i++) {
            ss << " or ";
            ss << vec.at(i).first;
        }
        ret_str = ss.str();
    }



    // cache the result if memoization is enabled
    if (core::is_disabled(flags, NO_MEMO)) {
        if (is_multiple) {
            core_debug("VM::brand(): cached multiple brand string");
            memo::multi_brand::store(ret_str);
        } else {
            core_debug("VM::brand(): cached brand string");
            memo::brand::store(ret_str);
        }
    }
    

    // debug stuff to see the brand scoreboard, ignore this
//#ifdef __VMAWARE_DEBUG__
    for (const auto& p : brands) {
        //core_debug("scoreboard: ", (int)p.second, " : ", p.first);
        std::cout << "scoreboard: " << (int)p.second << " : " << p.first;
    }
//#endif

    return ret_str;
}


/**
    * @brief Detect if running inside a VM
    * @param any flag combination in VM structure or nothing
    * @return bool
    * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmdetect
    */
template <typename ...Args>
static bool detect(Args ...args) {
    // fetch all the flags in a std::bitset
    flagset flags = arg::arg_handler(args...);

    // run all the techniques based on the 
    // flags above, and get a total score 
    const u16 points = core::run_all(flags, SHORTCUT);

#if (CPP >= 23)
    [[assume(points < maximum_points)]];
#endif

    u16 threshold = 150;

    // if high threshold is set, the points 
    // will be 300. If not, leave it as 150.
    if (core::is_enabled(flags, HIGH_THRESHOLD)) {
        threshold = high_threshold_score;
    }

    return (points >= threshold);
}


/**
    * @brief Get the percentage of how likely it's a VM
    * @param any flag combination in VM structure or nothing
    * @return std::uint8_t
    * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmpercentage
    */
template <typename ...Args>
static u8 percentage(Args ...args) {
    // fetch all the flags in a std::bitset
    const flagset flags = arg::arg_handler(args...);

    // run all the techniques based on the 
    // flags above, and get a total score
    const u16 points = core::run_all(flags, SHORTCUT);

#if (CPP >= 23)
    [[assume(points < maximum_points)]];
#endif

    u8 percent = 0;
    u16 threshold = 150;

    // set to 300 if high threshold is enabled
    if (core::is_enabled(flags, HIGH_THRESHOLD)) {
        threshold = high_threshold_score;
    }

    // the percentage will be set to 99%, because a score 
    // of 100 is not entirely robust. 150 is more robust
    // in my opinion, which is why you need a score of
    // above 150 to get to 100% 
    if (points >= threshold) {
        percent = 100;
    } else if (points >= 100) {
        percent = 99;
    } else {
        percent = static_cast<u8>(points);
    }

    return percent;
}


/**
    * @brief Add a custom technique to the VM detection technique collection
    * @param either a function pointer, lambda function, or std::function<bool()>
    * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmaddcustom
    * @return void
    */
static void add_custom(
    const std::uint8_t percent,
    std::function<bool()> detection_func
    // clang doesn't support std::source_location for some reason
#if (CPP >= 20 && !CLANG)
    , const std::source_location& loc = std::source_location::current()
#endif
) {
    // lambda to throw the error
    auto throw_error = [&](const char* text) -> void {
        std::stringstream ss;
#if (CPP >= 20 && !CLANG)
        ss << ", error in " << loc.function_name() << " at " << loc.file_name() << ":" << loc.line() << ")";
#endif
        ss << ". Consult the documentation's parameters for VM::add_custom()";
        throw std::invalid_argument(std::string(text) + ss.str());
    };

    if (percent > 100) {
        throw_error("Percentage parameter must be between 0 and 100");
    }

#if (CPP >= 23)
    [[assume(percent > 0 && percent <= 100)]];
#endif

    static u16 id = 0;
    id++;

    // generate the custom technique struct
    core::custom_technique query{
        percent,
        // this fucking sucks
        static_cast<u16>(static_cast<int>(base_technique_count) + static_cast<int>(id)),
        detection_func
    };

    globals::technique_count++;

    // push it to the custome_table vector
    core::custom_table.emplace_back(query);
}


/**
    * @brief disable the provided technique flags so they are not counted to the overall result
    * @param technique flag(s) only
    * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmdetect
    * @return flagset
    */
template <typename ...Args>
static flagset DISABLE(Args ...args) {
    // basically core::arg_handler but in reverse,
    // it'll clear the bits of the provided flags
    arg::disabled_arg_handler(args...);

    return arg::disabled_flag_collector;
}


/**
    * @brief This will convert the technique flag into a string, which will correspond to the technique name
    * @param single technique flag in VM structure
    */
[[nodiscard]] static std::string flag_to_string(const enum_flags flag) {
    switch (flag) {
        case VMID: return "VMID";
        case CPU_BRAND: return "CPU_BRAND";
        case HYPERVISOR_BIT: return "HYPERVISOR_BIT";
        case HYPERVISOR_STR: return "HYPERVISOR_STR";
        case TIMER: return "TIMER";
        case THREADCOUNT: return "THREADCOUNT";
        case MAC: return "MAC";
        case TEMPERATURE: return "TEMPERATURE";
        case SYSTEMD: return "SYSTEMD";
        case CVENDOR: return "CVENDOR";
        case CTYPE: return "CTYPE";
        case DOCKERENV: return "DOCKERENV";
        case DMIDECODE: return "DMIDECODE";
        case DMESG: return "DMESG";
        case HWMON: return "HWMON";
        case DLL: return "DLL";
        case REGISTRY: return "REGISTRY";
        case VM_FILES: return "VM_FILES";
        case HWMODEL: return "HWMODEL";
        case DISK_SIZE: return "DISK_SIZE";
        case VBOX_DEFAULT: return "VBOX_DEFAULT";
        case VBOX_NETWORK: return "VBOX_NETWORK";
/* GPL */   case COMPUTER_NAME: return "COMPUTER_NAME";
/* GPL */   case WINE_CHECK: return "WINE_CHECK";
/* GPL */   case HOSTNAME: return "HOSTNAME";
/* GPL */   case KVM_DIRS: return "KVM_DIRS";
/* GPL */   case QEMU_DIR: return "QEMU_DIR";
/* GPL */   case POWER_CAPABILITIES: return "POWER_CAPABILITIES";
/* GPL */   case SETUPAPI_DISK: return "SETUPAPI_DISK";
        case VM_PROCESSES: return "VM_PROCESSES";
        case LINUX_USER_HOST: return "LINUX_USER_HOST";
        case GAMARUE: return "GAMARUE";
        case BOCHS_CPU: return "BOCHS_CPU";
        case MSSMBIOS: return "MSSMBIOS";
        case MAC_MEMSIZE: return "MAC_MEMSIZE";
        case MAC_IOKIT: return "MAC_IOKIT";
        case IOREG_GREP: return "IOREG_GREP";
        case MAC_SIP: return "MAC_SIP";
        case HKLM_REGISTRIES: return "HKLM_REGISTRIES";
        case VPC_INVALID: return "VPC_INVALID";
        case SIDT: return "SIDT";
        case SGDT: return "SGDT";
        case SLDT: return "SLDT";
        case VMWARE_IOMEM: return "VMWARE_IOMEM";
        case VMWARE_IOPORTS: return "VMWARE_IOPORTS";
        case VMWARE_SCSI: return "VMWARE_SCSI";
        case VMWARE_DMESG: return "VMWARE_DMESG";
        case VMWARE_STR: return "VMWARE_STR";
        case VMWARE_BACKDOOR: return "VMWARE_BACKDOOR";
        case VMWARE_PORT_MEM: return "VMWARE_PORT_MEM";
        case SMSW: return "SMSW";
        case MUTEX: return "MUTEX";
        case ODD_CPU_THREADS: return "ODD_CPU_THREADS";
        case INTEL_THREAD_MISMATCH: return "INTEL_THREAD_MISMATCH";
        case XEON_THREAD_MISMATCH: return "XEON_THREAD_MISMATCH";
        case CUCKOO_DIR: return "CUCKOO_DIR";
        case CUCKOO_PIPE: return "CUCKOO_PIPE";
        case HYPERV_HOSTNAME: return "HYPERV_HOSTNAME";
        case GENERAL_HOSTNAME: return "GENERAL_HOSTNAME";
        case SCREEN_RESOLUTION: return "SCREEN_RESOLUTION";
        case DEVICE_STRING: return "DEVICE_STRING";
        case BLUESTACKS_FOLDERS: return "BLUESTACKS_FOLDERS";
        case CPUID_SIGNATURE: return "CPUID_SIGNATURE";
        case KVM_BITMASK: return "KVM_BITMASK";
        case KGT_SIGNATURE: return "KGT_SIGNATURE";
        case QEMU_VIRTUAL_DMI: return "QEMU_VIRTUAL_DMI";
        case QEMU_USB: return "QEMU_USB";
        case HYPERVISOR_DIR: return "HYPERVISOR_DIR";
        case UML_CPU: return "UML_CPU";
        case KMSG: return "KMSG";
        case VM_PROCS: return "VM_PROCS";
        case VBOX_MODULE: return "VBOX_MODULE";
        case SYSINFO_PROC: return "SYSINFO_PROC";
        case DEVICE_TREE: return "DEVICE_TREE";
        case DMI_SCAN: return "DMI_SCAN";
        case SMBIOS_VM_BIT: return "SMBIOS_VM_BIT";
        case PODMAN_FILE: return "PODMAN_FILE";
        case WSL_PROC: return "WSL_PROC";
        case DRIVER_NAMES: return "DRIVER_NAMES";
        case DISK_SERIAL: return "DISK_SERIAL";
        case PORT_CONNECTORS: return "PORT_CONNECTORS";
        case IVSHMEM: return "IVSHMEM";
        case GPU_VM_STRINGS: return "GPU_STRINGS";
        case GPU_CAPABILITIES: return "GPU_CAPABILITIES";
        case VM_DEVICES: return "VM_DEVICES";
        case PROCESSOR_NUMBER: return "PROCESSOR_NUMBER";
        case NUMBER_OF_CORES: return "NUMBER_OF_CORES";
        case ACPI_TEMPERATURE: return "ACPI_TEMPERATURE";
        case QEMU_FW_CFG: return "QEMU_FW_CFG";
        case LSHW_QEMU: return "LSHW_QEMU";
        case VIRTUAL_PROCESSORS: return "VIRTUAL_PROCESSORS";
        case HYPERV_QUERY: return "HYPERV_QUERY";
        case BAD_POOLS: return "BAD_POOLS";
        case AMD_SEV: return "AMD_SEV";
        case AMD_THREAD_MISMATCH: return "AMD_THREAD_MISMATCH";
        case NATIVE_VHD: return "NATIVE_VHD";
        case VIRTUAL_REGISTRY: return "VIRTUAL_REGISTRY";
        case FIRMWARE: return "FIRMWARE";
        case FILE_ACCESS_HISTORY: return "FILE_ACCESS_HISTORY";
        case AUDIO: return "AUDIO";
        case UNKNOWN_MANUFACTURER: return "UNKNOWN_MANUFACTURER";
        case NSJAIL_PID: return "NSJAIL_PID";
        case TPM: return "TPM";
        case PCI_VM_DEVICE_ID: return "PCI_VM_DEVICE_ID";
        case QEMU_PASSTHROUGH: return "QEMU_PASSTHROUGH";
        // ADD NEW CASE HERE FOR NEW TECHNIQUE
        default: return "Unknown flag";
    }
}


/**
    * @brief Fetch all the brands that were detected in a vector
    * @param any flag combination in VM structure or nothing
    * @return std::vector<VM::enum_flags>
    */
template <typename ...Args>
static std::vector<enum_flags> detected_enums(Args ...args) {
    const flagset flags = arg::arg_handler(args...);

    std::vector<enum_flags> tmp{};

    // this will loop through all the enums in the technique_vector variable,
    // and then checks each of them and outputs the enum that was detected
    for (const auto technique_enum : globals::technique_vector) {
        if (
            (flags.test(technique_enum)) &&
            (check(static_cast<enum_flags>(technique_enum)))
        ) {
            tmp.push_back(static_cast<enum_flags>(technique_enum));
        }
    }

    return tmp;
}


/**
    * @brief Change the certainty score of a technique
    * @param technique flag, then the new percentage score to overwite
    * @return void
    * @warning ⚠️ FOR DEVELOPMENT USAGE ONLY, NOT MEANT FOR PUBLIC USE FOR NOW ⚠️
    */
static void modify_score(
    const enum_flags flag,
    const u8 percent
    // clang doesn't support std::source_location for some reason
#if (CPP >= 20 && !CLANG)
    , const std::source_location& loc = std::source_location::current()
#endif
) {
    // lambda to throw the error
    auto throw_error = [&](const char* text) -> void {
        std::stringstream ss;
#if (CPP >= 20 && !CLANG)
        ss << ", error in " << loc.function_name() << " at " << loc.file_name() << ":" << loc.line() << ")";
#endif
        ss << ". Consult the documentation's parameters for VM::modify_score()";
        throw std::invalid_argument(std::string(text) + ss.str());
    };

    if (percent > 100) {
        throw_error("Percentage parameter must be between 0 and 100");
    }

#if (CPP >= 23)
    [[assume(percent <= 100)]];
#endif

    // check if the flag provided is a setting flag, which isn't valid.
    if (static_cast<u8>(flag) >= technique_end) {
        throw_error("The flag is not a technique flag");
    }

    using table_t =  std::map<enum_flags, core::technique>;

    auto modify = [](table_t &table, const enum_flags flag, const u8 percent) -> void {
        core::technique &tmp = table.at(flag);
        table[flag].points = percent;
        table[flag].run = tmp.run;
    };

    modify(core::technique_table, flag, percent);
}


/**
    * @brief Fetch the total number of detected techniques
    * @param any flag combination in VM structure or nothing
    * @return std::uint8_t
    */
template <typename ...Args>
static u8 detected_count(Args ...args) {
    flagset flags = arg::arg_handler(args...);

    // run all the techniques, which will set the detected_count variable 
    core::run_all(flags);

    return globals::detected_count_num;
}


/**
    * @brief Fetch the total number of detected techniques
    * @param any flag combination in VM structure or nothing
    * @return std::uint8_t
    */
template <typename ...Args>
static std::string type(Args ...args) {
    flagset flags = arg::arg_handler(args...);

    const std::string brand_str = brand(flags);

    // if multiple brands were found, return unknown
    if (util::find(brand_str, " or ")) {
        return "Unknown";
    }

    const std::map<std::string, const char*> type_table {
        // type 1
        { brands::XEN, "Hypervisor (type 1)" },
        { brands::VMWARE_ESX, "Hypervisor (type 1)" },
        { brands::ACRN, "Hypervisor (type 1)" },
        { brands::QNX, "Hypervisor (type 1)" },
        { brands::HYPERV, "Hypervisor (type 2)" }, // to clarify you're running under a Hyper-V guest VM
        { brands::AZURE_HYPERV, "Hypervisor (type 1)" },
        { brands::NANOVISOR, "Hypervisor (type 1)" },
        { brands::KVM, "Hypervisor (type 1)" },
        { brands::KVM_HYPERV, "Hypervisor (type 1)" },
        { brands::QEMU_KVM_HYPERV, "Hypervisor (type 1)" },
        { brands::QEMU_KVM, "Hypervisor (type 1)" },
        { brands::INTEL_HAXM, "Hypervisor (type 1)" },
        { brands::INTEL_KGT, "Hypervisor (type 1)" },
        { brands::SIMPLEVISOR, "Hypervisor (type 1)" },
        { brands::OPENSTACK, "Hypervisor (type 1)" },
        { brands::KUBEVIRT, "Hypervisor (type 1)" },
        { brands::POWERVM, "Hypervisor (type 1)" },
        { brands::AWS_NITRO, "Hypervisor (type 1)" },
        { brands::LKVM, "Hypervisor (type 1)" },
        { brands::NOIRVISOR, "Hypervisor (type 1)" },
        { brands::WSL, "Hypervisor (Type 1)" }, // Type 1-derived lightweight VM system

        // type 2
        { brands::BHYVE, "Hypervisor (type 2)" },
        { brands::VBOX, "Hypervisor (type 2)" },
        { brands::VMWARE, "Hypervisor (type 2)" },
        { brands::VMWARE_EXPRESS, "Hypervisor (type 2)" },
        { brands::VMWARE_GSX, "Hypervisor (type 2)" },
        { brands::VMWARE_WORKSTATION, "Hypervisor (type 2)" },
        { brands::VMWARE_FUSION, "Hypervisor (type 2)" },
        { brands::PARALLELS, "Hypervisor (type 2)" },
        { brands::VPC, "Hypervisor (type 2)" },
        { brands::NVMM, "Hypervisor (type 2)" },
        { brands::BSD_VMM, "Hypervisor (type 2)" },
        { brands::HYPERV_VPC, "Hypervisor (type 2)" },

        // sandbox
        { brands::CUCKOO, "Sandbox" },
        { brands::SANDBOXIE, "Sandbox" },
        { brands::HYBRID, "Sandbox" },
        { brands::CWSANDBOX, "Sandbox" },
        { brands::JOEBOX, "Sandbox" },
        { brands::ANUBIS, "Sandbox" },
        { brands::COMODO, "Sandbox" },
        { brands::THREATEXPERT, "Sandbox" },
        { brands::QIHOO, "Sandbox" },
        { brands::HYPERVISOR_PHANTOM, "Sandbox" },

        // misc
        { brands::BOCHS, "Emulator" },
        { brands::BLUESTACKS, "Emulator" },
        { brands::NEKO_PROJECT, "Emulator" },
        { brands::QEMU, "Emulator/Hypervisor (type 2)" },
        { brands::JAILHOUSE, "Partitioning Hypervisor" },
        { brands::UNISYS, "Partitioning Hypervisor" },
        { brands::DOCKER, "Container" },
        { brands::PODMAN, "Container" },
        { brands::OPENVZ, "Container" },
        { brands::LMHS, "Hypervisor (unknown type)" },
        { brands::WINE, "Compatibility layer" },
        { brands::INTEL_TDX, "Trusted Domain" },
        { brands::APPLE_VZ, "Unknown" },
        { brands::UML, "Paravirtualised/Hypervisor (type 2)" },
        { brands::AMD_SEV, "VM encryptor" },
        { brands::AMD_SEV_ES, "VM encryptor" },
        { brands::AMD_SEV_SNP, "VM encryptor" },
        { brands::GCE, "Cloud VM service" },
        { brands::NSJAIL, "Process isolator" },
        { brands::HYPERV_ARTIFACT, "Unknown" }, // This refers to the type 1 hypervisor where Windows normally runs under, we put "Unknown" to clarify you're not running under a VM if this is detected
        { brands::NULL_BRAND, "Unknown" }
    };

    auto it = type_table.find(brand_str.c_str());

    if (it != type_table.end()) {
        return it->second;
    }

    debug("VM::type(): No known brand found, something went terribly wrong here...");

    return "Unknown";
}


/**
    * @brief Fetch the conclusion message based on the brand and percentage
    * @param any flag combination in VM structure or nothing
    * @return std::string
    */
template <typename ...Args>
static std::string conclusion(Args ...args) {
    flagset flags = arg::arg_handler(args...);

    std::string brand_tmp = brand(flags);
    const u8 percent_tmp = percentage(flags);

    constexpr const char* baremetal = "Running on baremetal";
    constexpr const char* very_unlikely = "Very unlikely a VM";
    constexpr const char* unlikely = "Unlikely a VM";

#if (CPP >= 17)
    constexpr std::string_view potentially = "Potentially";
    constexpr std::string_view might = "Might be";
    constexpr std::string_view likely = "Likely";
    constexpr std::string_view very_likely = "Very likely";
    constexpr std::string_view inside_vm = "Running inside";
#else
    const std::string potentially = "Potentially";
    const std::string might = "Might be";
    const std::string likely = "Likely";
    const std::string very_likely = "Very likely";
    const std::string inside_vm = "Running inside";
#endif

#if (CPP >= 17)
    auto make_conclusion = [&](const std::string_view category) -> std::string {
#else
    auto make_conclusion = [&](const std::string &category) -> std::string {
#endif
        // this basically just fixes the grammatical syntax
        // by either having "a" or "an" before the VM brand
        // name. Like it would look weird if the conclusion 
        // message was "an VirtualBox" or "a Anubis", so this
        // lambda fixes that issue.
        std::string article = "";

        if (
            (brand_tmp == brands::ACRN) ||
            (brand_tmp == brands::ANUBIS) ||
            (brand_tmp == brands::BSD_VMM) ||
            (brand_tmp == brands::INTEL_HAXM) ||
            (brand_tmp == brands::APPLE_VZ) ||
            (brand_tmp == brands::INTEL_KGT) ||
            (brand_tmp == brands::POWERVM) ||
            (brand_tmp == brands::OPENSTACK) ||
            (brand_tmp == brands::AWS_NITRO) ||
            (brand_tmp == brands::OPENVZ) ||
            (brand_tmp == brands::INTEL_TDX) ||
            (brand_tmp == brands::AMD_SEV) ||
            (brand_tmp == brands::AMD_SEV_ES) ||
            (brand_tmp == brands::AMD_SEV_SNP) ||
            (brand_tmp == brands::NSJAIL) ||
            (brand_tmp == brands::NULL_BRAND)
        ) {
            article = " an ";
        } else {
            article = " a ";
        }

        // this is basically just to remove the capital "U", 
        // since it doesn't make sense to see "an Unknown"
        if (brand_tmp == brands::NULL_BRAND) {
            brand_tmp = "unknown";
        }

        // Hyper-V artifacts are an exception due to how unique the circumstance is
        if (brand_tmp == brands::HYPERV_ARTIFACT) {
            return std::string(category) + article + brand_tmp;
        }
        else {
            return std::string(category) + article + brand_tmp + " VM";
        }
    };

    if (core::is_enabled(flags, DYNAMIC)) {
        if      (percent_tmp == 0)  { return baremetal; }
        else if (percent_tmp <= 20) { return very_unlikely; }
        else if (percent_tmp <= 35) { return unlikely; }
        else if (percent_tmp < 50)  { return make_conclusion(potentially); }
        else if (percent_tmp <= 62) { return make_conclusion(might); }
        else if (percent_tmp <= 75) { return make_conclusion(likely); }
        else if (percent_tmp < 100) { return make_conclusion(very_likely); }
        else                        { return make_conclusion(inside_vm); }
    }

    if (percent_tmp == 100) {
        return make_conclusion(inside_vm);
    } else {
        return baremetal;
    }
}



#pragma pack(push, 1)
struct vmaware {
    std::string brand;
    std::string type;
    std::string conclusion;
    bool is_vm;
    u8 percentage;
    u8 detected_count;
    u16 technique_count;

    template <typename ...Args>
    vmaware(Args ...args) {
        flagset flags = arg::arg_handler(args...);

        brand = brand(flags);
        type = type(flags);
        conclusion = VM::conclusion(flags);
        is_vm = VM::detect(flags);
        percentage = VM::percentage(flags);
        detected_count = VM::detected_count(flags);
        technique_count = VM::technique_count;
    }
};
#pragma pack(pop)