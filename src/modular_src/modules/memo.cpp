//#include "../enums.hpp"
//#include "memo.hpp"
//#include <map>
//#include <vector>
//#include <string>


#include "memo.hpp"
#include "../types.hpp"
#include "../hyper_x.hpp"
#include "../globals.hpp"
#include <map>


// initial definitions for cache items because C++ forbids in-class initializations
std::map<u16, memo::data_t> memo::cache_table;
flagset memo::cache_keys = 0;
std::string memo::brand::brand_cache = "";
std::string memo::multi_brand::brand_cache = "";
std::string memo::cpu_brand::brand_cache = "";
hyperx_state memo::hyperx::state = HYPERV_UNKNOWN_VM;
bool memo::hyperx::cached = false;

void memo::cache_store(const u16 technique_macro, const bool result, const points_t points) {
    cache_table[technique_macro] = { result, points };
    cache_keys.set(technique_macro);
}


bool memo::is_cached(const u16 technique_macro) {
    return cache_keys.test(technique_macro);
}

memo::data_t memo::cache_fetch(const u16 technique_macro) {
    return cache_table.at(technique_macro);
}

std::vector<u16> memo::cache_fetch_all() {
    std::vector<u16> vec;

    for (auto it = cache_table.cbegin(); it != cache_table.cend(); ++it) {
        const data_t data = it->second;

        if (data.result == true) {
            const u16 macro = it->first;
            vec.push_back(macro);
        }
    }

    return vec;
}

// basically checks whether all the techniques were cached (with exception of techniques disabled by default)
bool memo::all_present() {
    if (cache_table.size() == globals::technique_count) {
        return true;
    } else if (cache_table.size() == static_cast<std::size_t>(globals::technique_count) - 3) {
        return (
            !cache_keys.test(VMWARE_DMESG) && 
            !cache_keys.test(PORT_CONNECTORS) && 
            !cache_keys.test(ACPI_TEMPERATURE) && 
            !cache_keys.test(LSHW_QEMU)
        );
    }

    return false;
}