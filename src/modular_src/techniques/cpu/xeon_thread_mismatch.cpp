#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Same as above, but for Xeon Intel CPUs
 * @category x86
 * @link https://en.wikipedia.org/wiki/List_of_Intel_Core_processors
 * @implements VM::XEON_THREAD_MISMATCH
 */
[[nodiscard]] static bool xeon_thread_mismatch() {
#if (!x86)
    return false;
#else
    if (!cpu::is_intel()) {
        return false;
    }

    if (cpu::has_hyperthreading()) {
        return false;
    }

    const cpu::model_struct model = cpu::get_model();

    if (!model.found) {
        return false;
    }

    if (!model.is_xeon) {
        return false;
    }

    debug("XEON_THREAD_MISMATCH: CPU model = ", model.string);

    struct CStrComparator {
        bool operator()(const char* a, const char* b) const {
            return std::strcmp(a, b) < 0;
        }
    };

    std::map<const char*, int, CStrComparator> thread_database = {
        // Xeon D
        { "D-1518", 8 },
        { "D-1520", 8 },
        { "D-1521", 8 },
        { "D-1527", 8 },
        { "D-1528", 12 },
        { "D-1529", 8 },
        { "D-1531", 12 },
        { "D-1537", 16 },
        { "D-1539", 16 },
        { "D-1540", 16 },
        { "D-1541", 16 },
        { "D-1548", 16 },
        { "D-1557", 24 },
        { "D-1559", 24 },
        { "D-1567", 24 },
        { "D-1571", 32 },
        { "D-1577", 32 },
        { "D-1581", 32 },
        { "D-1587", 32 },
        { "D-1513N", 8 },
        { "D-1523N", 8 },
        { "D-1533N", 12 },
        { "D-1543N", 16 },
        { "D-1553N", 16 },
        { "D-1602", 4 },
        { "D-1612", 8 },
        { "D-1622", 8 },
        { "D-1627", 8 },
        { "D-1632", 16 },
        { "D-1637", 12 },
        { "D-1623N", 8 },
        { "D-1633N", 12 },
        { "D-1649N", 16 },
        { "D-1653N", 16 },
        { "D-2141I", 16 },
        { "D-2161I", 24 },
        { "D-2191", 36 },
        { "D-2123IT", 8 },
        { "D-2142IT", 16 },
        { "D-2143IT", 16 },
        { "D-2163IT", 24 },
        { "D-2173IT", 28 },
        { "D-2183IT", 32 },
        { "D-2145NT", 16 },
        { "D-2146NT", 16 },
        { "D-2166NT", 24 },
        { "D-2177NT", 28 },
        { "D-2187NT", 32 },

        // Xeon E
        { "E-2104G", 4 },
        { "E-2124", 4 },
        { "E-2124G", 4 },
        { "E-2126G", 6 },
        { "E-2134", 8 },
        { "E-2136", 12 },
        { "E-2144G", 8 },
        { "E-2146G", 12 },
        { "E-2174G", 8 },
        { "E-2176G", 12 },
        { "E-2186G", 12 },
        { "E-2176M", 12 },
        { "E-2186M", 12 },
        { "E-2224", 4 },
        { "E-2224G", 4 },
        { "E-2226G", 6 },
        { "E-2234", 8 },
        { "E-2236", 12 },
        { "E-2244G", 8 },
        { "E-2246G", 12 },
        { "E-2274G", 8 },
        { "E-2276G", 12 },
        { "E-2278G", 16 },
        { "E-2286G", 12 },
        { "E-2288G", 16 },
        { "E-2276M", 12 },
        { "E-2286M", 16 },

        // Xeon W
        { "W-2102", 4 },
        { "W-2104", 4 },
        { "W-2123", 8 },
        { "W-2125", 8 },
        { "W-2133", 12 },
        { "W-2135", 12 },
        { "W-2140B", 16 },
        { "W-2145", 16 },
        { "W-2150B", 20 },
        { "W-2155", 20 },
        { "W-2170B", 28 },
        { "W-2175", 28 },
        { "W-2191B", 36 },
        { "W-2195", 36 },
        { "W-3175X", 56 },
        { "W-3223", 16 },
        { "W-3225", 16 },
        { "W-3235", 24 },
        { "W-3245", 32 },
        { "W-3245M", 32 },
        { "W-3265", 48 },
        { "W-3265M", 48 },
        { "W-3275", 56 },
        { "W-3275M", 56 }
    };

    // if it doesn't exist, return false
    if (thread_database.find(model.string.c_str()) == thread_database.end()) {
        return false;
    }

    const int threads = thread_database.at(model.string.c_str());

    debug("XEON_THREAD_MISMATCH: thread in database = ", static_cast<u32>(threads));

    return (std::thread::hardware_concurrency() != static_cast<unsigned int>(threads));
#endif
}