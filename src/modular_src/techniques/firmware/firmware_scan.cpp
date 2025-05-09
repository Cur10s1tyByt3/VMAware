#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Checks for VM signatures in firmware
 * @category Windows
 * @note Idea of detecting VMwareHardenerLoader was made by MegaMax, detection for Bochs, VirtualBox and WAET was made by dmfrpro
 * @credits MegaMax, dmfrpro
 * @implements VM::FIRMWARE
 */
[[nodiscard]] static bool firmware_scan() {
#if (WINDOWS)
#pragma warning (disable: 4459)
        typedef enum _SYSTEM_INFORMATION_CLASS {
            SystemFirmwareTableInformation = 76
        } SYSTEM_INFORMATION_CLASS;

        typedef struct _SYSTEM_FIRMWARE_TABLE_INFORMATION {
            ULONG ProviderSignature;
            ULONG Action;
            ULONG TableID;
            ULONG TableBufferLength;
            UCHAR TableBuffer[1];
        } SYSTEM_FIRMWARE_TABLE_INFORMATION, * PSYSTEM_FIRMWARE_TABLE_INFORMATION;
#pragma warning (default : 4459)

        typedef NTSTATUS(__stdcall* PNtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
        constexpr ULONG STATUS_BUFFER_TOO_SMALL = 0xC0000023;
        constexpr DWORD ACPI_SIG = 'ACPI';
        constexpr DWORD RSMB_SIG = 'RSMB';
        constexpr DWORD FIRM_SIG = 'FIRM';

        const HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
        if (!hNtdll) return false;

        const char* functionNames[] = { "NtQuerySystemInformation" };
        void* functionPointers[1] = { nullptr };

        util::GetFunctionAddresses(hNtdll, functionNames, functionPointers, 1);

        const auto ntqsi = reinterpret_cast<PNtQuerySystemInformation>(functionPointers[0]);
        if (!ntqsi)
            return false;

        constexpr const char* targets[] = {
            "Parallels Software International", "Parallels(R)", "innotek",
            "Oracle", "VirtualBox", "vbox", "VBOX", "VS2005R2", "VMware, Inc.",
            "VMware", "VMWARE", "S3 Corp.", "Virtual Machine", "QEMU", "WAET",
            "BOCHS", "BXPC"
        };

        PBYTE qsiBuffer = nullptr;
        ULONG qsiBufferSize = 0;

        auto ensure_buffer = [&](ULONG needed) -> bool {
            if (qsiBufferSize < needed) {
                free(qsiBuffer);
                qsiBuffer = static_cast<PBYTE>(malloc(needed));
                if (!qsiBuffer) {
                    qsiBufferSize = 0;
                    return false;
                }
                qsiBufferSize = needed;
            }
            return true;
            };

        auto query_table = [&](DWORD provider, ULONG tableID, bool rawEnum, PULONG outDataSize) -> PSYSTEM_FIRMWARE_TABLE_INFORMATION {
            // header-only to get size
            const ULONG header = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);
            if (!ensure_buffer(header)) return nullptr;

            auto hdr = reinterpret_cast<PSYSTEM_FIRMWARE_TABLE_INFORMATION>(qsiBuffer);
            hdr->ProviderSignature = provider;
            hdr->Action = rawEnum ? 0 : 1;
            hdr->TableID = tableID;
            hdr->TableBufferLength = 0;

            NTSTATUS st = ntqsi(SystemFirmwareTableInformation, hdr, header, outDataSize);
            if (st != STATUS_BUFFER_TOO_SMALL)
                return nullptr;

            ULONG fullSize = *outDataSize;
            if (!ensure_buffer(fullSize)) return nullptr;

            hdr = reinterpret_cast<PSYSTEM_FIRMWARE_TABLE_INFORMATION>(qsiBuffer);
            hdr->ProviderSignature = provider;
            hdr->Action = rawEnum ? 0 : 1;
            hdr->TableID = tableID;
            hdr->TableBufferLength = fullSize - header;

            st = ntqsi(SystemFirmwareTableInformation, hdr, fullSize, outDataSize);
            if (!NT_SUCCESS(st))
                return nullptr;

            return hdr;
            };

        auto check_firmware_table = [&](DWORD signature, ULONG tableID) -> bool {
            ULONG gotSize = 0;
            auto info = query_table(signature, tableID, false, &gotSize);
            if (!info) return false;

            const UCHAR* buf = info->TableBuffer;
            const size_t bufLen = info->TableBufferLength;

            for (auto target : targets) {
                size_t tlen = strlen(target);
                if (tlen > bufLen) continue;
                for (size_t i = 0; i <= bufLen - tlen; ++i) {
                    if (memcmp(buf + i, target, tlen) == 0) {
                        const char* brand = nullptr;
                        if (!strcmp(target, "Parallels Software International") || !strcmp(target, "Parallels(R)"))
                            brand = brands::PARALLELS;
                        else if (!strcmp(target, "innotek") || !strcmp(target, "VirtualBox") || !strcmp(target, "vbox") || !strcmp(target, "VBOX") || !strcmp(target, "Oracle"))
                            brand = brands::VBOX;
                        else if (!strcmp(target, "VMware, Inc.") || !strcmp(target, "VMware") || !strcmp(target, "VMWARE"))
                            brand = brands::VMWARE;
                        else if (!strcmp(target, "QEMU"))
                            brand = brands::QEMU;
                        else if (!strcmp(target, "BOCHS") || !strcmp(target, "BXPC"))
                            brand = brands::BOCHS;
                        else
                            return true;

                        return core::add(brand);
                    }
                }
            }

            // to detect VMAware's Hardener Loader, idea by MegaMax
            if (bufLen >= 6) {
                for (size_t i = 0; i <= bufLen - 6; ++i) {
                    if (buf[i] == '7' && buf[i + 1] == '7' && buf[i + 2] == '7' && buf[i + 3] == '7' && buf[i + 4] == '7' && buf[i + 5] == '7') {
                        return core::add(brands::VMWARE_HARD);
                    }
                }
            }
            return false;
            };

        // RSMB table
        if (check_firmware_table(RSMB_SIG, 0UL)) {
            free(qsiBuffer);
            return true;
        }

        // FIRM tables
        for (ULONG addr : { 0xC0000UL, 0xE0000UL }) {
            if (check_firmware_table(FIRM_SIG, addr)) {
                free(qsiBuffer);
                return true;
            }
        }

        // ACPI enumeration
        ULONG totalLen = 0;
        auto listInfo = query_table(ACPI_SIG, 0UL, true, &totalLen);
        if (!listInfo) {
            free(qsiBuffer);
            return false;
        }

        const DWORD* tables = reinterpret_cast<const DWORD*>(listInfo->TableBuffer);
        ULONG tableCount = listInfo->TableBufferLength / sizeof(DWORD);

        if (tableCount < 4) { // idea by dmfrpro
            free(qsiBuffer);
            return true;
        }

        // count SSDT
        const DWORD ssdtSig = 'TDSS';
        ULONG ssdtCount = 0;
        for (ULONG i = 0; i < tableCount; ++i) {
            if (tables[i] == ssdtSig)
                ++ssdtCount;
            if (ssdtCount == 2)
                break;
        }
        if (ssdtCount < 2) {
            free(qsiBuffer);
            return true;
        }

        // iterate all ACPI tables
        constexpr DWORD dsdtSig = 'TDSD';
        constexpr DWORD facpSig = 'PCAF';
        for (ULONG i = 0; i < tableCount; ++i) {
            DWORD sig = tables[i];
            if (sig == facpSig) {
                ULONG fSize = 0;
                auto fadt = query_table(ACPI_SIG, sig, false, &fSize);
                if (!fadt) continue;
                BYTE* buf = reinterpret_cast<BYTE*>(fadt->TableBuffer);
                // https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#preferred-pm-profile-system-types
                if (fSize >= 45 + 1 && buf[45] == 0) { // idea by dmfrpro
                    free(qsiBuffer);
                    return true;
                }
            }
            if (sig == dsdtSig) {
                ULONG dsdtSize = 0;
                auto dsdt = query_table(ACPI_SIG, sig, false, &dsdtSize);
                if (dsdt) {
                    const char* tb = reinterpret_cast<const char*>(dsdt->TableBuffer);
                    bool foundCPU = false;

                    for (ULONG j = 0; j + 8 <= dsdtSize; ++j) {
                        if (memcmp(tb + j, "ACPI0007", 8) == 0) { // idea by dmfrpro
                            foundCPU = true;
                            break;
                        }
                    }

                    if (!foundCPU) {
                        free(qsiBuffer);
                        return true;
                    }

                    constexpr const char* osi_targets[] = {
                        "Windows 95",            "Windows 98",
                        "Windows 2000",          "Windows 2000.1",
                        "Windows ME: Millennium Edition",
                        "Windows ME: Millennium Edition",  // some firmwares omit space
                        "Windows XP",            "Windows 2001",
                        "Windows 2006",          "Windows 2009",
                        "Windows 2012",          "Windows 2015",
                        "Windows 2020",          "Windows 2022",

                    };
                    constexpr size_t n_osi = sizeof(osi_targets) / sizeof(osi_targets[0]);

                    bool foundOSI = false;
                    for (size_t t = 0; t < n_osi && !foundOSI; ++t) {
                        const char* s = osi_targets[t];
                        size_t len = strlen(s);
                        for (ULONG j = 0; j + len <= dsdtSize; ++j) {
                            if (memcmp(tb + j, s, len) == 0) {
                                foundOSI = true;
                                break;
                            }
                        }
                    }

                    if (!foundOSI) {
                        free(qsiBuffer);
                        return true;
                    }
                }
            }
            if (check_firmware_table(ACPI_SIG, sig)) {
                free(qsiBuffer);
                return true;
            }
        }

        free(qsiBuffer);

        std::unique_ptr<util::sys_info> info = util::make_unique<util::sys_info>();

        const std::string str = info->get_serialnumber();

        if (util::find(str, "VMW")) {
            return core::add(brands::VMWARE_FUSION);
        }

        const std::size_t nl_pos = str.find('\n');

        if (nl_pos == std::string::npos) {
            return false;
        }

        debug("BIOS_SERIAL: ", str);

        const std::string extract = str.substr(nl_pos + 1);

        const bool all_digits = std::all_of(extract.cbegin(), extract.cend(), [](const char c) {
            return std::isdigit(c);
            });

        if (all_digits) {
            if (extract == "0") {
                return true;
            }
        }

        return false;
#elif (LINUX)
        // Author: dmfrpro
        if (!util::is_admin()) {
            return false;
        }

        DIR* dir = opendir("/sys/firmware/acpi/tables/");
        if (!dir) {
            debug("FIRMWARE: could not open ACPI tables directory");
            return false;
        }

        // Same targets as the Windows branch but without "WAET"
        constexpr const char* targets[] = {
            "Parallels Software International", "Parallels(R)", "innotek",
            "Oracle", "VirtualBox", "vbox", "VBOX", "VS2005R2", "VMware, Inc.",
            "VMware", "VMWARE", "S3 Corp.", "Virtual Machine", "QEMU", "BOCHS",
            "BXPC"
        };

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Skip "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char path[PATH_MAX];
            snprintf(path, sizeof(path), "/sys/firmware/acpi/tables/%s", entry->d_name);

            int fd = open(path, O_RDONLY);
            if (fd == -1) {
                debug("FIRMWARE: could not open ACPI table ", entry->d_name);
                continue;
            }

            struct stat statbuf;
            if (fstat(fd, &statbuf) != 0) {
                debug("FIRMWARE: skipped ", entry->d_name);
                close(fd);
                continue;
            }
            if (S_ISDIR(statbuf.st_mode)) {
                debug("FIRMWARE: skipped directory ", entry->d_name);
                close(fd);
                continue;
            }

            long file_size = statbuf.st_size;
            if (file_size <= 0) {
                debug("FIRMWARE: file empty or error ", entry->d_name);
                close(fd);
                continue;
            }

            char* buffer = static_cast<char*>(malloc(file_size));
            if (!buffer) {
                debug("FIRMWARE: failed to allocate memory for buffer");
                close(fd);
                continue;
            }
            close(fd);

            for (const char* target : targets) {
                size_t targetLen = strlen(target);
                if (targetLen == 0 || file_size < static_cast<long>(targetLen))
                    continue;
                for (long j = 0; j <= file_size - static_cast<long>(targetLen); ++j) {
                    if (memcmp(buffer + j, target, targetLen) == 0) {
                        const char* brand = nullptr;
                        if (strcmp(target, "Parallels Software International") == 0 ||
                            strcmp(target, "Parallels(R)") == 0) {
                            brand = brands::PARALLELS;
                        }
                        else if (strcmp(target, "innotek") == 0 ||
                            strcmp(target, "Oracle") == 0 ||
                            strcmp(target, "VirtualBox") == 0 ||
                            strcmp(target, "vbox") == 0 ||
                            strcmp(target, "VBOX") == 0) {
                            brand = brands::VBOX;
                        }
                        else if (strcmp(target, "VMware, Inc.") == 0 ||
                            strcmp(target, "VMware") == 0 ||
                            strcmp(target, "VMWARE") == 0) {
                            brand = brands::VMWARE;
                        }
                        else if (strcmp(target, "QEMU")) {
                            brand = brands::QEMU;
                        }
                        else if (strcmp(target, "BOCHS") == 0 ||
                            strcmp(target, "BXPC") == 0) {
                            brand = brands::BOCHS;
                        }
                        free(buffer);
                        closedir(dir);
                        if (brand)
                            return core::add(brand);
                        else
                            return true;
                    }
                }
            }
            free(buffer);
        }

        closedir(dir);
        return false;
#else
        return false;
#endif
    }
