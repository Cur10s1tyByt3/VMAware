#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if process status matches with nsjail patterns with PID anomalies
 * @category Linux
 * @implements VM::NSJAIL_PID
 */
[[nodiscard]] static bool nsjail_proc_id() {
#if (!LINUX)
    return false;
#else
    std::ifstream status_file("/proc/self/status");
    std::string line;
    bool pid_match = false;
    bool ppid_match = false;

    while (std::getline(status_file, line)) {
        if (line.find("Pid:") == 0) {
            std::string num_str = "";
            for (char ch : line) {
                if (isdigit(ch)) {
                    num_str += ch;
                }
            }

            if (num_str.empty()) {
                return false;
            }

            if (std::stoi(num_str) == 1) {
                pid_match = true;
            }
        }

        if (line.find("PPid:") == 0) {
            std::string num_str = "";
            for (char ch : line) {
                if (isdigit(ch)) {
                    num_str += ch;
                }
            }

            if (num_str.empty()) {
                return false;
            }

            if (std::stoi(num_str) == 0) {
                ppid_match = true;
            }
        }
    }

    if (pid_match && ppid_match) {
        return core::add(brands::NSJAIL);
    }

    return false;
#endif
}