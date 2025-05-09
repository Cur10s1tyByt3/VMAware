#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for podman file in /run/
 * @note idea from https://github.com/systemd/systemd/blob/main/src/basic/virt.c
 * @category Linux
 * @implements VM::PODMAN_FILE
 */
[[nodiscard]] static bool podman_file() {
#if (!LINUX)
    return false;
#else
    if (util::exists("/run/.containerenv")) {
        return core::add(brands::PODMAN);
    }

    return false;
#endif
}