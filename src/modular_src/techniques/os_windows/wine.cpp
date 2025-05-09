#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/* GPL */     // @brief Check wine_get_unix_file_name file for Wine
/* GPL */     // @author pafish project
/* GPL */     // @link https://github.com/a0rtega/pafish/blob/master/pafish/wine.c
/* GPL */     // @category Windows
/* GPL */     // @implements VM::WINE_CHECK
/* GPL */     [[nodiscard]] static bool wine() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         
/* GPL */         const HMODULE k32 = GetModuleHandle(_T("kernel32.dll"));
/* GPL */ 
/* GPL */         if (k32 != NULL) {
/* GPL */             if (GetProcAddress(k32, "wine_get_unix_file_name") != NULL) {
/* GPL */                 return core::add(brands::WINE);
/* GPL */             }
/* GPL */         }
/* GPL */ 
/* GPL */         return false;
/* GPL */ #endif
/* GPL */     }