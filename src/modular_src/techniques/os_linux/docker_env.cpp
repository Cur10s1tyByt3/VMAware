#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if /.dockerenv or /.dockerinit file is present
 * @category Linux
 * @implements VM::DOCKERENV
 */
[[nodiscard]] static bool dockerenv() {
#if (!LINUX)
    return false;
#else
    if (util::exists("/.dockerenv") || util::exists("/.dockerinit")) {
        return core::add(brands::DOCKER);
    }

    return false;
#endif
}