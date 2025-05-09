#pragma once

#include "cpu.hpp"
#include "../types.hpp"
#include "../hyper_x.hpp"
#include <unordered_set>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <array>
#if (CPP >= 17)
#include <filesystem>
#endif

#ifdef __VMAWARE_DEBUG__
#define debug(...) util::debug_msg(__VA_ARGS__)
#define core_debug(...) util::core_debug_msg(__VA_ARGS__)
#else
#define debug(...)
#define core_debug(...)
#endif


// miscellaneous functionalities
struct util {
#if (LINUX)
    // fetch file data
    [[nodiscard]] static std::string read_file(const char* raw_path) {
        std::string path = "";
        const std::string raw_path_str = raw_path;

        // replace the "~" part with the home directory
        if (raw_path[0] == '~') {
            const char* home = std::getenv("HOME");
            if (home) {
                path = std::string(home) + raw_path_str.substr(1);
            }
        } else {
            path = raw_path;
        }

        if (!exists(path.c_str())) {
            return "";
        }

        std::ifstream file{};
        std::string data{};
        std::string line{};

        file.open(path);

        if (file.is_open()) {
            while (std::getline(file, line)) {
                data += line + "\n";
            }
        }

        file.close();
        return data;
    }
#endif

    // fetch the file but in binary form
    [[nodiscard]] static std::vector<u8> read_file_binary(const char* file_path) {
        std::ifstream file(file_path, std::ios::binary);

        if (!file) {
            return {};
        }

        std::vector<u8> buffer;
        std::istreambuf_iterator<char> it(file);
        std::istreambuf_iterator<char> end;

        while (it != end) {
            buffer.push_back(static_cast<u8>(*it));
            ++it;
        }

        file.close();

        return buffer;
    }

    // check if file exists
    [[nodiscard]] static bool exists(const char* path) {
#if (WINDOWS)
        return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) || (GetLastError() != ERROR_FILE_NOT_FOUND);
#else 
    #if (CPP >= 17)
        return std::filesystem::exists(path);
    #elif (CPP >= 11)
        struct stat buffer;
        return (stat(path, &buffer) == 0);
    #else 
        debug("exists(): no suitable C++ standard found");
        return false;
    #endif
#endif
    }

#if (WINDOWS) && (defined(UNICODE) || defined(_UNICODE))
    // handle TCHAR conversion
    [[nodiscard]] static bool exists(const TCHAR* path) {
        char c_szText[_MAX_PATH]{};
        size_t convertedChars = 0;
        wcstombs_s(&convertedChars, c_szText, path, _MAX_PATH);
        return exists(c_szText);
    }
#endif

#if (LINUX)
    static bool is_directory(const char* path) {
        struct stat info;
        if (stat(path, &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR); // check if directory
    };
#endif

    // wrapper for std::make_unique because it's not available for C++11
    template<typename T, typename... Args>
    [[nodiscard]] static std::unique_ptr<T> make_unique(Args&&... args) {
#if (CPP < 14)
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#else
        return std::make_unique<T>(std::forward<Args>(args)...);
#endif
    }

    // self-explanatory
    [[nodiscard]] static bool is_admin() noexcept {
#if (LINUX || APPLE)
        const uid_t uid = getuid();
        const uid_t euid = geteuid();

        return (
            (uid != euid) ||
            (euid == 0)
        );
#elif (WINDOWS)
        BOOL is_admin = FALSE;
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DWORD dwSize = 0;
            if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                PTOKEN_MANDATORY_LABEL pTIL = (PTOKEN_MANDATORY_LABEL)malloc(dwSize);
                if (pTIL != NULL) {
                    if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize)) {
                        SID* pSID = (SID*)GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));
                        DWORD dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

                        if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) {
                            is_admin = TRUE;
                        }

                        UNUSED(pSID);
                    }
                    free(pTIL);
                }
            }
        }

        CloseHandle(hToken); 

        return is_admin;
#else 
        debug("is_admin(): no suitable OS found, returning false");
        return false;
#endif
    }

    // scan for keyword in string
    [[nodiscard]] static bool find(const std::string& base_str, const char* keyword) noexcept {
        return (base_str.find(keyword) != std::string::npos);
    };

    // for debug output
#ifdef __VMAWARE_DEBUG__
#if (CPP < 17)
    // Helper function to handle the recursion
    static inline void print_to_stream(std::ostream&) noexcept {
        // Base case: do nothing
    }

    template <typename T, typename... Args>
    static void print_to_stream(std::ostream& os, T&& first, Args&&... args) noexcept {
        os << std::forward<T>(first);
        using expander = int[];
        (void)expander {
            0, (void(os << std::forward<Args>(args)), 0)...
        };
    }
#endif

    template <typename... Args>
    static inline void debug_msg(Args... message) noexcept {
#if (LINUX || APPLE)
        constexpr const char* black_bg = "\x1B[48;2;0;0;0m";
        constexpr const char* bold = "\033[1m";
        constexpr const char* blue = "\x1B[38;2;00;59;193m";
        constexpr const char* ansiexit = "\x1B[0m";

        std::cout.setf(std::ios::fixed, std::ios::floatfield);
        std::cout.setf(std::ios::showpoint);

        std::cout << black_bg << bold << "[" << blue << "DEBUG" << ansiexit << bold << black_bg << "]" << ansiexit << " ";
#else       
        std::cout << "[DEBUG] ";
#endif

#if (CPP >= 17)
        ((std::cout << message), ...);
#else
        print_to_stream(std::cout, message...);
#endif

        std::cout << std::dec << "\n";
    }

    template <typename... Args>
    static inline void core_debug_msg(Args... message) noexcept {
#if (LINUX || APPLE)
        constexpr const char* black_bg = "\x1B[48;2;0;0;0m";
        constexpr const char* bold = "\033[1m";
        constexpr const char* blue = "\x1B[38;2;255;180;5m";
        constexpr const char* ansiexit = "\x1B[0m";

        std::cout.setf(std::ios::fixed, std::ios::floatfield);
        std::cout.setf(std::ios::showpoint);

        std::cout << black_bg << bold << "[" << blue << "CORE DEBUG" << ansiexit << bold << black_bg << "]" << ansiexit << " ";
#else       
        std::cout << "[CORE DEBUG] ";
#endif

#if (CPP >= 17)
        ((std::cout << message), ...);
#else
        print_to_stream(std::cout, message...);
#endif

        std::cout << std::dec << "\n";
    }
#endif

    // basically std::system but it runs in the background with std::string output
    [[nodiscard]] static std::unique_ptr<std::string> sys_result(const TCHAR* cmd) {
#if (CPP < 14)
        std::unique_ptr<std::string> tmp(nullptr);
        UNUSED(cmd);
        return tmp;
#else
#if (LINUX || APPLE)
#if (ARM)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif
        std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose);

#if (ARM)
#pragma GCC diagnostic pop
#endif

        if (!pipe) {
            return nullptr;
        }

        std::string result{};
        std::array<char, 128> buffer{};

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        result.pop_back();

        return util::make_unique<std::string>(result);
#elif (WINDOWS)
        // Set up the structures for creating the process
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);

        // Create a pipe to capture the command output
        HANDLE hReadPipe, hWritePipe;
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            debug("sys_result: ", "error creating pipe");

            return nullptr;
        }

        // Set up the startup information with the write end of the pipe as the standard output
        si.hStdError = hWritePipe;
        si.hStdOutput = hWritePipe;
        si.dwFlags |= STARTF_USESTDHANDLES;

        // Create the process
        if (!CreateProcess(NULL, const_cast<TCHAR*>(cmd), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            debug("sys_result: ", "error creating process");

            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return nullptr;
        }

        // Close the write end of the pipe as it's not needed in this process
        CloseHandle(hWritePipe);

        // Read the output from the pipe
        char buffer[4096];
        DWORD bytesRead;
        std::string result;

        while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
            result.append(buffer, bytesRead);
        }

        // Close handles
        CloseHandle(hReadPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Return the result as a unique_ptr<string>
        return util::make_unique<std::string>(result);
#endif
#endif
    }

    /**
        * @brief Get the disk size in GB
        * @category Linux, Windows
        * @returns Disk size in GB
        */
    [[nodiscard]] static u32 get_disk_size() {
        u32 size = 0;
        constexpr u64 GB = (static_cast<u64>(1024) * 1024 * 1024);  // Size of 1 GB in bytes

#if (LINUX)
        struct statvfs stat;

        if (statvfs("/", &stat) != 0) {
            debug("private util::get_disk_size( function: ", "failed to fetch disk size");
            return 0; // Return 0 to indicate failure
        }

        // in gigabytes
        size = static_cast<u32>((stat.f_blocks * stat.f_frsize) / GB);
#elif (WINDOWS)
        ULARGE_INTEGER totalNumberOfBytes;

        if (GetDiskFreeSpaceExW(
            L"C:",  // Drive or directory path (use wide character string)
            nullptr,  // Free bytes available to the caller (not needed for total size)
            &totalNumberOfBytes,  // Total number of bytes on the disk
            nullptr  // Total number of free bytes on the disk (not needed for total size)
        )) {
            // Convert bytes to GB
            size = static_cast<u32>(totalNumberOfBytes.QuadPart / GB);
        }
        else {
            debug("util::get_disk_size(: ", "failed to fetch size in GB");
        }
#endif

        if (size == 0)
            return 81;
        
        return size;  // Return disk size in GB
    }

    // get physical RAM size in GB
    [[nodiscard]] static u64 get_physical_ram_size() {
#if (LINUX)
        if (!util::is_admin()) {
            debug("private get_physical_ram_size function: ", "not root, returned 0");
            return 0;
        }

        auto result = util::sys_result("dmidecode --type 19 | grep 'Size' | grep '[[:digit:]]*'");

        if (result == nullptr) {
            debug("private get_physical_ram_size function: ", "invalid system result from dmidecode, returned 0");
            return 0;
        }

        const bool MB = (std::regex_search(*result, std::regex("MB")));
        const bool GB = (std::regex_search(*result, std::regex("GB")));

        if (!(MB || GB)) {
            debug("private get_physical_ram_size function: ", "neither MB nor GB found, returned 0");
            return 0;
        }

        std::string number_str;
        bool in_number = false;

        for (char c : *result) {
            if (std::isdigit(c)) {
                number_str += c;
                in_number = true;
            } else if (in_number) {
                break;
            }
        }

        if (number_str.empty()) {
            debug("private get_physical_ram_size_gb function: ", "string is empty, returned 0");
            return 0;
        }

        u64 number = 0;

        number = std::stoull(number_str);

        if (MB == true) {
            number = static_cast<u64>(std::round(number / 1024));
        }

        return number; // in GB
#elif (WINDOWS)
        ULONGLONG total_memory_kb = 0;

        if (GetPhysicallyInstalledSystemMemory(&total_memory_kb) == ERROR_INVALID_DATA) {
            return 0;
        }

        return (total_memory_kb / (static_cast<unsigned long long>(1024) * 1024)); // MB
#else
        return 0;
#endif
    }

    // Get available memory space
    [[nodiscard]] static u64 get_memory_space() {
#if (WINDOWS)
        MEMORYSTATUSEX statex = { 0 };
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex); // calls NtQuerySystemInformation
        return statex.ullTotalPhys;
#elif (LINUX)
        const i64 pages = sysconf(_SC_PHYS_PAGES);
        const i64 page_size = sysconf(_SC_PAGE_SIZE);
        return (pages * page_size);
#elif (APPLE)
        i32 mib[2] = { CTL_HW, HW_MEMSIZE };
        u32 namelen = sizeof(mib) / sizeof(mib[0]);
        u64 size = 0;
        std::size_t len = sizeof(size);

        if (sysctl(mib, namelen, &size, &len, NULL, 0) < 0) {
            return 0;
        }

        return size; // in bytes
#else 
        debug("get_memory_space error: no suitable OS found, returning 0");
        return 0;
#endif
    }

    // Checks if a process is running
    [[nodiscard]] static bool is_proc_running(const char* executable) {
#if (WINDOWS)
        DWORD processes[1024], bytesReturned;

        if (!K32EnumProcesses(processes, sizeof(processes), &bytesReturned))
            return false;

        DWORD numProcesses = bytesReturned / sizeof(DWORD);

        for (DWORD i = 0; i < numProcesses; ++i) {
            const HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (process != nullptr) {
                char processName[MAX_PATH] = { 0 };
                if (K32GetModuleBaseNameA(process, nullptr, processName, sizeof(processName))) {
                    if (_stricmp(processName, executable) == 0) {
                        CloseHandle(process);
                        return true;
                    }
                }
                CloseHandle(process);
            }
        }

        return false;
#elif (LINUX)
#if (CPP >= 17)
        for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
            if (!(entry.is_directory())) {
                continue;
            }

            const std::string filename = entry.path().filename().string();
#else
        //DIR* dir = opendir("/proc/");
        std::unique_ptr<DIR, decltype(&closedir)> dir(opendir("/proc"), closedir);
        if (!dir) {
            debug("util::is_proc_running: ", "failed to open /proc directory");
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir.get())) != nullptr) {
            std::string filename(entry->d_name);
            if (filename == "." || filename == "..") {
                continue;
            }
#endif
            if (!(std::all_of(filename.begin(), filename.end(), ::isdigit))) {
                continue;
            }

            const std::string cmdline_file = "/proc/" + filename + "/cmdline";
            std::ifstream cmdline(cmdline_file);
            if (!(cmdline.is_open())) {
                continue;
            }

            std::string line;
            std::getline(cmdline, line);
            cmdline.close();

            if (line.empty()) {
                continue;
            }

            //std::cout << "\n\nLINE = " << line << "\n";
            if (line.find(executable) == std::string::npos) {
                //std::cout << "skipped\n";
                continue;
            }

            //std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nNOT SKIPPED\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

            const std::size_t slash_index = line.find_last_of('/');

            if (slash_index == std::string::npos) {
                continue;
            }

            line = line.substr(slash_index + 1);

            const std::size_t space_index = line.find_first_of(' ');

            if (space_index != std::string::npos) {
                line = line.substr(0, space_index);
            }

            if (line != executable) {
                continue;
            }
            //#if (CPP < 17)
            //                closedir(dir);
            //                free(dir);
            //#endif
            return true;
        }

        return false;
#else
        return false;
#endif
    }

    // Returns a list of running process names
    [[nodiscard]] static std::unordered_set<std::string> get_running_process_names() {
        std::unordered_set<std::string> processNames;
#if (WINDOWS)
        typedef NTSTATUS(__stdcall* PFN_NtQuerySystemInformation)(
            SYSTEM_INFORMATION_CLASS,
            PVOID,
            ULONG,
            PULONG
            );

        static PFN_NtQuerySystemInformation pNtQSI = nullptr;
        static std::vector<BYTE> buffer;
        static std::mutex initMutex;

        if (!pNtQSI) {
            std::lock_guard<std::mutex> lk(initMutex);
            if (!pNtQSI) {
                const HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
                if (hNtdll) {
#pragma warning (disable : 4191)
                    pNtQSI = reinterpret_cast<PFN_NtQuerySystemInformation>(GetProcAddress(hNtdll, "NtQuerySystemInformation"));
#pragma warning (default : 4191)                       
                }
                if (!pNtQSI) {
                    return {}; 
                }
            }
        }

        ULONG needed = 0;
        NTSTATUS status = pNtQSI(SystemProcessInformation, nullptr, 0, &needed);
        if (status != 0) {
            return {};
        }

        if (buffer.size() < needed) {
            buffer.resize(needed);
        }

        status = pNtQSI(
            SystemProcessInformation,
            buffer.data(),
            static_cast<ULONG>(buffer.size()),
            &needed
        );
        if (!NT_SUCCESS(status)) {
            return {};
        }

        const BYTE* cur = buffer.data();
        while (true) {
            auto pi = reinterpret_cast<const SYSTEM_PROCESS_INFORMATION*>(cur);
            if (pi->ImageName.Buffer && pi->ImageName.Length) {
                int wideChars = static_cast<int>(pi->ImageName.Length / sizeof(WCHAR));
                int utf8Len = WideCharToMultiByte(
                    CP_UTF8, 0,
                    pi->ImageName.Buffer, wideChars,
                    nullptr, 0,
                    nullptr, nullptr
                );
                if (utf8Len > 0) {
                    std::string name;
                    name.resize(static_cast<size_t>(utf8Len));
                    WideCharToMultiByte(
                        CP_UTF8, 0,
                        pi->ImageName.Buffer, wideChars,
                        &name[0], utf8Len,
                        nullptr, nullptr
                    );
                    processNames.insert(std::move(name));
                }
            }
            if (pi->NextEntryOffset == 0) {
                break;
            }
            cur += pi->NextEntryOffset;
        }
#endif
        return processNames;
    }

    // Retrieves the computer name
    [[nodiscard]] static std::string get_hostname() {
#if (WINDOWS)
        char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD cbComputerName = sizeof(ComputerName);

        if (GetComputerNameA(ComputerName, &cbComputerName)) {
            return std::string(ComputerName);
        }
#elif (LINUX)
        char hostname[HOST_NAME_MAX];

        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
#endif

        return std::string();
    }


    [[nodiscard]] static bool is_running_under_translator() {
        #if (WINDOWS)
        u8 ver = get_windows_version();
        if (ver == 10 || ver == 11) {
            USHORT procMachine = 0, nativeMachine = 0;
            if (IsWow64Process2(GetCurrentProcess(), &procMachine, &nativeMachine)) {
                if (nativeMachine == IMAGE_FILE_MACHINE_ARM64 &&
                    (procMachine == IMAGE_FILE_MACHINE_AMD64 ||
                        procMachine == IMAGE_FILE_MACHINE_I386))
                {
                    return true;
                }
            }
        }
#endif

        if (cpu::is_leaf_supported(cpu::leaf::hypervisor)) {
            std::string vendor = cpu::cpu_manufacturer(cpu::leaf::hypervisor);
            if (vendor == "VirtualApple" ||   // Apple Rosetta
                vendor == "PowerVM Lx86")     // IBM PowerVM Lx86
            {
                return true;
            }
        }

        return false;
    }


    /**
        * @brief Checks whether the system is running in a Hyper-V virtual machine or if the host system has Hyper-V enabled
        * @note Hyper-V's presence on a host system can set certain hypervisor-related CPU flags that may appear similar to those in a virtualized environment, which can make it challenging to differentiate between an actual Hyper-V virtual machine (VM) and a host system with Hyper-V enabled.
        *       This can lead to false conclusions, where the system might mistakenly be identified as running in a Hyper-V VM, when in reality, it's simply the host system with Hyper-V features active.
        *       This check aims to distinguish between these two cases by identifying specific CPU flags and hypervisor-related artifacts that are indicative of a Hyper-V VM rather than a host system with Hyper-V enabled.
        * @author Requiem (https://github.com/NotRequiem)
        * @returns hyperx_state enum indicating the detected state:
        *          - HYPERV_ARTIFACT_VM for host with Hyper-V enabled
        *          - HYPERV_REAL_VM for real Hyper-V VM
        *          - HYPERV_ENLIGHTENMENT for QEMU with Hyper-V enlightenments
        *          - HYPERV_UNKNOWN_VM for unknown/undetected state
        */
    [[nodiscard]] static hyperx_state hyper_x() {
#if (!WINDOWS)
        return HYPERV_UNKNOWN_VM;
#else
        if (memo::hyperx::is_cached()) {
            core_debug("HYPER_X: returned from cache");
            return memo::hyperx::fetch();
        }

        // check if hypervisor feature bit in CPUID eax bit 31 is enabled (always false for physical CPUs)
        auto is_hyperv_present = []() -> bool {
            u32 unused, ecx = 0;
            cpu::cpuid(unused, unused, ecx, unused, 1);

            return (ecx & (1 << 31));
            };

        // 0x40000003 on EBX indicates the flags that a parent partition specified to create a child partition (https://learn.microsoft.com/en-us/virtualization/hyper-v-on-windows/tlfs/datatypes/hv_partition_privilege_mask)
        auto is_root_partition = []() -> bool {
            u32 ebx, unused = 0;
            cpu::cpuid(unused, ebx, unused, unused, 0x40000003);
            const bool result = (ebx & 1);

#ifdef __VMAWARE_DEBUG__
            if (result) {
                core_debug("HYPER_X: running under root partition");
            }
#endif
            return result;
            };

        /**
            * On Hyper-V virtual machines, the cpuid function reports an EAX value of 11
            * This value is tied to the Hyper-V partition model, where each virtual machine runs as a child partition
            * These child partitions have limited privileges and access to hypervisor resources, 
            * which is reflected in the maximum input value for hypervisor CPUID information as 11.
            * Essentially, it indicates that the hypervisor is managing the VM and that the VM is not running directly on hardware but rather in a virtualized environment
        */
        auto eax = []() -> u32 {
            char out[sizeof(i32) * 4 + 1] = { 0 };
            cpu::cpuid((int*)out, cpu::leaf::hypervisor);

            const u32 eax = static_cast<u32>(out[0]);

            return eax;
            };

        hyperx_state state;

        if (!is_root_partition()) {
            if (eax() == 11 && is_hyperv_present()) {
                // Windows machine running under Hyper-V type 2
                core_debug("HYPER_X: added Hyper-V real VM");
                core::add(brands::HYPERV);
                state = HYPERV_REAL_VM;
            }
            else {
                core_debug("HYPER_X: none found");
                state = HYPERV_UNKNOWN_VM;
            }
        }
        else {
            // normally eax 12
            const std::string brand_str = cpu::cpu_manufacturer(0x40000001);

            if (util::find(brand_str, "KVM")) {
                core_debug("HYPER_X: added Hyper-V Enlightenments");
                core::add(brands::QEMU_KVM_HYPERV);
                state = HYPERV_ENLIGHTENMENT;
            }
            else {
                // Windows machine running under Hyper-V type 1
                core_debug("HYPER_X: added Hyper-V artifact VM");
                core::add(brands::HYPERV_ARTIFACT);
                state = HYPERV_ARTIFACT_VM;
            }
        }

        memo::hyperx::store(state);
        core_debug("HYPER_X: cached");

        return state;
#endif
    }

#if (WINDOWS)
    /**
        * @link: https://codereview.stackexchange.com/questions/249034/systeminfo-a-c-class-to-retrieve-system-management-data-from-the-bios
        * @author: arcomber
        */
    class sys_info {
    private:
#pragma pack(push) 
#pragma pack(1)
        /*
        SMBIOS Structure header (System Management BIOS) spec:
        https ://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
        */
        struct SMBIOSHEADER
        {
            u8 type;
            u8 length;
            u16 handle;
        };

        /*
        Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
        see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
        */
        struct SMBIOSData {
            u8  Used20CallingMethod;
            u8  SMBIOSMajorVersion;
            u8  SMBIOSMinorVersion;
            u8  DmiRevision;
            u32  Length;
            u8  SMBIOSTableData[1];
        };

        // System Information (Type 1)
        struct SYSTEMINFORMATION {
            SMBIOSHEADER Header;
            u8 Manufacturer;
            u8 ProductName;
            u8 Version;
            u8 SerialNumber;
            u8 UUID[16];
            u8 WakeUpType;  // Identifies the event that caused the system to power up
            u8 SKUNumber;   // identifies a particular computer configuration for sale
            u8 Family;
        };
#pragma pack(pop) 

        // helper to retrieve string at string offset. Optional null string description can be set.
        const char* get_string_by_index(const char* str, int index, const char* null_string_text = "")
        {
            if (0 == index || 0 == *str) {
                return null_string_text;
            }

            while (--index) {
                str += strlen(str) + 1;
            }
            return str;
        }

        // retrieve the BIOS data block from the system
        SMBIOSData* get_bios_data() {
            SMBIOSData* bios_data = nullptr;

            // GetSystemFirmwareTable with arg RSMB retrieves raw SMBIOS firmware table
            // return value is either size of BIOS table or zero if function fails
            DWORD bios_size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);

            if (bios_size > 0) {
                if (bios_data != nullptr) {
                    bios_data = (SMBIOSData*)malloc(bios_size);

                    // Retrieve the SMBIOS table
                    DWORD bytes_retrieved = GetSystemFirmwareTable('RSMB', 0, bios_data, bios_size);

                    if (bytes_retrieved != bios_size) {
                        free(bios_data);
                        bios_data = nullptr;
                    }
                }
            }

            return bios_data;
        }


        // locates system information memory block in BIOS table
        SYSTEMINFORMATION* find_system_information(SMBIOSData* bios_data) {
            u8* data = bios_data->SMBIOSTableData;

            while (data < bios_data->SMBIOSTableData + bios_data->Length)
            {
                u8* next;
                SMBIOSHEADER* header = (SMBIOSHEADER*)data;

                if (header->length < 4)
                    break;

                //Search for System Information structure with type 0x01 (see para 7.2)
                if (header->type == 0x01 && header->length >= 0x19)
                {
                    return (SYSTEMINFORMATION*)header;
                }

                //skip over formatted area
                next = data + header->length;

                //skip over unformatted area of the structure (marker is 0000h)
                while (next < bios_data->SMBIOSTableData + bios_data->Length && (next[0] != 0 || next[1] != 0)) {
                    next++;
                }
                next += 2;

                data = next;
            }
            return nullptr;
        }

    public:
        // System information data retrieved on construction and string members populated
        sys_info() {
            SMBIOSData* bios_data = get_bios_data();

            if (bios_data) {
                SYSTEMINFORMATION* sysinfo = find_system_information(bios_data);
                if (sysinfo) {
                    const char* str = (const char*)sysinfo + sysinfo->Header.length;

                    manufacturer_ = get_string_by_index(str, sysinfo->Manufacturer);
                    productname_ = get_string_by_index(str, sysinfo->ProductName);
                    serialnumber_ = get_string_by_index(str, sysinfo->SerialNumber);
                    version_ = get_string_by_index(str, sysinfo->Version);

                    // for v2.1 and later
                    if (sysinfo->Header.length > 0x08)
                    {
                        static const int max_uuid_size{ 50 };
                        char uuid[max_uuid_size] = {};
                        _snprintf_s(uuid, max_uuid_size, static_cast<size_t>(max_uuid_size) - 1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                            sysinfo->UUID[0], sysinfo->UUID[1], sysinfo->UUID[2], sysinfo->UUID[3],
                            sysinfo->UUID[4], sysinfo->UUID[5], sysinfo->UUID[6], sysinfo->UUID[7],
                            sysinfo->UUID[8], sysinfo->UUID[9], sysinfo->UUID[10], sysinfo->UUID[11],
                            sysinfo->UUID[12], sysinfo->UUID[13], sysinfo->UUID[14], sysinfo->UUID[15]);

                        uuid_ = uuid;
                    }

                    if (sysinfo->Header.length > 0x19)
                    {
                        // supported in v 2.4 spec
                        sku_ = get_string_by_index(str, sysinfo->SKUNumber);
                        family_ = get_string_by_index(str, sysinfo->Family);
                    }
                }
                free(bios_data);
            }
        }

        // get product family
        const std::string get_family() const {
            return family_;
        }

        // get manufacturer - generally motherboard or system assembler name
        const std::string get_manufacturer() const {
            return manufacturer_;
        }

        // get product name
        const std::string get_productname() const {
            return productname_;
        }

        // get BIOS serial number
        const std::string get_serialnumber() const {
            return serialnumber_;
        }

        // get SKU / system configuration
        const std::string get_sku() const {
            return sku_;
        }

        // get a universally unique identifier for system
        const std::string get_uuid() const {
            return uuid_;
        }

        // get version of system information
        const std::string get_version() const {
            return version_;
        }

        sys_info(sys_info const&) = delete;
        sys_info& operator=(sys_info const&) = delete;

    private:
        std::string family_;
        std::string manufacturer_;
        std::string productname_;
        std::string serialnumber_;
        std::string sku_;
        std::string uuid_;
        std::string version_;
    };


    /**
        * @brief Determines if the current process is running under WOW64.
        *
        * WOW64 (Windows-on-Windows 64-bit) is a subsystem that allows 32-bit
        * applications to run on 64-bit Windows. This function checks whether the
        * current process is a 32-bit application running on a 64-bit OS.
        *
        * @return true if the process is running under WOW64, otherwise false.
        */
    [[nodiscard]] static bool is_wow64() {
        BOOL isWow64 = FALSE;
        BOOL pbool = IsWow64Process(GetCurrentProcess(), &isWow64);
        return (pbool && isWow64);
    }


    /**
        * @brief Retrieves the Windows major version using RtlGetVersion.
        *
        * This function queries the ntdll.dll library to obtain the Windows version.
        * It maps the build number to a major Windows version using a predefined map.
        * If the primary method fails, it falls back to get_windows_version_backup().
        *
        * @return The major version of Windows (e.g., 6 for Vista/7, 10 for Windows 10).
        */
    [[nodiscard]] static u8 get_windows_version() {
        const std::map<DWORD, u8> windowsVersions = {
            { static_cast<DWORD>(6002), static_cast<u8>(6) }, // windows vista
            { static_cast<DWORD>(7601), static_cast<u8>(7) },
            { static_cast<DWORD>(9200), static_cast<u8>(8) },
            { static_cast<DWORD>(9600), static_cast<u8>(8) },
            { static_cast<DWORD>(10240), static_cast<u8>(10) },
            { static_cast<DWORD>(10586), static_cast<u8>(10) },
            { static_cast<DWORD>(14393), static_cast<u8>(10) },
            { static_cast<DWORD>(15063), static_cast<u8>(10) },
            { static_cast<DWORD>(16299), static_cast<u8>(10) },
            { static_cast<DWORD>(17134), static_cast<u8>(10) },
            { static_cast<DWORD>(17763), static_cast<u8>(10) },
            { static_cast<DWORD>(18362), static_cast<u8>(10) },
            { static_cast<DWORD>(18363), static_cast<u8>(10) },
            { static_cast<DWORD>(19041), static_cast<u8>(10) },
            { static_cast<DWORD>(19042), static_cast<u8>(10) },
            { static_cast<DWORD>(19043), static_cast<u8>(10) },
            { static_cast<DWORD>(19044), static_cast<u8>(10) },
            { static_cast<DWORD>(19045), static_cast<u8>(10) },
            { static_cast<DWORD>(22000), static_cast<u8>(11) },
            { static_cast<DWORD>(22621), static_cast<u8>(11) },
            { static_cast<DWORD>(22631), static_cast<u8>(11) },
            { static_cast<DWORD>(26100), static_cast<u8>(11) }
        };

        const HMODULE ntdll = GetModuleHandle(_T("ntdll.dll"));
        if (!ntdll) {
            return 0;
        }

        typedef NTSTATUS(__stdcall* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
#pragma warning (disable : 4191)
        RtlGetVersionFunc pRtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(GetProcAddress(ntdll, "RtlGetVersion"));
#pragma warning (default : 4191)
        if (!pRtlGetVersion) {
            return 0;
        }

        RTL_OSVERSIONINFOW osvi{};
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        if (pRtlGetVersion(&osvi) != 0) {
            return 0;
        }

        u8 major_version = 0;

        if (windowsVersions.find(osvi.dwBuildNumber) != windowsVersions.end()) {
            major_version = windowsVersions.at(osvi.dwBuildNumber);
        }

        return major_version;
    }


    /**
        * @brief Retrieves the last error message from the Windows API. Useful for __VMAWARE_DEBUG__
        * @return A std::wstring containing the error message.
        */
    [[nodiscard]] static std::wstring GetLastErrorString() {
        const DWORD error = GetLastError();
        LPWSTR messageBuffer = nullptr;
        size_t size = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, 0, (LPWSTR)&messageBuffer, 0, nullptr
        );

        std::wstring message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }

    /**
        * @brief Retrieves the addresses of specified functions from a loaded module using the export directory.
        *
        * This function dynamically resolves the addresses of specified functions in a given module by accessing
        * the export directory of the module. It searches for functions by their names and populates an array of
        * function pointers with the resolved addresses.
        *
        *
        * @param hModule Handle to the loaded module (DLL or EXE) in which to resolve the function addresses.
        * @param names An array of function names (strings) to be resolved in the module.
        * @param functions An array of function pointers where the resolved function addresses will be stored.
        * @param count The number of functions to resolve.
        *
        * @return bool true if all requested function addresses were successfully resolved, false otherwise.
        */
    static void GetFunctionAddresses(const HMODULE hModule, const char* names[], void** functions, size_t count) {
        const PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
        const PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<BYTE*>(hModule) + dosHeader->e_lfanew);
        const PIMAGE_EXPORT_DIRECTORY exportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
            reinterpret_cast<BYTE*>(hModule) + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

        const DWORD* nameOffsets = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(hModule) + exportDirectory->AddressOfNames);
        const DWORD* funcOffsets = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(hModule) + exportDirectory->AddressOfFunctions);
        const WORD* ordinals = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(hModule) + exportDirectory->AddressOfNameOrdinals);

        size_t resolved = 0;
        for (DWORD i = 0; i < exportDirectory->NumberOfNames && resolved < count; ++i) {
            const char* exportName = reinterpret_cast<const char*>(reinterpret_cast<BYTE*>(hModule) + nameOffsets[i]);
            for (size_t j = 0; j < count; ++j) {
                if (functions[j] == nullptr && strcmp(exportName, names[j]) == 0) {
                    functions[j] = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(hModule) + funcOffsets[ordinals[i]]);
                    ++resolved;
                }
            }
        }
    }
#endif
};