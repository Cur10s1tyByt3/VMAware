#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Checks for particular object directory which is present in Sandboxie virtual environment but not in usual host systems
 * @category Windows
 * @note https://evasions.checkpoint.com/src/Evasions/techniques/global-os-objects.html
 * @implements VM::VIRTUAL_REGISTRY
 */
[[nodiscard]] static bool virtual_registry() {
#if (!WINDOWS)
    return false;
#else
#pragma warning(disable : 4459)
    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, * PUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

    typedef enum _OBJECT_INFORMATION_CLASS {
        ObjectBasicInformation = 0,
        ObjectNameInformation = 1,
        ObjectTypeInformation = 2
    } OBJECT_INFORMATION_CLASS;

    typedef struct _OBJECT_NAME_INFORMATION {
        UNICODE_STRING Name;
    } OBJECT_NAME_INFORMATION, * POBJECT_NAME_INFORMATION;
#pragma warning(default : 4459)

    typedef NTSTATUS(__stdcall* PNtOpenKey)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
    typedef NTSTATUS(__stdcall* PNtQueryObject)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);

    const HMODULE hModule = GetModuleHandle(_T("ntdll.dll"));
    if (!hModule)
        return false;

    const char* functionNames[] = { "NtOpenKey", "NtQueryObject" };
    void* functionPointers[2] = { nullptr, nullptr };

    util::GetFunctionAddresses(hModule, functionNames, functionPointers, 2);

    const auto NtOpenKey = reinterpret_cast<PNtOpenKey>(functionPointers[0]);
    const auto NtQueryObject = reinterpret_cast<PNtQueryObject>(functionPointers[1]);
    if (!NtOpenKey || !NtQueryObject)
        return false;

    UNICODE_STRING keyPath{};
    keyPath.Buffer = const_cast<PWSTR>(L"\\REGISTRY\\USER");
    keyPath.Length = static_cast<USHORT>(wcslen(keyPath.Buffer) * sizeof(WCHAR));
    keyPath.MaximumLength = keyPath.Length + sizeof(WCHAR);

    OBJECT_ATTRIBUTES objAttr = {
        sizeof(OBJECT_ATTRIBUTES),
        nullptr,
        &keyPath,
        0x00000040L,  // OBJ_CASE_INSENSITIVE
        nullptr,
        nullptr
    };

    HANDLE hKey = nullptr;
    NTSTATUS status = NtOpenKey(&hKey, KEY_READ, &objAttr);
    if (!NT_SUCCESS(status))
        return false;

    alignas(16) BYTE buffer[1024]{};
    ULONG returnedLength = 0;
    status = NtQueryObject(hKey, ObjectNameInformation, buffer, sizeof(buffer), &returnedLength);
    CloseHandle(hKey);
    if (!NT_SUCCESS(status))
        return false;

    auto pObjectName = reinterpret_cast<POBJECT_NAME_INFORMATION>(buffer);

    UNICODE_STRING expectedName{};
    expectedName.Buffer = const_cast<PWSTR>(L"\\REGISTRY\\USER");
    expectedName.Length = static_cast<USHORT>(wcslen(expectedName.Buffer) * sizeof(WCHAR));
    expectedName.MaximumLength = expectedName.Length + sizeof(WCHAR);

    const bool mismatch = (pObjectName->Name.Length != expectedName.Length) ||
        (memcmp(pObjectName->Name.Buffer, expectedName.Buffer, expectedName.Length) != 0);

    return mismatch ? core::add(brands::SANDBOXIE) : false;
#endif
}