#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for potential VM info in /proc/sysinfo
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::SYSINFO_PROC
 */
[[nodiscard]] static bool sysinfo_proc() {
#if (!LINUX)
    return false;
#else
    const char* file = "/proc/sysinfo";

    if (!util::exists(file)) {
        return false;
    }

    const std::string content = util::read_file(file);

    if (util::find(content, "VM00")) {
        return true;
    }

    return false;
#endif
} 