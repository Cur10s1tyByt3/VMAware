#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/* GPL */     // @brief Check if hostname is specific
/* GPL */     // @author InviZzzible project
/* GPL */     // @category Windows
/* GPL */     // @implements VM::HOSTNAME
/* GPL */     [[nodiscard]] static bool hostname_match() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         auto out_length = MAX_PATH;
/* GPL */         std::vector<u8> dns_host_name(static_cast<u32>(out_length), 0);
/* GPL */         GetComputerNameExA(ComputerNameDnsHostname, (LPSTR)dns_host_name.data(), (LPDWORD)&out_length);
/* GPL */ 
/* GPL */         debug("HOSTNAME: ", (LPCSTR)dns_host_name.data());
/* GPL */ 
/* GPL */         return (!lstrcmpiA((LPCSTR)dns_host_name.data(), "SystemIT"));
/* GPL */ #endif
/* GPL */     }