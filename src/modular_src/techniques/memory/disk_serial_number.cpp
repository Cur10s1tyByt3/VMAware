#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for serial numbers of virtual disks
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @note VMware can't be flagged without also flagging legitimate devices
 * @implements VM::DISK_SERIAL
 */
[[nodiscard]] static bool disk_serial_number() {
#if (!WINDOWS)
    return false;
#else
    bool result = false;
    constexpr DWORD MAX_PHYSICAL_DRIVES = 4;

    auto is_vbox_serial = [](const char* str, size_t len) -> bool {
        // VirtualBox pattern: VB + 8 hex + - + 8 hex
        if (len != 19) return false;
        if ((str[0] != 'V' && str[0] != 'v') ||
            (str[1] != 'B' && str[1] != 'b')) return false;

        auto is_hex = [](char c) {
            return (c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'F') ||
                (c >= 'a' && c <= 'f');
            };

        for (int i : {2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18}) {
            if (!is_hex(str[i])) return false;
        }
        return str[10] == '-';
        };

    for (DWORD drive = 0; drive < MAX_PHYSICAL_DRIVES; drive++) {
        wchar_t path[32];
        swprintf_s(path, L"\\\\.\\PhysicalDrive%lu", drive);

        HANDLE hDevice = CreateFileW(path, 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            OPEN_EXISTING, 0, nullptr);

        if (hDevice == INVALID_HANDLE_VALUE) continue;

        STORAGE_PROPERTY_QUERY query{};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        STORAGE_DESCRIPTOR_HEADER header{};
        DWORD bytesReturned = 0;

        if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &query, sizeof(query), &header, sizeof(header),
            &bytesReturned, nullptr) || header.Size == 0) {
            CloseHandle(hDevice);
            continue;
        }

        BYTE stackBuf[512]{};
        BYTE* buffer = nullptr;

        if (header.Size <= sizeof(stackBuf)) {
            buffer = stackBuf;
        }
        else {
            buffer = static_cast<BYTE*>(LocalAlloc(LMEM_FIXED, header.Size));
            if (buffer == nullptr) {
                CloseHandle(hDevice);
                continue;
            }
        }

        if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &query, sizeof(query), buffer, header.Size,
            &bytesReturned, nullptr)) {
            if (buffer != stackBuf) {
                LocalFree(buffer);
            }
            CloseHandle(hDevice);
            continue;
        }

        auto descriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer);
        const DWORD serialOffset = descriptor->SerialNumberOffset;

        if (serialOffset > 0 && serialOffset < header.Size) {
            const char* serial = reinterpret_cast<const char*>(buffer + serialOffset);
            const size_t serialLen = strnlen(serial, header.Size - static_cast<size_t>(serialOffset));

            char upperSerial[256] = { 0 };
            const size_t copyLen = (serialLen < sizeof(upperSerial))
                ? serialLen
                : sizeof(upperSerial) - 1;

            for (size_t i = 0; i < copyLen; i++) {
                char c = serial[i];
                upperSerial[i] = (c >= 'a' && c <= 'z') ? c - 32 : c;
            }
            upperSerial[copyLen] = '\0';

            if (is_vbox_serial(upperSerial, copyLen)) {
                result = core::add(brands::VBOX);
                if (buffer != stackBuf) {
                    LocalFree(buffer);
                }
                CloseHandle(hDevice);
                return result;
            }
        }

        if (buffer != stackBuf) {
            LocalFree(buffer);
        }
        CloseHandle(hDevice);
    }

    return result;
#endif
}