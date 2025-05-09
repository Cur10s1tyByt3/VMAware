#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Find for VM specific files
 * @category Windows
 * @implements VM::VM_FILES
 */
[[nodiscard]] static bool vm_files() {
#if !WINDOWS
    return false;
#else
    PVOID oldValue = nullptr;
    bool wow64RedirectDisabled = false;

    if (util::is_wow64()) {
        Wow64DisableWow64FsRedirection(&oldValue);
        wow64RedirectDisabled = true;
    }

    // System directory instead of the Windows directory
    char szSysDir[MAX_PATH] = { 0 };
    if (GetSystemDirectoryA(szSysDir, MAX_PATH) == 0) {
        if (wow64RedirectDisabled) {
            Wow64RevertWow64FsRedirection(&oldValue);
        }
        return false;
    }

    constexpr std::array<const char*, 12> vmwareFiles = { {
        "drivers\\Vmmouse.sys",
        "drivers\\Vmusbmouse.sys",
        "drivers\\vm3dgl.dll",
        "drivers\\vmdum.dll",
        "drivers\\VmGuestLibJava.dll",
        "drivers\\vm3dver.dll",
        "drivers\\vmtray.dll",
        "drivers\\VMToolsHook.dll",
        "drivers\\vmGuestLib.dll",
        "drivers\\vmhgfs.dll",
        "vm3dum64_loader.dll",
        "vm3dum64_10.dll"
    } };

    constexpr std::array<const char*, 17> vboxFiles = { {
            "drivers\\VBoxMouse.sys",
            "drivers\\VBoxGuest.sys",
            "drivers\\VBoxSF.sys",
            "drivers\\VBoxVideo.sys",
            "vboxoglpackspu.dll",
            "vboxoglpassthroughspu.dll",
            "vboxservice.exe",
            "vboxoglcrutil.dll",
            "vboxdisp.dll",
            "vboxhook.dll",
            "vboxmrxnp.dll",
            "vboxogl.dll",
            "vboxtray.exe",
            "VBoxControl.exe",
            "vboxoglerrorspu.dll",
            "vboxoglfeedbackspu.dll",
            "vboxoglarrayspu.dll"
        } };

    constexpr std::array<const char*, 10> kvmFiles = { {
        "drivers\\balloon.sys",
        "drivers\\netkvm.sys",
        "drivers\\pvpanic.sys",
        "drivers\\viofs.sys",
        "drivers\\viogpudo.sys",
        "drivers\\vioinput.sys",
        "drivers\\viorng.sys",
        "drivers\\vioscsi.sys",
        "drivers\\vioser.sys",
        "drivers\\viostor.sys"
    } };

    constexpr std::array<const char*, 7> parallelsFiles = { {
        "drivers\\prleth.sys",
        "drivers\\prlfs.sys",
        "drivers\\prlmouse.sys",
        "drivers\\prlvideo.sys",
        "drivers\\prltime.sys",
        "drivers\\prl_pv32.sys",
        "drivers\\prl_paravirt_32.sys"
    } };

    constexpr std::array<const char*, 2> vpcFiles = { {
        "drivers\\vmsrvc.sys",
        "drivers\\vpc-s3.sys"
    } };

    u8 vmware = 0, vbox = 0, kvm = 0, vpc = 0, parallels = 0;

    auto checkFiles = [&](const auto& files, u8& count) {
        for (const auto& relativePath : files) {
            char szPath[MAX_PATH] = { 0 };
            // Combination of the system directory with the relative path
            PathCombineA(szPath, szSysDir, relativePath);
            if (util::exists(szPath)) {
                count++;
            }
        }
        };

    checkFiles(vmwareFiles, vmware);
    checkFiles(vboxFiles, vbox);
    checkFiles(kvmFiles, kvm);
    checkFiles(parallelsFiles, parallels);
    checkFiles(vpcFiles, vpc);

    if (wow64RedirectDisabled) {
        Wow64RevertWow64FsRedirection(&oldValue);
    }

    if (util::exists("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\agent.pyw")) {
        debug("VM Files: Found startup agent (agent.pyw), indicating CUCKOO VM.");
        return core::add(brands::CUCKOO);
    }
    if (vbox > vmware && vbox > kvm && vbox > vpc && vbox > parallels) {
        debug("VM Files: Detected VBox files with count ", vbox);
        return core::add(brands::VBOX);
    }
    if (vmware > vbox && vmware > kvm && vmware > vpc && vmware > parallels) {
        debug("VM Files: Detected VMware files with count ", vmware);
        return core::add(brands::VMWARE);
    }
    if (kvm > vbox && kvm > vmware && kvm > vpc && kvm > parallels) {
        debug("VM Files: Detected KVM files with count ", kvm);
        return core::add(brands::KVM);
    }
    if (vpc > vbox && vpc > vmware && vpc > kvm && vpc > parallels) {
        debug("VM Files: Detected VPC files with count ", vpc);
        return core::add(brands::VPC);
    }
    if (parallels > vbox && parallels > vmware && parallels > kvm && parallels > vpc) {
        debug("VM Files: Detected Parallels files with count ", parallels);
        return core::add(brands::PARALLELS);
    }

    return false;
#endif
}