#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"



/**
 * @brief Check for pre-set screen resolutions commonly found in VMs
 * @category Windows
 * @note Idea from Thomas Roccia (fr0gger)
 * @link https://unprotect.it/technique/checking-screen-resolution/
 * @implements VM::SCREEN_RESOLUTION
 */
[[nodiscard]] static bool screen_resolution() {
#if (!WINDOWS)
    return false;
#else
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    const i32 horiz = desktop.right;
    const i32 verti = desktop.bottom;

    debug("SCREEN_RESOLUTION: horizontal = ", horiz, ", vertical = ", verti);

    if (
        (horiz == 1024 && verti == 768) ||
        (horiz == 800 && verti == 600) ||
        (horiz == 640 && verti == 480)
    ) {
        return true;
    }

    return false;
#endif
}