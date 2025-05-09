#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/**
 * @brief Check for official VPC method
 * @category Windows, x86_32
 * @implements VM::VPC_INVALID
 */
[[nodiscard]] static bool vpc_invalid() {
#if (WINDOWS && x86_32 && !CLANG)
    bool rc = false;

    auto IsInsideVPC_exceptionFilter = [](PEXCEPTION_POINTERS ep) -> DWORD {
        PCONTEXT ctx = ep->ContextRecord;

        ctx->Ebx = static_cast<DWORD>(-1); // Not running VPC
        ctx->Eip += 4; // skip past the "call VPC" opcodes
        return static_cast<DWORD>(EXCEPTION_CONTINUE_EXECUTION);
        // we can safely resume execution since we skipped faulty instruction
        };

    __try {
        __asm {
            push eax
            push ebx
            push ecx
            push edx

            mov ebx, 0h
            mov eax, 01h

            __emit 0Fh
            __emit 3Fh
            __emit 07h
            __emit 0Bh

            test ebx, ebx
            setz[rc]

            pop edx
            pop ecx
            pop ebx
            pop eax
        }
    }
    __except (IsInsideVPC_exceptionFilter(GetExceptionInformation())) {
        rc = false;
    }

    return rc;
#else
    return false;
#endif
}