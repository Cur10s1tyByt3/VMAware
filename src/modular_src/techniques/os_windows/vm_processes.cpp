#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for any VM processes that are active
 * @category Windows
 * @implements VM::VM_PROCESSES
 */
[[nodiscard]] static bool vm_processes() {
#if (WINDOWS)
    const auto runningProcesses = util::get_running_process_names();

    if (runningProcesses.count("joeboxserver.exe") || runningProcesses.count("joeboxcontrol.exe")) {
        debug("VM_PROCESSES: Detected JoeBox process.");
        return core::add(brands::JOEBOX);
    }

    if (runningProcesses.count("prl_cc.exe") || runningProcesses.count("prl_tools.exe")) {
        debug("VM_PROCESSES: Detected Parallels process.");
        return core::add(brands::PARALLELS);
    }

    if (runningProcesses.count("vboxservice.exe") || runningProcesses.count("vboxtray.exe") || runningProcesses.count("VBoxControl.exe")) {
        debug("VM_PROCESSES: Detected VBox process.");
        return core::add(brands::VBOX);
    }

    if (runningProcesses.count("vmsrvc.exe") || runningProcesses.count("vmusrvc.exe")) {
        debug("VM_PROCESSES: Detected VPC process.");
        return core::add(brands::VPC);
    }

    if (runningProcesses.count("xenservice.exe") || runningProcesses.count("xsvc_depriv.exe")) {
        debug("VM_PROCESSES: Detected Xen process.");
        return core::add(brands::XEN);
    }

    if (runningProcesses.count("vm3dservice.exe") ||
        runningProcesses.count("VGAuthService.exe") ||
        runningProcesses.count("vmtoolsd.exe")) {
        debug("VM_PROCESSES: Detected VMware process.");
        return core::add(brands::VMWARE);
    }

    if (runningProcesses.count("vdagent.exe") ||
        runningProcesses.count("vdservice.exe") ||
        runningProcesses.count("qemuwmi.exe")) {
        debug("VM_PROCESSES: Detected QEMU process.");
        return core::add(brands::QEMU);
    }

    if (runningProcesses.count("looking-glass-host.exe") ||
        runningProcesses.count("VDDSysTray.exe")) {
        return core::add(brands::HYPERVISOR_PHANTOM);
    }
#elif (LINUX)
    if (util::is_proc_running("qemu_ga")) {
        debug("VM_PROCESSES: Detected QEMU guest agent process.");
        return core::add(brands::QEMU);
    }
#endif
    return false;
}