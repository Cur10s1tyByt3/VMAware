#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../brands.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"

/* GPL */     // @brief Checks for virtual machine signatures in disk drive device identifiers
/* GPL */     // @category Windows
/* GPL */     // @author Al-Khaser project
/* GPL */     // @implements VM::SETUPAPI_DISK
/* GPL */     [[nodiscard]] static bool setupapi_disk() {
/* GPL */ #if (!WINDOWS)
/* GPL */         return false;
/* GPL */ #else
/* GPL */         HDEVINFO hDevInfo;
/* GPL */         SP_DEVINFO_DATA DeviceInfoData{};
/* GPL */         DWORD i;
/* GPL */ 
/* GPL */         hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_DISKDRIVE,
/* GPL */             0,
/* GPL */             0,
/* GPL */             DIGCF_PRESENT);
/* GPL */ 
/* GPL */         if (hDevInfo == INVALID_HANDLE_VALUE)
/* GPL */             return false;
/* GPL */ 
/* GPL */         DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
/* GPL */         DWORD dwPropertyRegDataType;
/* GPL */         LPTSTR buffer = NULL;
/* GPL */         DWORD dwSize = 0;
/* GPL */ 
/* GPL */         for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
/* GPL */         {
/* GPL */             while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID,
/* GPL */                 &dwPropertyRegDataType, (PBYTE)buffer, dwSize, &dwSize))
/* GPL */             {
/* GPL */                 if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
/* GPL */                     if (buffer)LocalFree(buffer);
/* GPL */                     buffer = (LPTSTR)LocalAlloc(LPTR, static_cast<SIZE_T>(dwSize) * 2);
/* GPL */                     if (buffer == NULL)
/* GPL */                         break;
/* GPL */                 }
/* GPL */                 else
/* GPL */                     break;
/* GPL */ 
/* GPL */             }
/* GPL */ 
/* GPL */             if (buffer) {
/* GPL */                 if ((StrStrI(buffer, _T("vbox")) != NULL) ||
/* GPL */                     (StrStrI(buffer, _T("vmware")) != NULL) ||
/* GPL */                     (StrStrI(buffer, _T("qemu")) != NULL) ||
/* GPL */                     (StrStrI(buffer, _T("virtual")) != NULL))
/* GPL */                 {
/* GPL */                     return true;
/* GPL */                 }
/* GPL */             }
/* GPL */         }
/* GPL */ 
/* GPL */         if (buffer) LocalFree(buffer);
/* GPL */         SetupDiDestroyDeviceInfoList(hDevInfo);
/* GPL */         if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS) return false;
/* GPL */ 
/* GPL */         return false;
/* GPL */ #endif
/* GPL */     }