#pragma once

#include "../types.hpp"
#include "../enums.hpp"

#include <functional>
#include <map>


struct core {
    MSVC_DISABLE_WARNING(PADDING)
    struct technique {
        u8 points = 0;                // this is the certainty score between 0 and 100
        std::function<bool()> run;    // this is the technique function itself
    
        technique() : points(0), run(nullptr) {}

        technique(u8 points, std::function<bool()> run) : points(points), run(run) {}
    };

    struct custom_technique {
        u8 points;
        u16 id;
        std::function<bool()> run;
    };
    MSVC_ENABLE_WARNING(PADDING)

    // initial technique list, this is where all the techniques are stored
    static std::pair<enum_flags, technique> technique_list[];

    // the actual table, which is derived from the list above and will be 
    // used for most functionalities related to technique interactions
    static std::map<enum_flags, technique> technique_table;

    // specific to VM::add_custom(), where custom techniques will be stored here
    static std::vector<custom_technique> custom_table;
    
    // VM scoreboard table specifically for VM::brand()
    static std::map<const char*, brand_score_t> brand_scoreboard;

    // directly return when adding a brand to the scoreboard for a more succint expression
    static inline bool add(const char* p_brand, const char* extra_brand = "") noexcept;

    // assert if the flag is enabled, far better expression than typing std::bitset member functions
    [[nodiscard]] static inline bool is_disabled(const flagset& flags, const u8 flag_bit) noexcept;

    // same as above but for checking enabled flags
    [[nodiscard]] static inline bool is_enabled(const flagset& flags, const u8 flag_bit) noexcept;

    [[nodiscard]] static bool is_technique_set(const flagset& flags);

    [[nodiscard]] static bool is_setting_flag_set(const flagset& flags);

    // manage the flag to handle edgecases
    static void flag_sanitizer(flagset& flags);

    // run every VM detection mechanism in the technique table
    static u16 run_all(const flagset& flags, const bool shortcut = false);
};