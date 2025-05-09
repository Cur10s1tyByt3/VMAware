#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
* @brief Check for VMware string in /proc/iomem
* @category Linux
* @note idea from ScoopyNG by Tobias Klein
* @implements VM::VMWARE_IOMEM
*/
[[nodiscard]] static bool vmware_iomem() {
#if (!LINUX)
    return false;
#else
    const std::string iomem_file = util::read_file("/proc/iomem");

    if (util::find(iomem_file, "VMware")) {
        return core::add(brands::VMWARE);
    }

    return false;
#endif
}
