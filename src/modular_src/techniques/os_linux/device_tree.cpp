#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for specific files in /proc/device-tree directory
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::DEVICE_TREE
 */
[[nodiscard]] static bool device_tree() {
#if (!LINUX)
    return false;
#else
    if (util::exists("/proc/device-tree/fw-cfg")) {
        return core::add(brands::QEMU);
    }

    return (util::exists("/proc/device-tree/hypervisor/compatible"));
#endif
}