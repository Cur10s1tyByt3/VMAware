#pragma once

#include "../types.hpp"
#include <string>

// various cpu operation stuff
struct cpu {
    // cpuid leaf values
    struct leaf {
        static constexpr u32
            func_ext = 0x80000000,
            proc_ext = 0x80000001,
            brand1 = 0x80000002,
            brand2 = 0x80000003,
            brand3 = 0x80000004,
            hypervisor = 0x40000000,
            amd_easter_egg = 0x8fffffff;
    };

    // cross-platform wrapper function for linux and MSVC cpuid
    static void cpuid
    (
        u32& a, u32& b, u32& c, u32& d,
        const u32 a_leaf,
        const u32 c_leaf = 0xFF  // dummy value if not set manually
    );

    // same as above but for array type parameters (MSVC specific)
    static void cpuid
    (
        i32 x[4],
        const u32 a_leaf,
        const u32 c_leaf = 0xFF
    );

    // check for maximum function leaf
    static bool is_leaf_supported(const u32 p_leaf);

    // check AMD
    [[nodiscard]] static bool is_amd();

    // check Intel
    [[nodiscard]] static bool is_intel();

    // check for POSSIBILITY of hyperthreading, I don't think there's a 
    // full-proof method to detect if you're actually hyperthreading imo.
    [[nodiscard]] static bool has_hyperthreading();

    // get the CPU product
    [[nodiscard]] static std::string get_brand();

    // cpu manufacturer id
    [[nodiscard]] static std::string cpu_manufacturer(const u32 p_leaf);

    struct stepping_struct {
        u8 model;
        u8 family;
        u8 extmodel;
    };

    [[nodiscard]] static stepping_struct fetch_steppings();

    // check if the CPU is an intel celeron
    [[nodiscard]] static bool is_celeron(const stepping_struct steps);

    struct model_struct {
        bool found;
        bool is_xeon;
        bool is_i_series;
        bool is_ryzen;
        std::string string;
    };

    [[nodiscard]] static model_struct get_model();

    [[nodiscard]] static bool vmid_template(const u32 p_leaf);
};
