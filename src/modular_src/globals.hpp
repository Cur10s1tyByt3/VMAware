#pragma once

#include "types.hpp"
#include "enums.hpp"
#include <vector>

struct globals {
    static u8 detected_count_num; 
    static u16 technique_count; // get total number of techniques
    static std::vector<enum_flags> technique_vector;
    #ifdef __VMAWARE_DEBUG__
    static u16 total_points;
    #endif
};