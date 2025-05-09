#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for PCI vendor and device IDs that are VM-specific
 * @link https://www.pcilookup.com/?ven=&dev=&action=submit
 * @category Linux
 * @implements VM::PCI_VM_DEVICE_ID
 */
[[nodiscard]] static bool pci_vm_device_id() {
#if (!LINUX)
    return false;
#else
    struct PCI_Device {
        u16 vendor_id;
        u16 device_id;
    };

    const std::string pci_path = "/sys/bus/pci/devices";
    std::vector<PCI_Device> devices;

#if (CPP >= 17)
    for (const auto& entry : std::filesystem::directory_iterator(pci_path)) {
        std::string dev_path = entry.path();
#else 
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(pci_path.c_str())) == nullptr) {
        debug("unable to open the PCI data");
        return false;
    }

    while ((ent = readdir(dir)) != nullptr) {
        std::string dev_name = ent->d_name;
        
        if (dev_name == "." || dev_name == "..") {
            continue;
        }
        
        std::string dev_path = pci_path + "/" + dev_name;
#endif
        PCI_Device dev;

        std::ifstream vendor_file(dev_path + "/vendor");
        std::ifstream device_file(dev_path + "/device");

        vendor_file >> std::hex >> dev.vendor_id;
        device_file >> std::hex >> dev.device_id;

        devices.push_back(dev);
    }

    #ifdef __VMAWARE_DEBUG__
        debug("PCI Device Table");
        debug("-------------------------");
        debug("Vendor ID  | Device ID ");
        debug("-------------------------");

        for (const auto& dev : devices) {
            debug(
                "0x", std::setw(4), std::setfill('0'), std::hex, dev.vendor_id, "     | "
                "0x", std::setw(4), std::setfill('0'), dev.device_id, " | ", std::dec
            );
        }
    #endif

    auto found = [](const std::string &b) -> bool {
        debug(
            "PCI_VM_DEVICE_ID: found ", b, ", vendor ID = ", 
            "0x", std::setw(4), std::setfill('0'), std::hex, dev.vendor_id,
            " device ID = 0x", std::setw(4), std::setfill('0'), std::hex, dev.device_id
        );

        return true;
    };

    for (const auto& dev : devices) {
        const u32 id = ((dev.vendor_id << 16) | dev.device_id);

        switch (id) {
            // Red Hat + Virtio
            case 0x1af41000: // Virtio network device
            case 0x1af41001: // Virtio block device
            case 0x1af41002: //	Virtio memory balloon
            case 0x1af41003: // Virtio console
            case 0x1af41004: // Virtio SCSI
            case 0x1af41005: // Virtio RNG
            case 0x1af41009: // Virtio filesystem
            case 0x1af41041: // Virtio network device
            case 0x1af41042: // Virtio block device
            case 0x1af41043: // Virtio console
            case 0x1af41044: // Virtio RNG
            case 0x1af41045: // Virtio memory balloon
            case 0x1af41048: // Virtio SCSI
            case 0x1af41049: // Virtio filesystem
            case 0x1af41050: // Virtio GPU
            case 0x1af41052: // Virtio input
            case 0x1af41053: // Virtio socket
            case 0x1af4105a: // Virtio file system
            case 0x1af41110: // Inter-VM shared memory
                return true;

            // VMware
            case 0x15ad0405: // SVGA II Adapter
            case 0x15ad0710: // SVGA Adapter
            case 0x15ad0720: // VMXNET Ethernet Controller
            case 0x15ad0740: // Virtual Machine Communication Interface
            case 0x15ad0770: // USB2 EHCI Controller
            case 0x15ad0774: // USB1.1 UHCI Controller
            case 0x15ad0778: // USB3 xHCI 0.96 Controller
            case 0x15ad0779: // USB3 xHCI 1.0 Controller
            case 0x15ad0790: // PCI bridge
            case 0x15ad07a0: // PCI Express Root Port
            case 0x15ad07b0: // VMXNET3 Ethernet Controller
            case 0x15ad07c0: // PVSCSI SCSI Controller
            case 0x15ad07e0: // SATA AHCI controller
            case 0x15ad07f0: // NVMe SSD Controller
            case 0x15ad0801: // Virtual Machine Interface
            case 0x15ad0820: // Paravirtual RDMA controller
            case 0x15ad1977: // HD Audio Controller
            case 0xfffe0710: // Virtual SVGA
            case 0x0e0f0001: // Device
            case 0x0e0f0002: // Virtual USB Hub
            case 0x0e0f0003: // Virtual Mouse
            case 0x0e0f0004: // Virtual CCID
            case 0x0e0f0005: // Virtual Mass Storage
            case 0x0e0f0006: // Virtual Keyboard
            case 0x0e0f000a: // Virtual Sensors
            case 0x0e0f8001: // Root Hub
            case 0x0e0f8002: // Root Hub
            case 0x0e0f8003: // Root Hub
            case 0x0e0ff80a: // Smoker FX2
                return found(brands::VMWARE);

            // Red Hat + QEMU
            case 0x1b360001: // Red Hat, Inc. QEMU PCI-PCI bridge
            case 0x1b360002: // Red Hat, Inc. QEMU PCI 16550A Adapter
            case 0x1b360003: // Red Hat, Inc. QEMU PCI Dual-port 16550A Adapter
            case 0x1b360004: // Red Hat, Inc. QEMU PCI Quad-port 16550A Adapter
            case 0x1b360005: // Red Hat, Inc. QEMU PCI Test Device
            case 0x1b360008: // Red Hat, Inc. QEMU PCIe Host bridge
            case 0x1b360009: // Red Hat, Inc. QEMU PCI Expander bridge
            case 0x1b36000b: // Red Hat, Inc. QEMU PCIe Expander bridge
            case 0x1b36000c: // Red Hat, Inc. QEMU PCIe Root port
            case 0x1b36000d: // Red Hat, Inc. QEMU XHCI Host Controller
            case 0x1b360010: // Red Hat, Inc. QEMU NVM Express Controller
            case 0x1b360011: // Red Hat, Inc. QEMU PVPanic device
            case 0x1b360013: // Red Hat, Inc. QEMU UFS Host Controller
            case 0x1b360100: // Red Hat, Inc. QXL paravirtual graphic card

            // QEMU
            case 0x06270001: // Adomax Technology Co., Ltd QEMU Tablet
            case 0x1d1d1f1f: // CNEX Labs QEMU NVM Express LightNVM Controller
            case 0x80865845: // Intel Corporation QEMU NVM Express Controller
            case 0x1d6b0200: // Linux Foundation Qemu Audio Device
                return found(brands::QEMU);

            // vGPUs (mostly NVIDIA)
            case 0x10de0fe7: // GK107GL [GRID K100 vGPU]
            case 0x10de0ff7: // GK107GL [GRID K140Q vGPU]
            case 0x10de118d: // GK104GL [GRID K200 vGPU]
            case 0x10de11b0: // GK104GL [GRID K240Q\K260Q vGPU]
            case 0x1ec6020f: // Vastai Technologies SG100 vGPU
                return true;
            
            // VirtualBox
            case 0x80ee0021: // USB Tablet
            case 0x80ee0022: // multitouch tablet
                return found(brands::VBOX);
            
            // Connectix (VirtualPC) OHCI USB 1.1 controller
            case 0x29556e61: return found(brands::VPC);
        
            // Parallels, Inc.	Virtual Machine Communication Interface
            case 0x1ab84000: return found(brands::PARALLELS);
        }
        
        // TODO: EXTRAS TO ADD (64 instead of 32 bits for device_id field)
        // 
        // Advanced Micro Devices, Inc. [AMD]	1022	QEMU Virtual Machine	1af41100
        // Apple Inc.	106b	QEMU Virtual Machine	1af41100
        // Cirrus Logic	1013	QEMU Virtual Machine	1af41100
        // Intel Corporation	8086	QEMU Virtual Machine	1af41100
        // NEC Corporation	1033	QEMU Virtual Machine	1af41100
        // Realtek Semiconductor Co., Ltd.	10ec	QEMU Virtual Machine	1af41100
        // VIA Technologies, Inc.	1106	QEMU Virtual Machine	1af41100
        // Red Hat, Inc. 1af4 QEMU Virtual Machine	1af41100
        // Red Hat, Inc. 1b36 QEMU Virtual Machine	1af41100
    }
    
    return false;
#endif
}