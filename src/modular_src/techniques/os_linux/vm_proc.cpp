#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VM processes
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::VM_PROCS
 */
[[nodiscard]] static bool vm_procs() {
#if (!LINUX)
        return false;
#else
        if (util::exists("/proc/xen")) {
            return core::add(brands::XEN);
        }

        if (util::exists("/proc/vz")) {
            return core::add(brands::OPENVZ);
        }

        return false;
#endif
    } 