#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for default VM username and hostname for linux
 * @category Linux
 * @implements VM::LINUX_USER_HOST
 */
[[nodiscard]] static bool linux_user_host() {
#if (!LINUX)
    return false;
#else
    if (util::is_admin()) {
        return false;
    }

    const char* username = std::getenv("USER");
    const char* hostname = std::getenv("HOSTNAME");

    if (!username || !hostname) {
        debug("VM::LINUX_USER_HOST: environment variables not found");
        return false;
    }

    debug("LINUX_USER_HOST: user = ", username);
    debug("LINUX_USER_HOST: host = ", hostname);

    return (
        (strcmp(username, "liveuser") == 0) &&
        (strcmp(hostname, "localhost-live") == 0)
    );
#endif
}