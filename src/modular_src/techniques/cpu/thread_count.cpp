#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if there are only 1 or 2 threads, which is a common pattern in VMs with default settings (nowadays physical CPUs should have at least 4 threads for modern CPUs
 * @category x86 (ARM might have very low thread counts, which is why it should be only for x86)
 * @implements VM::THREADCOUNT
 */
[[nodiscard]] bool techniques::thread_count() {
#if (x86)
    debug("THREADCOUNT: ", "threads = ", std::thread::hardware_concurrency());

    struct cpu::stepping_struct steps = cpu::fetch_steppings();

    if (cpu::is_celeron(steps)) {
        return false;
    }

    return (std::thread::hardware_concurrency() <= 2);
#else 
    return false;
#endif
}