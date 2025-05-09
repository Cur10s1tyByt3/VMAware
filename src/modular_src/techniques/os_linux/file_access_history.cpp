#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if the number of accessed files are too low for a human-managed environment
 * @category Linux
 * @note idea from https://unprotect.it/technique/xbel-recently-opened-files-check/
 * @implements VM::FILE_ACCESS_HISTORY
 */
[[nodiscard]] static bool file_access_history() {
#if (!LINUX)
    return false;
#else 
    const std::string xbel_file = util::read_file("~/.local/share/recently-used.xbel");
    
    if (xbel_file.empty()) {
        debug("FILE_ACCESS_HISTORY: file content is empty");
        return false;
    }

    const std::string key = "href";

    u32 count = 0;
    std::size_t pos = 0;

    while ((pos = xbel_file.find(key, pos)) != std::string::npos) {
        count++;
        pos += key.length();
    }

    return (count <= 10); 
#endif
}