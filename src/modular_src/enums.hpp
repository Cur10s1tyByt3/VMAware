#pragma once

#include "types.hpp"
#include <bitset>

enum enum_flags : u8 {
    VMID = 0,
    CPU_BRAND,
    HYPERVISOR_BIT,
    HYPERVISOR_STR,
    TIMER,
    THREADCOUNT,
    MAC,
    TEMPERATURE,
    SYSTEMD,
    CVENDOR,
    CTYPE,
    DOCKERENV,
    DMIDECODE,
    DMESG,
    HWMON,
    DLL,
    REGISTRY,
    VM_FILES,
    HWMODEL,
    DISK_SIZE,
    VBOX_DEFAULT,
    VBOX_NETWORK,
/* GPL */ COMPUTER_NAME,
/* GPL */ WINE_CHECK,
/* GPL */ HOSTNAME,
/* GPL */ KVM_DIRS,
/* GPL */ QEMU_DIR,
/* GPL */ POWER_CAPABILITIES,
/* GPL */ SETUPAPI_DISK,
    VM_PROCESSES,
    LINUX_USER_HOST,
    GAMARUE,
    BOCHS_CPU,
    MSSMBIOS,
    MAC_MEMSIZE,
    MAC_IOKIT,
    IOREG_GREP,
    MAC_SIP,
    HKLM_REGISTRIES,
    VPC_INVALID,
    SIDT,
    SGDT,
    SLDT,
    VMWARE_IOMEM,
    VMWARE_IOPORTS,
    VMWARE_SCSI,
    VMWARE_DMESG,
    VMWARE_STR,
    VMWARE_BACKDOOR,
    VMWARE_PORT_MEM,
    SMSW,
    MUTEX,
    ODD_CPU_THREADS,
    INTEL_THREAD_MISMATCH,
    XEON_THREAD_MISMATCH,
    CUCKOO_DIR,
    CUCKOO_PIPE,
    HYPERV_HOSTNAME,
    GENERAL_HOSTNAME,
    SCREEN_RESOLUTION,
    DEVICE_STRING,
    BLUESTACKS_FOLDERS,
    CPUID_SIGNATURE,
    KVM_BITMASK,
    KGT_SIGNATURE,
    QEMU_VIRTUAL_DMI,
    QEMU_USB,
    HYPERVISOR_DIR,
    UML_CPU,
    KMSG,
    VM_PROCS,
    VBOX_MODULE,
    SYSINFO_PROC,
    DEVICE_TREE,
    DMI_SCAN,
    SMBIOS_VM_BIT,
    PODMAN_FILE,
    WSL_PROC,
    DRIVER_NAMES,
    DISK_SERIAL,
    PORT_CONNECTORS,
    IVSHMEM,
    GPU_VM_STRINGS,
    GPU_CAPABILITIES,
    VM_DEVICES,
    PROCESSOR_NUMBER,
    NUMBER_OF_CORES,
    ACPI_TEMPERATURE,
    QEMU_FW_CFG,
    LSHW_QEMU,
    VIRTUAL_PROCESSORS,
    HYPERV_QUERY,
    BAD_POOLS,
    AMD_SEV,
    AMD_THREAD_MISMATCH,
    NATIVE_VHD,
    VIRTUAL_REGISTRY,
    FIRMWARE,
    FILE_ACCESS_HISTORY,
    AUDIO,
    UNKNOWN_MANUFACTURER,
    NSJAIL_PID,
    TPM,
    PCI_VM_DEVICE_ID,
    QEMU_PASSTHROUGH,
    // ADD NEW TECHNIQUE ENUM NAME HERE

    // special flags, different to settings
    DEFAULT,
    ALL,
    NULL_ARG, // does nothing, just a placeholder flag mainly for the CLI

    // start of settings technique flags (THE ORDERING IS VERY SPECIFIC HERE AND MIGHT BREAK SOMETHING IF RE-ORDERED)
    NO_MEMO,
    HIGH_THRESHOLD,
    DYNAMIC,
    MULTIPLE
};

static constexpr u8 enum_size = MULTIPLE; // get enum size through value of last element
static constexpr u8 settings_count = MULTIPLE - NO_MEMO + 1; // get number of settings technique flags like VM::NO_MEMO for example
static constexpr u8 INVALID = 255; // explicit invalid technique macro
static constexpr u16 base_technique_count = NO_MEMO; // original technique count, constant on purpose (can also be used as a base count value if custom techniques are added)
static constexpr u16 maximum_points = 5510; // theoretical total points if all VM detections returned true (which is practically impossible)
static constexpr u16 high_threshold_score = 300; // new threshold score from 150 to 300 if VM::HIGH_THRESHOLD flag is enabled
static constexpr bool SHORTCUT = true; // macro for whether VM::core::run_all() should take a shortcut by skipping the rest of the techniques if the threshold score is already met

static constexpr u8 enum_begin = 0;
static constexpr u8 enum_end = enum_size + 1;
static constexpr u8 technique_begin = enum_begin;
static constexpr u8 technique_end = DEFAULT;
static constexpr u8 settings_begin = DEFAULT;
static constexpr u8 settings_end = enum_end;

using flagset = std::bitset<enum_size + 1>;