#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check MacOS' IO kit registry for VM-specific strings
 * @category MacOS
 * @link https://evasions.checkpoint.com/techniques/macos.html
 * @implements VM::MAC_IOKIT
 */
[[nodiscard]] static bool io_kit() {
#if (!APPLE)
    return false;
#else
    // board_ptr and manufacturer_ptr empty
    std::unique_ptr<std::string> platform_ptr = util::sys_result("ioreg -rd1 -c IOPlatformExpertDevice");
    std::unique_ptr<std::string> board_ptr = util::sys_result("ioreg -rd1 -c board-id");
    std::unique_ptr<std::string> manufacturer_ptr = util::sys_result("ioreg -rd1 -c manufacturer");

    const std::string platform = *platform_ptr;
    const std::string board = *board_ptr;
    const std::string manufacturer = *manufacturer_ptr;

    auto check_platform = [&]() -> bool {
        debug("IO_KIT: ", "platform = ", platform);

        if (platform.empty()) {
            return false;
        }

        for (const char c : platform) {
            if (!std::isdigit(c)) {
                return false;
            }
        }

        return (platform == "0");
        };

    auto check_board = [&]() -> bool {
        debug("IO_KIT: ", "board = ", board);

        if (board.empty()) {
            return false;
        }

        if (util::find(board, "Mac")) {
            return false;
        }

        if (util::find(board, "VirtualBox")) {
            return core::add(brands::VBOX);
        }

        if (util::find(board, "VMware")) {
            return core::add(brands::VMWARE);
        }

        return false;
        };

    auto check_manufacturer = [&]() -> bool {
        debug("IO_KIT: ", "manufacturer = ", manufacturer);

        if (manufacturer.empty()) {
            return false;
        }

        if (util::find(manufacturer, "Apple")) {
            return false;
        }

        if (util::find(manufacturer, "innotek")) {
            return core::add(brands::VBOX);
        }

        return false;
        };

    return (
        check_platform() ||
        check_board() ||
        check_manufacturer()
        );

    return false;
#endif            
}