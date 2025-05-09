#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/* GPL */     // @brief Check if the computer name (not username to be clear) is VM-specific
/* GPL */     // @category Windows
/* GPL */     // @author InviZzzible project
/* GPL */     // @implements VM::COMPUTER_NAME
/* GPL */     [[nodiscard]] static bool computer_name_match() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         auto out_length = MAX_PATH;
/* GPL */         std::vector<u8> comp_name(static_cast<u32>(out_length), 0);
/* GPL */         GetComputerNameA((LPSTR)comp_name.data(), (LPDWORD)&out_length);
/* GPL */ 
/* GPL */         auto compare = [&](const std::string& s) -> bool {
/* GPL */             return (std::strcmp((LPCSTR)comp_name.data(), s.c_str()) == 0);
/* GPL */         };
/* GPL */ 
/* GPL */         debug("COMPUTER_NAME: fetched = ", (LPCSTR)comp_name.data());
/* GPL */ 
/* GPL */         if (compare("InsideTm") || compare("TU-4NH09SMCG1HC")) { // anubis
/* GPL */             debug("COMPUTER_NAME: detected Anubis");
/* GPL */             return core::add(brands::ANUBIS);
/* GPL */         }
/* GPL */ 
/* GPL */         if (compare("klone_x64-pc") || compare("tequilaboomboom")) { // general
/* GPL */             debug("COMPUTER_NAME: detected general (VM but unknown)");
/* GPL */             return true;
/* GPL */         }
/* GPL */ 
/* GPL */         return false;
/* GPL */ #endif
/* GPL */     }