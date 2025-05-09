#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for presence of any files in /sys/hypervisor directory
 * @category Linux
 * @implements VM::HYPERVISOR_DIR
 */
[[nodiscard]] static bool hypervisor_dir() {
#if (!LINUX)
    return false;
#else
    DIR* dir = opendir("/sys/hypervisor");

    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    int count = 0;

    while ((entry = readdir(dir)) != nullptr) {
        if (
            (entry->d_name[0] == '.' && entry->d_name[1] == '\0') || 
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
        ) {
            continue;
        }

        count++;
        break;
    }

    closedir(dir);

    bool type = false;

    if (util::exists("/sys/hypervisor/type")) {
        type = true;
    }

    if (type) {
        const std::string content = util::read_file("/sys/hypervisor/type");
        if (util::find(content, "xen")) {
            return core::add(brands::XEN);
        }
    }

    // check if there's a few files in that directory
    return ((count != 0) && type);
#endif
} 