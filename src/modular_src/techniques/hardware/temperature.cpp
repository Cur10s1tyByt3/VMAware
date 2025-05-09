#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"

/**
 * @brief Check if thermal directory in linux is present, might not be present in VMs
 * @category Linux
 * @implements VM::TEMPERATURE
 */
[[nodiscard]] bool techniques::temperature() {
#if (!LINUX)
    return false;
#else
    return (!util::exists("/sys/class/thermal/thermal_zone0/"));
#endif
}