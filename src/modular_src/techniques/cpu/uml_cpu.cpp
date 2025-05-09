#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for the "UML" string in the CPU brand
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::UML_CPU
 */
[[nodiscard]] static bool uml_cpu() {
#if (!LINUX)
    return false;
#else
    // method 1, get the CPU brand model
    const std::string brand = cpu::get_brand();

    if (brand == "UML") {
        return core::add(brands::UML);
    }

    // method 2, match for the "User Mode Linux" string in /proc/cpuinfo
    const char* file = "/proc/cpuinfo";

    if (util::exists(file)) {
        const std::string file_content = util::read_file(file);

        if (util::find(file_content, "User Mode Linux")) {
            return core::add(brands::UML);
        }
    }

    return false;
#endif
} 