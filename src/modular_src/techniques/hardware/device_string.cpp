#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"



/**
 * @brief Check if bogus device string would be accepted
 * @category Windows
 * @author Huntress Research Team
 * @link https://unprotect.it/technique/buildcommdcbandtimeouta/
 * @implements VM::DEVICE_STRING
 */
[[nodiscard]] static bool device_string() {
#if (!WINDOWS)
    return false;
#else
    DCB dcb = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    if (BuildCommDCBAndTimeoutsA("jhl46745fghb", &dcb, &timeouts)) {
        return true;
    } else {
        return false;
    }
#endif
}