#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check if the system has a physical TPM by matching the TPM manufacturer against known physical TPM chip vendors
 * @category Windows
 * @note CRB model will succeed, while TIS will fail
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::TPM
 */
[[nodiscard]] static bool tpm() {
#if (!WINDOWS)
    return false;
#else
    struct TbsContext {
        TBS_HCONTEXT hContext = 0;
        explicit TbsContext(const TBS_CONTEXT_PARAMS2& params) {
            Tbsi_Context_Create(reinterpret_cast<PCTBS_CONTEXT_PARAMS>(&params), &hContext);
        }
        ~TbsContext() {
            if (hContext) {
                Tbsip_Context_Close(hContext);
            }
        }
        bool isValid() const { return hContext != 0; }
    };

    TBS_CONTEXT_PARAMS2 params{};
    params.version = TBS_CONTEXT_VERSION_TWO;
    params.includeTpm20 = 1;
    params.includeTpm12 = 1;

    TbsContext ctx(params);
    if (!ctx.isValid()) {
        return false;
    }

    // Prebuilt TPM2_GetCapability command for TPM_PT_MANUFACTURER
    static constexpr u8 cmd[] = {
        0x80,0x01,             // Tag: TPM_ST_NO_SESSIONS
        0x00,0x00,0x00,0x16,    // Command Size: 22
        0x00,0x00,0x01,0x7A,    // TPM2_GetCapability
        0x00,0x00,0x00,0x06,    // TPM_CAP_TPM_PROPERTIES
        0x00,0x00,0x01,0x05,    // TPM_PT_MANUFACTURER
        0x00,0x00,0x00,0x01     // Property Count: 1
    };

    u8 resp[1024] = {};
    u32 respSize = sizeof(resp);
    if (Tbsip_Submit_Command(ctx.hContext,
        TBS_COMMAND_LOCALITY_ZERO,
        TBS_COMMAND_PRIORITY_NORMAL,
        cmd,
        static_cast<u32>(sizeof(cmd)),
        resp,
        &respSize) != TBS_SUCCESS || respSize < 27) {
        return false;
    }

    const u32 manufacturerVal = (static_cast<u32>(resp[23]) << 24) |
        (static_cast<u32>(resp[24]) << 16) |
        (static_cast<u32>(resp[25]) << 8) |
        static_cast<u32>(resp[26]);
    switch (manufacturerVal) {
        case 0x414D4400u: // "AMD\0"
        case 0x41544D4Cu: // "ATML"
        case 0x4252434Du: // "BRCM"
        case 0x49424D00u: // "IBM\0"
        case 0x49465800u: // "IFX\0"
        case 0x494E5443u: // "INTC"
        case 0x4E534D20u: // "NSM "
        case 0x4E544300u: // "NTC\0"
        case 0x51434F4Du: // "QCOM"
        case 0x534D5343u: // "SMSC"
        case 0x53544D20u: // "STM "
        case 0x54584E00u: // "TXN\0"
        case 0x524F4343u: // "ROCC"
        case 0x4C454E00u: // "LEN\0"
            return false;
        default:
            return true;
    }
#endif
}