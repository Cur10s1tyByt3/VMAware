#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/* GPL */     // @brief Check for QEMU-specific blacklisted directories
/* GPL */     // @author LordNoteworthy
/* GPL */     // @link https://github.com/LordNoteworthy/al-khaser/blob/master/al-khaser/AntiVM/Qemu.cpp
/* GPL */     // @category Windows
/* GPL */     // @note from al-khaser project
/* GPL */     // @implements VM::QEMU_DIR
/* GPL */     [[nodiscard]] static bool qemu_dir() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         TCHAR szProgramFile[MAX_PATH];
/* GPL */         TCHAR szPath[MAX_PATH] = _T("");
/* GPL */ 
/* GPL */         const TCHAR* szDirectories[] = {
/* GPL */             _T("qemu-ga"),	// QEMU guest agent.
/* GPL */             _T("SPICE Guest Tools"), // SPICE guest tools.
/* GPL */         };
/* GPL */ 
/* GPL */         WORD iLength = sizeof(szDirectories) / sizeof(szDirectories[0]);
/* GPL */         for (int i = 0; i < iLength; i++) {
/* GPL */             if (util::is_wow64())
/* GPL */                 ExpandEnvironmentStrings(_T("%ProgramW6432%"), szProgramFile, ARRAYSIZE(szProgramFile));
/* GPL */             else
/* GPL */                 SHGetSpecialFolderPath(NULL, szProgramFile, CSIDL_PROGRAM_FILES, FALSE);
/* GPL */ 
/* GPL */             PathCombine(szPath, szProgramFile, szDirectories[i]);
/* GPL */ 
/* GPL */             if (util::exists(szPath))
/* GPL */                 return core::add(brands::QEMU);
/* GPL */         }
/* GPL */ 
/* GPL */         return false;
/* GPL */ #endif
/* GPL */     }