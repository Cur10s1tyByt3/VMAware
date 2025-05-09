#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for GPU capabilities related to VMs
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::GPU_CAPABILITIES
 */
[[nodiscard]] static bool gpu_capabilities() {
#if (!WINDOWS)
    return false;
#else
    static Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    if (!factory) {
        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
            debug("GPU_CAPABILITIES: failed to create DXGIFactory2");
            return false;
        }
    }

    static HMODULE warpMod = LoadLibrary(_T("dxgi.dll"));
    if (warpMod) {
        static Microsoft::WRL::ComPtr<IDXGIAdapter> swAdapter;
        if (SUCCEEDED(factory->CreateSoftwareAdapter(warpMod, &swAdapter))) {
            Microsoft::WRL::ComPtr<IDXGIAdapter1> swAdapter1;
            if (SUCCEEDED(swAdapter.As(&swAdapter1))) {
                DXGI_ADAPTER_DESC1 swDesc = {};
                if (SUCCEEDED(swAdapter1->GetDesc1(&swDesc)) &&
                    (swDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
                    debug("GPU_CAPABILITIES: detected software (WARP) adapter");
                    return true;
                }
            }
        }
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> hwAdapter1;
    if (FAILED(factory->EnumAdapters1(0, &hwAdapter1))) {
        debug("GPU_CAPABILITIES: no adapters found");
        return false;
    }

    DXGI_ADAPTER_DESC1 hwDesc = {};
    if (FAILED(hwAdapter1->GetDesc1(&hwDesc))) {
        debug("GPU_CAPABILITIES: failed to get adapter desc");
        return false;
    }

    if (hwDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        debug("GPU_CAPABILITIES: primary adapter is software");
        return true;
    }

    return false;
#endif
}