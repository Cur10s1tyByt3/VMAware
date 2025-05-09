#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if /sys/class/hwmon/ directory is present. If not, likely a VM
 * @category Linux
 * @implements VM::HWMON
 */
[[nodiscard]] static bool hwmon() {
#if (!LINUX)
    return false;
#else
    return (!util::exists("/sys/class/hwmon/"));
#endif
}