#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for VMware string in /proc/scsi/scsi
 * @category Windows
 * @note idea from ScoopyNG by Tobias Klein
 * @implements VM::VMWARE_SCSI
 */
[[nodiscard]] static bool vmware_scsi() {
#if (!LINUX)
    return false;
#else
    const std::string scsi_file = util::read_file("/proc/scsi/scsi");

    if (util::find(scsi_file, "VMware")) {
        return core::add(brands::VMWARE);
    }

    return false;
#endif
}