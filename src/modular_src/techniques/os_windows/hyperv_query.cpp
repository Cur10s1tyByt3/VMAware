#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Checks if a call to NtQuerySystemInformation with the 0x9f leaf fills a _SYSTEM_HYPERVISOR_DETAIL_INFORMATION structure
 * @category Windows
 * @implements VM::HYPERV_QUERY
 */
[[nodiscard]] static bool hyperv_query() {
#if (!WINDOWS)
    return false;
#else 
    if (util::hyper_x() == HYPERV_ARTIFACT_VM) {
        return false;
    }

    typedef struct _HV_DETAILS {
        ULONG Data[4];
    } HV_DETAILS, * PHV_DETAILS;

    typedef struct _SYSTEM_HYPERVISOR_DETAIL_INFORMATION {
        HV_DETAILS HvVendorAndMaxFunction;
        HV_DETAILS HypervisorInterface;
        HV_DETAILS HypervisorVersion;
        HV_DETAILS HvFeatures;
        HV_DETAILS HwFeatures;
        HV_DETAILS EnlightenmentInfo;
        HV_DETAILS ImplementationLimits;
    } SYSTEM_HYPERVISOR_DETAIL_INFORMATION, * PSYSTEM_HYPERVISOR_DETAIL_INFORMATION;

    typedef NTSTATUS(__stdcall* FN_NtQuerySystemInformation)(
        SYSTEM_INFORMATION_CLASS SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength
        );

    const HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
    if (!hNtdll) {
        return false;
    }

    const char* functionNames[] = { "NtQuerySystemInformation" };
    void* functions[1] = { nullptr };

    util::GetFunctionAddresses(hNtdll, functionNames, functions, 1);

    FN_NtQuerySystemInformation pNtQuerySystemInformation = reinterpret_cast<FN_NtQuerySystemInformation>(functions[0]);
    if (pNtQuerySystemInformation) {
        SYSTEM_HYPERVISOR_DETAIL_INFORMATION hvInfo = { {} };
        const NTSTATUS status = pNtQuerySystemInformation(static_cast<SYSTEM_INFORMATION_CLASS>(0x9F), &hvInfo, sizeof(hvInfo), nullptr);
        if (status != 0) {
            return false;
        }

        if (hvInfo.HvVendorAndMaxFunction.Data[0] != 0) {
            return true;
        }
    }

    return false;
#endif
}