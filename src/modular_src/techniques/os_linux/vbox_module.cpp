#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for a VBox kernel module
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::VBOX_MODULE
 */
[[nodiscard]] static bool vbox_module() {
#if (!LINUX)
    return false;
#else
    const char* file = "/proc/modules";

    if (!util::exists(file)) {
        return false;
    }

    const std::string content = util::read_file(file);

    if (util::find(content, "vboxguest")) {
        return core::add(brands::VBOX);
    }

    return false;
#endif
}