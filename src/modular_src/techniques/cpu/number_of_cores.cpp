#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for number of physical cores
 * @category Windows
 * @implements VM::NUMBER_OF_CORES
 */
[[nodiscard]] static bool number_of_cores() {
#if (!WINDOWS)
    return false;
#else
    DWORD size = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &size);

    std::vector<BYTE> buffer(size);
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &size)) {
        return false;
    }

    int physicalCoreCount = 0;
    auto* ptr = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());

    while (size > 0) {
        if (ptr->Relationship == RelationProcessorCore) {
            ++physicalCoreCount;
            if (physicalCoreCount > 1)
                return false;
        }
        size -= ptr->Size;
        ptr = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(reinterpret_cast<BYTE*>(ptr) + ptr->Size);
    }

    return true;
#endif
}