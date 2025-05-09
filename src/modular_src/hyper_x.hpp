#pragma once

#include "types.hpp"

// specifically for util::hyper_x() and memo::hyperv
enum hyperx_state : u8 {
    HYPERV_UNKNOWN_VM = 0,
    HYPERV_REAL_VM,
    HYPERV_ARTIFACT_VM,
    HYPERV_ENLIGHTENMENT
};
