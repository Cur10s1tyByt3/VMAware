#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for any indications of hypervisors in the kernel message logs
 * @note idea from https://github.com/ShellCode33/VM-Detection/blob/master/vmdetect/linux.go
 * @category Linux
 * @implements VM::KMSG
 */
[[nodiscard]] static bool kmsg() {
#if (!LINUX)
    return false;
#else
    if (!util::is_admin()) {
        return false;
    }

    int fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        debug("KMSG: Failed to open /dev/kmsg");
        return false;
    }

    char buffer[1024];
    std::stringstream ss;

    while (true) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            ss << buffer;
        } else if (bytes_read == 0) {
            usleep(100000); // Sleep for 100 milliseconds
        } else {
            if (errno == EAGAIN) {
                usleep(100000); // Sleep for 100 milliseconds
            } else {
                debug("KMSG: Error reading /dev/kmsg");
                break;
            }
        }

        if (bytes_read < 0) {
            break;
        }
    }

    close(fd);

    const std::string content = ss.str();

    if (content.empty()) {
        return false;
    }

    return (util::find(content, "Hypervisor detected"));
#endif
} 