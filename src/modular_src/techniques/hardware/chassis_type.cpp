#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if the chassis type is valid (it's very often invalid in VMs)
 * @category Linux
 * @implements VM::CTYPE
 */
[[nodiscard]] bool techniques::chassis_type() {
#if (!LINUX)
    return false;
#else
    const char* chassis = "/sys/devices/virtual/dmi/id/chassis_type";

    if (util::exists(chassis)) {
        return (stoi(util::read_file(chassis)) == 1);
    } else {
        debug("CTYPE: ", "file doesn't exist");
    }

    return false;
#endif
}