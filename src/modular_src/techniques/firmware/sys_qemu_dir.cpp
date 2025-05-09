#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for QEMU fw_cfg device
 * @note Windows method extracts 'FWCF' from APCI devices' LocationPaths
 * @note Linux method checks for existence of qemu_fw_cfg dirs within sys/{module, firmware}
 * @category Windows, Linux
 * @implements VM::QEMU_FW_CFG
 */
[[nodiscard]] static bool sys_qemu_dir() {
#if (!LINUX)
    return false;
#else 
    const std::string module_path = "/sys/module/qemu_fw_cfg/";
    const std::string firmware_path = "/sys/firmware/qemu_fw_cfg/";

    if (
        util::is_directory(module_path.c_str()) && 
        util::is_directory(firmware_path.c_str()) &&
        util::exists(module_path.c_str()) &&
        util::exists(firmware_path.c_str())
    ) {
        return core::add(brands::QEMU);
    }

    return false;
#endif
}
