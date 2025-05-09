#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for cuckoo directory using crt and WIN API directory functions
 * @category Windows
 * @author 一半人生
 * @link https://unprotect.it/snippet/checking-specific-folder-name/196/
 * @implements VM::CUCKOO_DIR
 */
[[nodiscard]] static bool cuckoo_dir() {
#if (!WINDOWS)
    return false;
#else
    const DWORD attrs = GetFileAttributesA("C:\\Cuckoo");
    const bool exists = (attrs != INVALID_FILE_ATTRIBUTES) &&
        (attrs & FILE_ATTRIBUTE_DIRECTORY);

    if (exists) {
        return core::add(brands::CUCKOO);
    }

    return false;
#endif
}

