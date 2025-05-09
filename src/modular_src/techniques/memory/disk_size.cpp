#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if disk size is under or equal to 50GB
 * @category Linux, Windows
 * @implements VM::DISK_SIZE
 */
[[nodiscard]] static bool disk_size() {
#if (!LINUX && !WINDOWS)
    return false;
#else
    const u32 size = util::get_disk_size();

    debug("DISK_SIZE: size = ", size);

    return (size <= 80);
#endif
}
