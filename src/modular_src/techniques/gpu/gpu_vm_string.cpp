#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for specific GPU string signatures related to VMs
 * @category Windows
 * @author Requiem (https://github.com/NotRequiem)
 * @author dmfrpro (https://github.com/dmfrpro) (VDD detection)
 * @note utoshu did this with WMI in a removed technique (VM::GPU_CHIPTYPE)
 * @implements VM::GPU_VM_STRING
 */
[[nodiscard]] static bool gpu_vm_strings() {
#if (!WINDOWS)
    return false;
#else
    #if (CPP >= 17)
        struct VMGpuInfo {
            std::wstring_view name;
            const char* brand;
        };

        static constexpr std::array<VMGpuInfo, 8> vm_gpu_names = { {
            { L"VMware SVGA 3D",                   brands::VMWARE    },
            { L"VirtualBox Graphics Adapter",      brands::VBOX      },
            { L"QXL GPU",                          brands::KVM       },
            { L"VirGL 3D",                         brands::QEMU      },
            { L"Microsoft Hyper-V Video",          brands::HYPERV    },
            { L"Parallels Display Adapter (WDDM)", brands::PARALLELS },
            { L"Bochs Graphics Adapter",           brands::BOCHS     },
            { L"IddSampleDriver Device",           brands::NULL_BRAND}
        } };
    #else
        struct VMGpuInfo {
            const wchar_t* name;
            const char* brand;
        };

        static const VMGpuInfo vm_gpu_names[8] = {
            { L"VMware SVGA 3D",                   brands::VMWARE    },
            { L"VirtualBox Graphics Adapter",      brands::VBOX      },
            { L"QXL GPU",                          brands::KVM       },
            { L"VirGL 3D",                         brands::QEMU      },
            { L"Microsoft Hyper-V Video",          brands::HYPERV    },
            { L"Parallels Display Adapter (WDDM)", brands::PARALLELS },
            { L"Bochs Graphics Adapter",           brands::BOCHS     },
            { L"IddSampleDriver Device",           brands::NULL_BRAND}
        };
    #endif

    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);

    for (DWORD deviceNum = 0; EnumDisplayDevicesW(nullptr, deviceNum, &dd, 0); ++deviceNum) {
        #if (CPP >= 17)
            std::wstring_view devStr{ dd.DeviceString };
            for (auto const& info : vm_gpu_names) {
                if (devStr == info.name) {
                    return core::add(info.brand);
                }
            }
        #else
            for (int i = 0; i < 8; ++i) {
                if (wcscmp(dd.DeviceString, vm_gpu_names[i].name) == 0) {
                    return core::add(vm_gpu_names[i].brand);
                }
            }
        #endif
    }

    return false;
#endif
}