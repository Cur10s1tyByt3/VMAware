#include "../enums.hpp"
#include "../api.hpp"
#include "../arg_handle.hpp"
#include "../globals.hpp"
#include "memo.hpp"
#include "core.hpp"
#include <map>
#include <vector>
#include <cstring>


// directly return when adding a brand to the scoreboard for a more succint expression
inline bool core::add(const char* p_brand, const char* extra_brand) noexcept {
    core::brand_scoreboard.at(p_brand)++;
    if (std::strcmp(extra_brand, "") != 0) {
        core::brand_scoreboard.at(p_brand)++;
    }
    return true;
}

// assert if the flag is enabled, far better expression than typing std::bitset member functions
[[nodiscard]] inline bool core::is_disabled(const flagset& flags, const u8 flag_bit) noexcept {
    return (!flags.test(flag_bit));
}

// same as above but for checking enabled flags
[[nodiscard]] inline bool core::is_enabled(const flagset& flags, const u8 flag_bit) noexcept {
    return (flags.test(flag_bit));
}

[[nodiscard]] bool core::is_technique_set(const flagset& flags) {
    for (std::size_t i = technique_begin; i < technique_end; i++) {
        if (flags.test(i)) {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool core::is_setting_flag_set(const flagset& flags) {
    for (std::size_t i = settings_begin; i < settings_end; i++) {
        if (flags.test(i)) {
            return true;
        }
    }

    return false;
}

// manage the flag to handle edgecases
void core::flag_sanitizer(flagset& flags) {
    if (flags.count() == 0) {
        arg::generate_default(flags);
        return;
    }

    if (flags.test(DEFAULT)) {
        return;
    }
    
    if (flags.test(ALL)) {
        return;
    }

    // check if any technique flag is set, which is the "correct" way
    if (core::is_technique_set(flags)) {
        return;
    }

    if (!core::is_setting_flag_set(flags)) {
        throw std::invalid_argument("Invalid flag option for function parameter found, either leave it empty or add the VM::DEFAULT flag");
    }

    // at this stage, only setting flags are asserted to be set
    if (
        flags.test(NO_MEMO) ||
        flags.test(HIGH_THRESHOLD) ||
        flags.test(DYNAMIC) ||
        flags.test(NULL_ARG) ||
        flags.test(MULTIPLE)
    ) {
        arg::generate_default(flags);
    } else {
        throw std::invalid_argument("Invalid flag option found, aborting");
    }
}

// run every VM detection mechanism in the technique table
u16 core::run_all(const flagset& flags, const bool shortcut) {
    u16 points = 0;

    const bool memo_enabled = core::is_disabled(flags, NO_MEMO);

    u16 threshold_points = 150;
    
    // set it to 300 if high threshold is enabled
    if (core::is_enabled(flags, HIGH_THRESHOLD)) {
        threshold_points = high_threshold_score;
    }

    // loop through the technique table, where all the techniques are stored
    for (const auto& tmp : technique_table) {
        const enum_flags technique_macro = tmp.first;
        const technique technique_data = tmp.second;

        // check if the technique is disabled
        if (core::is_disabled(flags, technique_macro)) {
            continue;
        }

        // both of these depend interchangeably, so both scores
        // are "merged" by making it 100 instead of 200 combined.
        // the GPU ones are that exception, and they will be run
        // in the post-processing stage within run_all();
        if (
            (technique_macro == GPU_CAPABILITIES) ||
            (technique_macro == GPU_VM_STRINGS)
        ) {
            continue;
        }

        // check if the technique is cached already
        if (memo_enabled && memo::is_cached(technique_macro)) {
            const memo::data_t data = memo::cache_fetch(technique_macro);

            if (data.result) {
                points += data.points;
            }

            continue;
        }

        // run the technique
        const bool result = technique_data.run();

        // accumulate the points if the technique detected a VM
        if (result) {
            points += technique_data.points;

            // this is specific to VM::detected_count() which 
            // returns the number of techniques that found a VM.
            globals::detected_count_num++;
        }

        // store the current technique result to the cache
        if (memo_enabled) {
            memo::cache_store(technique_macro, result, technique_data.points);
        }

        // for things like VM::detect() and VM::percentage(),
        // a score of 150+ is guaranteed to be a VM, so
        // there's no point in running the rest of the techniques
        // (unless the threshold is set to be higher, but it's the 
        // same story here nonetheless, except the threshold is 300)
        if (
            (shortcut) && 
            (points >= threshold_points)
        ) {
            return points;
        }
    }

    // for custom VM techniques, won't be used most of the time
    if (!custom_table.empty()) {
        for (const auto& technique : custom_table) {
            // if cached, return that result
            if (memo_enabled && memo::is_cached(technique.id)) {
                const memo::data_t data = memo::cache_fetch(technique.id);

                if (data.result) {
                    points += data.points;
                }

                continue;
            }

            // run the custom technique
            const bool result = technique.run();

            // accumulate a few important values
            if (result) {
                points += technique.points;
                globals::detected_count_num++;
            }

            // cache the result
            if (memo_enabled) {
                memo::cache_store(
                    technique.id,
                    result, 
                    technique.points
                );
            }
        }
    }


    // points post-processing stage
    const std::vector<enum_flags> post_processed_techniques = {
        GPU_CAPABILITIES,
        GPU_VM_STRINGS
    };

    auto merge_scores = [&](
        const enum_flags a, 
        const enum_flags b, 
        const u8 new_score
    ) {
        if (
            core::is_disabled(flags, a) ||
            core::is_disabled(flags, b)
        ) {
            return;
        }

        const bool result_a = check(a);
        const bool result_b = check(b);

        if (result_a && result_b) {
            points += new_score;
            return;
        } else if ((result_a == false) && (result_b == false)) {
            return;
        } else {
            enum_flags tmp_flag;

            if (result_a == true) {
                tmp_flag = a;
            } else {
                tmp_flag = b;
            }

            const technique tmp = technique_table.at(tmp_flag);
            points += tmp.points;
        }
    };

    merge_scores(GPU_CAPABILITIES, GPU_VM_STRINGS, 100); // instead of 200, it's 100 now

    return points;
}


































    // this is initialised as empty, because this is where custom techniques can be added at runtime 
std::vector<core::custom_technique> core::custom_table = {

};

#define table_t std::map<enum_flags, core::technique>

// the 0~100 points are debatable, but I think it's fine how it is. Feel free to disagree.
std::pair<enum_flags, core::technique> core::technique_list[] = {
    // FORMAT: { VM::<ID>, { certainty%, function pointer } },
    // START OF TECHNIQUE TABLE
    std::make_pair(VMID, core::technique(100, vmid)),
    std::make_pair(CPU_BRAND, core::technique(50, cpu_brand)),
    std::make_pair(HYPERVISOR_BIT, core::technique(100, hypervisor_bit)),
    std::make_pair(HYPERVISOR_STR, core::technique(75, hypervisor_str)),
    std::make_pair(TIMER, core::technique(45, timer)),
    std::make_pair(THREADCOUNT, core::technique(35, thread_count)),
    std::make_pair(MAC, core::technique(20, mac_address_check)),
    std::make_pair(TEMPERATURE, core::technique(15, temperature)),
    std::make_pair(SYSTEMD, core::technique(35, systemd_virt)),
    std::make_pair(CVENDOR, core::technique(65, chassis_vendor)),
    std::make_pair(CTYPE, core::technique(20, chassis_type)),
    std::make_pair(DOCKERENV, core::technique(30, dockerenv)),
    std::make_pair(DMIDECODE, core::technique(55, dmidecode)),
    std::make_pair(DMESG, core::technique(55, dmesg)),
    std::make_pair(HWMON, core::technique(35, hwmon)),
    std::make_pair(DLL, core::technique(25, dll_check)),
    std::make_pair(REGISTRY, core::technique(50, registry_key)),
    std::make_pair(VM_FILES, core::technique(25, vm_files)),
    std::make_pair(HWMODEL, core::technique(100, hwmodel)),
    std::make_pair(DISK_SIZE, core::technique(60, disk_size)),
    std::make_pair(VBOX_DEFAULT, core::technique(25, vbox_default_specs)),
    std::make_pair(VBOX_NETWORK, core::technique(100, vbox_network_share)),
/* GPL */ std::make_pair(COMPUTER_NAME, core::technique(10, computer_name_match)),
/* GPL */ std::make_pair(WINE_CHECK, core::technique(100, wine)),
/* GPL */ std::make_pair(HOSTNAME, core::technique(10, hostname_match)),
/* GPL */ std::make_pair(KVM_DIRS, core::technique(30, kvm_directories)),
/* GPL */ std::make_pair(QEMU_DIR, core::technique(30, qemu_dir)),
/* GPL */ std::make_pair(POWER_CAPABILITIES, core::technique(50, power_capabilities)),
/* GPL */ std::make_pair(SETUPAPI_DISK, core::technique(100, setupapi_disk)),
    std::make_pair(VM_PROCESSES, core::technique(15, vm_processes)),
    std::make_pair(LINUX_USER_HOST, core::technique(10, linux_user_host)),
    std::make_pair(GAMARUE, core::technique(10, gamarue)),
    std::make_pair(BOCHS_CPU, core::technique(100, bochs_cpu)),
    std::make_pair(MSSMBIOS, core::technique(100, mssmbios)),
    std::make_pair(MAC_MEMSIZE, core::technique(15, hw_memsize)),
    std::make_pair(MAC_IOKIT, core::technique(100, io_kit)),
    std::make_pair(IOREG_GREP, core::technique(100, ioreg_grep)),
    std::make_pair(MAC_SIP, core::technique(40, mac_sip)),
    std::make_pair(HKLM_REGISTRIES, core::technique(25, hklm_registries)),
    std::make_pair(VPC_INVALID, core::technique(75, vpc_invalid)),
    std::make_pair(SIDT, core::technique(25, sidt)),
    std::make_pair(SGDT, core::technique(30, sgdt)),
    std::make_pair(SLDT, core::technique(15, sldt)),
    std::make_pair(VMWARE_IOMEM, core::technique(65, vmware_iomem)),
    std::make_pair(VMWARE_IOPORTS, core::technique(70, vmware_ioports)),
    std::make_pair(VMWARE_SCSI, core::technique(40, vmware_scsi)),
    std::make_pair(VMWARE_DMESG, core::technique(65, vmware_dmesg)),
    std::make_pair(VMWARE_STR, core::technique(35, vmware_str)),
    std::make_pair(VMWARE_BACKDOOR, core::technique(100, vmware_backdoor)),
    std::make_pair(VMWARE_PORT_MEM, core::technique(85, vmware_port_memory)),
    std::make_pair(SMSW, core::technique(30, smsw)),
    std::make_pair(MUTEX, core::technique(85, mutex)),
    std::make_pair(ODD_CPU_THREADS, core::technique(80, odd_cpu_threads)),
    std::make_pair(INTEL_THREAD_MISMATCH, core::technique(95, intel_thread_mismatch)),
    std::make_pair(XEON_THREAD_MISMATCH, core::technique(95, xeon_thread_mismatch)),
    std::make_pair(AMD_THREAD_MISMATCH, core::technique(95, amd_thread_mismatch)),
    std::make_pair(CUCKOO_DIR, core::technique(30, cuckoo_dir)),
    std::make_pair(CUCKOO_PIPE, core::technique(30, cuckoo_pipe)),
    std::make_pair(HYPERV_HOSTNAME, core::technique(30, hyperv_hostname)),
    std::make_pair(GENERAL_HOSTNAME, core::technique(10, general_hostname)),
    std::make_pair(SCREEN_RESOLUTION, core::technique(20, screen_resolution)),
    std::make_pair(DEVICE_STRING, core::technique(25, device_string)),
    std::make_pair(BLUESTACKS_FOLDERS, core::technique(5, bluestacks)),
    std::make_pair(CPUID_SIGNATURE, core::technique(95, cpuid_signature)),
    std::make_pair(KVM_BITMASK, core::technique(40, kvm_bitmask)),
    std::make_pair(KGT_SIGNATURE, core::technique(80, intel_kgt_signature)),
    std::make_pair(QEMU_VIRTUAL_DMI, core::technique(40, qemu_virtual_dmi)),
    std::make_pair(QEMU_USB, core::technique(20, qemu_USB)),
    std::make_pair(HYPERVISOR_DIR, core::technique(20, hypervisor_dir)),
    std::make_pair(UML_CPU, core::technique(80, uml_cpu)),
    std::make_pair(KMSG, core::technique(5, kmsg)),
    std::make_pair(VM_PROCS, core::technique(10, vm_procs)),
    std::make_pair(VBOX_MODULE, core::technique(15, vbox_module)),
    std::make_pair(SYSINFO_PROC, core::technique(15, sysinfo_proc)),
    std::make_pair(DEVICE_TREE, core::technique(20, device_tree)),
    std::make_pair(DMI_SCAN, core::technique(50, dmi_scan)),
    std::make_pair(SMBIOS_VM_BIT, core::technique(50, smbios_vm_bit)),
    std::make_pair(PODMAN_FILE, core::technique(5, podman_file)),
    std::make_pair(WSL_PROC, core::technique(30, wsl_proc_subdir)),
    std::make_pair(DRIVER_NAMES, core::technique(100, driver_names)),
    std::make_pair(DISK_SERIAL, core::technique(100, disk_serial_number)),
    std::make_pair(PORT_CONNECTORS, core::technique(25, port_connectors)),
    std::make_pair(IVSHMEM, core::technique(100, ivshmem)),
    std::make_pair(GPU_VM_STRINGS, core::technique(100, gpu_vm_strings)),
    std::make_pair(GPU_CAPABILITIES, core::technique(100, gpu_capabilities)),
    std::make_pair(VM_DEVICES, core::technique(50, vm_devices)),
    std::make_pair(PROCESSOR_NUMBER, core::technique(50, processor_number)),
    std::make_pair(NUMBER_OF_CORES, core::technique(50, number_of_cores)),
    std::make_pair(ACPI_TEMPERATURE, core::technique(25, acpi_temperature)),
    std::make_pair(QEMU_FW_CFG, core::technique(70, sys_qemu_dir)),
    std::make_pair(LSHW_QEMU, core::technique(80, lshw_qemu)),
    std::make_pair(VIRTUAL_PROCESSORS, core::technique(50, virtual_processors)),
    std::make_pair(HYPERV_QUERY, core::technique(100, hyperv_query)),
    std::make_pair(BAD_POOLS, core::technique(80, bad_pools)),
    std::make_pair(AMD_SEV, core::technique(50, amd_sev)),
    std::make_pair(NATIVE_VHD, core::technique(100, native_vhd)),
    std::make_pair(VIRTUAL_REGISTRY, core::technique(65, virtual_registry)),
    std::make_pair(FIRMWARE, core::technique(100, firmware_scan)),
    std::make_pair(FILE_ACCESS_HISTORY, core::technique(15, file_access_history)),
    std::make_pair(AUDIO, core::technique(25, check_audio)),
    std::make_pair(UNKNOWN_MANUFACTURER, core::technique(50, unknown_manufacturer)),
    std::make_pair(NSJAIL_PID, core::technique(75, nsjail_proc_id)),
    std::make_pair(TPM, core::technique(50, tpm)),
    std::make_pair(PCI_VM_DEVICE_ID, core::technique(90, pci_vm_device_id)),
    std::make_pair(QEMU_PASSTHROUGH, core::technique(90, qemu_passthrough)),

    // ADD NEW TECHNIQUE STRUCTURE HERE
};


// the reason why the map isn't directly initialized is due to potential 
// SDK errors on windows combined with older C++ standards
table_t VM::core::technique_table = []() -> table_t {
    table_t table;
    for (const auto& technique : VM::core::technique_list) {
        table.insert(technique);
    }
    return table;
}();







// scoreboard list of brands, if a VM detection technique detects a brand, that will be incremented here as a single point.
std::map<const char*, VM::brand_score_t> VM::core::brand_scoreboard{
    { brands::VBOX, 0 },
    { brands::VMWARE, 0 },
    { brands::VMWARE_EXPRESS, 0 },
    { brands::VMWARE_ESX, 0 },
    { brands::VMWARE_GSX, 0 },
    { brands::VMWARE_WORKSTATION, 0 },
    { brands::VMWARE_FUSION, 0 },
    { brands::VMWARE_HARD, 0 },
    { brands::BHYVE, 0 },
    { brands::KVM, 0 },
    { brands::QEMU, 0 },
    { brands::QEMU_KVM, 0 },
    { brands::KVM_HYPERV, 0 },
    { brands::QEMU_KVM_HYPERV, 0 },
    { brands::HYPERV, 0 },
    { brands::HYPERV_VPC, 0 },
    { brands::PARALLELS, 0 },
    { brands::XEN, 0 },
    { brands::ACRN, 0 },
    { brands::QNX, 0 },
    { brands::HYBRID, 0 },
    { brands::SANDBOXIE, 0 },
    { brands::DOCKER, 0 },
    { brands::WINE, 0 },
    { brands::VPC, 0 },
    { brands::ANUBIS, 0 },
    { brands::JOEBOX, 0 },
    { brands::THREATEXPERT, 0 },
    { brands::CWSANDBOX, 0 },
    { brands::COMODO, 0 },
    { brands::BOCHS, 0 },
    { brands::NVMM, 0 },
    { brands::BSD_VMM, 0 },
    { brands::INTEL_HAXM, 0 },
    { brands::UNISYS, 0 },
    { brands::LMHS, 0 },
    { brands::CUCKOO, 0 },
    { brands::BLUESTACKS, 0 },
    { brands::JAILHOUSE, 0 },
    { brands::APPLE_VZ, 0 },
    { brands::INTEL_KGT, 0 },
    { brands::AZURE_HYPERV, 0 },
    { brands::NANOVISOR, 0 },
    { brands::SIMPLEVISOR, 0 },
    { brands::HYPERV_ARTIFACT, 0 },
    { brands::UML, 0 },
    { brands::POWERVM, 0 },
    { brands::GCE, 0 },
    { brands::OPENSTACK, 0 },
    { brands::KUBEVIRT, 0 },
    { brands::AWS_NITRO, 0 },
    { brands::PODMAN, 0 },
    { brands::WSL, 0 },
    { brands::OPENVZ, 0 },
    { brands::BAREVISOR, 0 },
    { brands::HYPERPLATFORM, 0 },
    { brands::MINIVISOR, 0 },
    { brands::INTEL_TDX, 0 },
    { brands::LKVM, 0 },
    { brands::AMD_SEV, 0 },
    { brands::AMD_SEV_ES, 0 },
    { brands::AMD_SEV_SNP, 0 },
    { brands::NEKO_PROJECT, 0 },
    { brands::QIHOO, 0 },
    { brands::NOIRVISOR, 0 },
    { brands::NSJAIL, 0 },
    { brands::HYPERVISOR_PHANTOM, 0 },
    { brands::NULL_BRAND, 0 }
};

// these are basically the base values for the core::arg_handler function.
// It's like a bucket that will collect all the bits enabled. If for example 
// VM::detect(VM::HIGH_THRESHOLD) is passed, the HIGH_THRESHOLD bit will be 
// collected in this flagset (std::bitset) variable, and eventually be the 
// return value for actual end-user functions like VM::detect() to rely 
// and work on.
flagset core::flag_collector;
flagset core::disabled_flag_collector;
