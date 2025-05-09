#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"


/**
 * @brief Check for the presence of BlueStacks-specific folders
 * @category ARM, Linux
 * @implements VM::BLUESTACKS_FOLDERS
 */
[[nodiscard]] static bool bluestacks() {
#if (!(ARM && LINUX))
    return false;
#else
    if (
        util::exists("/mnt/windows/BstSharedFolder") ||
        util::exists("/sdcard/windows/BstSharedFolder")
    ) {
        return core::add(brands::BLUESTACKS);
    }

    return false;
#endif
}