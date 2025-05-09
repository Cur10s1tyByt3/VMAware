#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for the VM bit in the SMBIOS data
 * @note idea from https://github.com/systemd/systemd/blob/main/src/basic/virt.c
 * @category Linux
 * @implements VM::SMBIOS_VM_BIT
 */
[[nodiscard]] static bool smbios_vm_bit() {
#if (!LINUX)
    return false;
#else
    if (!util::is_admin()) {
        return false;
    }

    const char* file = "/sys/firmware/dmi/entries/0-0/raw";

    if (!util::exists(file)) {
        return false;
    }

    const std::vector<u8> content = util::read_file_binary(file);

    if (content.size() < 20 || content.at(1) < 20) {
        debug("SMBIOS_VM_BIT: ", "only read ", content.size(), " bytes, expected 20");
        return false;
    }

    debug("SMBIOS_VM_BIT: ", "content.at(19) = ", static_cast<int>(content.at(19)));

    return (content.at(19) & (1 << 4));
#endif
} 