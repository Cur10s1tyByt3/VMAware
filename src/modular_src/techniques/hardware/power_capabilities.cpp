#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/* GPL */     // @brief Check what power states are enabled
/* GPL */     // @category Windows
/* GPL */     // @author Al-Khaser project
/* GPL */     // @implements VM::POWER_CAPABILITIES
/* GPL */     [[nodiscard]] static bool power_capabilities() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         SYSTEM_POWER_CAPABILITIES powerCaps;
/* GPL */         bool power_stats = false;
/* GPL */         if (GetPwrCapabilities(&powerCaps) == TRUE)
/* GPL */         {
/* GPL */             if ((powerCaps.SystemS1 | powerCaps.SystemS2 | powerCaps.SystemS3 | powerCaps.SystemS4) == FALSE)
/* GPL */             {
/* GPL */                 power_stats = (powerCaps.ThermalControl == FALSE);
/* GPL */             }
/* GPL */                           
/* GPL */         }
/* GPL */ 
/* GPL */         return power_stats;
/* GPL */ #endif
/* GPL */     }