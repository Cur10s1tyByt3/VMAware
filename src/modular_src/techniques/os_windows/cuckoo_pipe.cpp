#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for Cuckoo specific piping mechanism
 * @category Windows
 * @author Thomas Roccia (fr0gger)
 * @link https://unprotect.it/snippet/checking-specific-folder-name/196/
 * @implements VM::CUCKOO_PIPE
 */
[[nodiscard]] static bool cuckoo_pipe() {
#if (!LINUX)
    return false;
#else
    int fd = open("\\\\.\\pipe\\cuckoo", O_RDONLY);
    bool is_cuckoo = false;

    if (fd >= 0) {
        is_cuckoo = true;
    }

    close(fd);

    if (is_cuckoo) {
        return core::add(brands::CUCKOO);
    }

    return false;
#endif
}