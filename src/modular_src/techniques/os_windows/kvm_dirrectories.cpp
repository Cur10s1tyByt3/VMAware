#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"


/* GPL */     // @brief Check for KVM directory "Virtio-Win"
/* GPL */     // @category Windows
/* GPL */     // @author LordNoteWorthy
/* GPL */     // @note from Al-Khaser project
/* GPL */     // @link https://github.com/LordNoteworthy/al-khaser/blob/0f31a3866bafdfa703d2ed1ee1a242ab31bf5ef0/al-khaser/AntiVM/KVM.cpp
/* GPL */     // @implements VM::KVM_DIRS
/* GPL */ [[nodiscard]] static bool kvm_directories() {
/* GPL */ #if (!WINDOWS)
/* GPL */     return false;
/* GPL */ #else
/* GPL */     char basePath[MAX_PATH];
/* GPL */     const char* programDir;
/* GPL */
/* GPL */     if (util::is_wow64()) {     
/* GPL */         DWORD len = GetEnvironmentVariable(_T("ProgramW6432"), basePath, MAX_PATH);
/* GPL */         if (len == 0 || len >= MAX_PATH) return false;
/* GPL */         programDir = basePath;
/* GPL */         }
/* GPL */         else {  
/* GPL */           if (SHGetFolderPath(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, basePath) != S_OK) {
/* GPL */               return false;
/* GPL */           }
/* GPL */           programDir = basePath;
/* GPL */         }
/* GPL */   
/* GPL */     const char suffix[] = "\\Virtio-Win\\";
/* GPL */     size_t dirLen = strlen(programDir);
/* GPL */     size_t suffLen = sizeof(suffix) - 1;
/* GPL */     if (dirLen + suffLen >= MAX_PATH) return false;
/* GPL */     memcpy(basePath + dirLen, suffix, suffLen + 1);
/* GPL */
/* GPL */     return util::exists(basePath);
/* GPL */ #endif
/* GPL */     }