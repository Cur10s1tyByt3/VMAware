#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for default RAM and DISK sizes set by VirtualBox
 * @note        RAM     DISK
 * WINDOWS 11:  4096MB, 80GB
 * WINDOWS 10:  2048MB, 50GB
 * ARCH, OPENSUSE, REDHAD, GENTOO, FEDORA, DEBIAN: 1024MB, 8GB
 * UBUNTU:      1028MB, 10GB
 * ORACLE:      1024MB, 12GB
 * OTHER LINUX: 512MB,  8GB
 
 * @category Linux, Windows
 * @implements VM::VBOX_DEFAULT
 */
[[nodiscard]] static bool vbox_default_specs() {
#if (APPLE)
    return false;
#else
    const u32 disk = util::get_disk_size();
    const u64 ram = util::get_physical_ram_size();

    debug("VBOX_DEFAULT: ram = ", ram);

    if ((disk > 80) || (ram > 4)) {
        debug("VBOX_DEFAULT: returned false due to lack of precondition spec comparisons");
        return false;
    }

    #if (LINUX)
        auto get_distro = []() -> std::string {
            std::ifstream osReleaseFile("/etc/os-release");
            std::string line;

            while (std::getline(osReleaseFile, line)) {
                if (line.find("ID=") != std::string::npos) {
                    const std::size_t start = line.find('"');
                    const std::size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        return line.substr(start + 1, end - start - 1);
                    }
                }
            }

            return "unknown";
            };

        const std::string distro = get_distro();

        debug("VBOX_DEFAULT: linux, detected distro: ", distro);

        // yoda notation ftw
        if ("unknown" == distro) {
            return false;
        }

        if (
            "arch" == distro ||
            "opensuse" == distro ||
            "redhat" == distro ||
            "gentoo" == distro ||
            "fedora" == distro ||
            "debian" == distro
            ) {
            return ((8 == disk) && (1 == ram));
        }

        if ("ubuntu" == distro) {
            return ((10 == disk) && (1 == ram));
        }

        if ("ol" == distro) { // ol = oracle
            return ((12 == disk) && (1 == ram));
        }

        return false;
    #elif (WINDOWS)
        const u8 version = util::get_windows_version();

        if (version < 10) {
            return false;
        }

        if (version == 10) {
            debug("VBOX_DEFAULT: Windows 10 detected");
            return ((50 == disk) && (2 == ram));
        }

        debug("VBOX_DEFAULT: Windows 11 detected");
        return ((80 == disk) && (4 == ram));
    #endif
#endif
}
