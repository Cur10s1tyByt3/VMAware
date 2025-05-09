#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for VM-specific names for drivers
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::DRIVER_NAMES
 */
[[nodiscard]] static bool driver_names() {
#if (!WINDOWS)
    return false;
#else
    typedef struct _SYSTEM_MODULE_INFORMATION {
        PVOID  Reserved[2];
        PVOID  ImageBaseAddress;
        ULONG  ImageSize;
        ULONG  Flags;
        USHORT Index;
        USHORT NameLength;
        USHORT LoadCount;
        USHORT PathLength;
        CHAR   ImageName[256];
    } SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

    typedef struct _SYSTEM_MODULE_INFORMATION_EX {
        ULONG  NumberOfModules;
        SYSTEM_MODULE_INFORMATION Module[1];
    } SYSTEM_MODULE_INFORMATION_EX, * PSYSTEM_MODULE_INFORMATION_EX;

    typedef NTSTATUS(__stdcall* NtQuerySystemInformationFn)(
        ULONG SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength
        );

    typedef NTSTATUS(__stdcall* NtAllocateVirtualMemoryFn)(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
        );

    typedef NTSTATUS(__stdcall* NtFreeVirtualMemoryFn)(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG FreeType
        );

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

    constexpr ULONG SystemModuleInformation = 11;
    const HMODULE hModule = GetModuleHandle(_T("ntdll.dll"));
    if (!hModule) return false;

    const char* functionNames[] = { "NtQuerySystemInformation", "NtAllocateVirtualMemory", "NtFreeVirtualMemory" };
    void* functionPointers[3] = { nullptr, nullptr, nullptr };

    util::GetFunctionAddresses(hModule, functionNames, functionPointers, 3);

    const auto ntQuerySystemInformation = reinterpret_cast<NtQuerySystemInformationFn>(functionPointers[0]);
    const auto ntAllocateVirtualMemory = reinterpret_cast<NtAllocateVirtualMemoryFn>(functionPointers[1]);
    const auto ntFreeVirtualMemory = reinterpret_cast<NtFreeVirtualMemoryFn>(functionPointers[2]);

    if (ntQuerySystemInformation == nullptr || ntAllocateVirtualMemory == nullptr || ntFreeVirtualMemory == nullptr)
        return false;
    
    ULONG ulSize = 0;
    NTSTATUS status = ntQuerySystemInformation(SystemModuleInformation, nullptr, 0, &ulSize);
    if (status != STATUS_INFO_LENGTH_MISMATCH) return false;

    const HANDLE hProcess = GetCurrentProcess();
    PVOID allocatedMemory = nullptr;
    SIZE_T regionSize = ulSize;
    ntAllocateVirtualMemory(hProcess, &allocatedMemory, 0, &regionSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    auto pSystemModuleInfoEx = reinterpret_cast<PSYSTEM_MODULE_INFORMATION_EX>(allocatedMemory);
    status = ntQuerySystemInformation(SystemModuleInformation, pSystemModuleInfoEx, ulSize, &ulSize);
    if (!NT_SUCCESS(status)) {
        ntFreeVirtualMemory(hProcess, &allocatedMemory, &regionSize, MEM_RELEASE);
        return false;
    }

    for (ULONG i = 0; i < pSystemModuleInfoEx->NumberOfModules; ++i) {
        const char* driverPath = reinterpret_cast<const char*>(pSystemModuleInfoEx->Module[i].ImageName);
        if (
            strstr(driverPath, "VBoxGuest") ||
            strstr(driverPath, "VBoxMouse") ||
            strstr(driverPath, "VBoxSF")
            ) {
            debug("DRIVER_NAMES: Detected VBox driver: ", driverPath);
            ntFreeVirtualMemory(hProcess, &allocatedMemory, &regionSize, MEM_RELEASE);
            return core::add(brands::VBOX);
        }

        if (
            strstr(driverPath, "vmusbmouse") ||
            strstr(driverPath, "vmmouse") ||
            strstr(driverPath, "vmmemctl")
            ) {
            debug("DRIVER_NAMES: Detected VMware driver: ", driverPath);
            ntFreeVirtualMemory(hProcess, &allocatedMemory, &regionSize, MEM_RELEASE);
            return core::add(brands::VMWARE);
        }
    }

    ntFreeVirtualMemory(hProcess, &allocatedMemory, &regionSize, MEM_RELEASE);
    return false;
#endif
}