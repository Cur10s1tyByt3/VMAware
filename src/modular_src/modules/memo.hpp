#pragma once

#include "../types.hpp"
#include "../enums.hpp"
#include "../hyper_x.hpp"
#include <vector>
#include <map>

struct memo {
private:
    using points_t = u8;

public:
    struct data_t {
        bool result;
        points_t points;
    };

private:
    static std::map<u16, data_t> cache_table;
    static flagset cache_keys;

public:
    static void cache_store(const u16 technique_macro, const bool result, const points_t points);

    static bool is_cached(const u16 technique_macro);

    static data_t cache_fetch(const u16 technique_macro);

    static std::vector<u16> cache_fetch_all();

    // basically checks whether all the techniques were cached (with exception of techniques disabled by default)
    static bool all_present();

    struct brand {
        static std::string brand_cache;

        static std::string fetch() {
            return brand_cache;
        }

        static void store(const std::string& p_brand) {
            brand_cache = p_brand;
        }

        static bool is_cached() {
            return (!brand_cache.empty());
        }
    };

    struct multi_brand {
        static std::string brand_cache;

        static std::string fetch() {
            return brand_cache;
        }

        static void store(const std::string& p_brand) {
            brand_cache = p_brand;
        }

        static bool is_cached() {
            return (!brand_cache.empty());
        }
    };

    struct cpu_brand {
        static std::string brand_cache;

        static std::string fetch() {
            return brand_cache;
        }

        static void store(const std::string& p_brand) {
            brand_cache = p_brand;
        }

        static bool is_cached() {
            return (!brand_cache.empty());
        }
    };

    struct hyperx {
        static hyperx_state state;
        static bool cached;

        static hyperx_state fetch() {
            return state;
        }

        static void store(const hyperx_state p_state) {
            state = p_state;
            cached = true;
        }

        static bool is_cached() {
            return cached;
        }
    };
};