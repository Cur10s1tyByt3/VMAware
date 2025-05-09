#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"


/**
 * @brief Check for WSL or microsoft indications in /proc/ subdirectories
 * @note idea from https://github.com/systemd/systemd/blob/main/src/basic/virt.c
 * @category Linux
 * @implements VM::WSL_PROC
 */
[[nodiscard]] static bool wsl_proc_subdir() {
#if (!LINUX)
    return false;
#else
    const char* osrelease = "/proc/sys/kernel/osrelease";
    const char* version = "/proc/version";

    if (
        util::exists(osrelease) &&
        util::exists(version)
    ) {
        const std::string osrelease_content = util::read_file(osrelease);
        const std::string version_content = util::read_file(version);

        if (
            (util::find(osrelease_content, "WSL") || util::find(osrelease_content, "Microsoft")) &&
            (util::find(version, "WSL") || util::find(version, "Microsoft"))
        ) {
            return core::add(brands::WSL);
        }
    }

    return false;
#endif
} 