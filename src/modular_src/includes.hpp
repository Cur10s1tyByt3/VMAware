#pragma once

#include "types.hpp"

#ifdef _MSC_VER
#pragma warning(push) // Save current warning state and disable all warnings for external Windows header files
#pragma warning(disable : 4365)
#pragma warning(disable : 4668)
#pragma warning(disable : 4820)
#pragma warning(disable : 5039)
#pragma warning(disable : 5204)
#endif

#if (CPP >= 23)
#include <limits>
#endif
#if (CPP >= 20)
#include <bit>
#include <ranges>
#include <source_location>
#endif
#if (CPP >= 17)
#include <filesystem>
#endif
#ifdef __VMAWARE_DEBUG__
#include <iomanip>
#include <ios>
#include <locale>
#include <codecvt>
#endif

#include <functional>
#include <cstring>
#include <string>
#include <fstream>
#include <regex>
#include <thread>
#include <cstdint>
#include <map>
#include <unordered_set>
#include <array>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
#include <sstream>
#include <bitset>
#include <type_traits>

#if (WINDOWS)
#include <windows.h>
#include <intrin.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <winioctl.h>
#include <winternl.h>
#include <winuser.h>
#include <psapi.h>
#include <shlwapi.h>
#include <shlobj_core.h>
#include <winspool.h>
#include <powerbase.h>
#include <setupapi.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <tbs.h>
#include <mutex>
#include <initguid.h>
#include <devpkey.h>
#include <devguid.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "MPR")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "tbs.lib")

#elif (LINUX)
#if (x86)
#include <cpuid.h>
#include <x86intrin.h>
#include <immintrin.h>
#endif
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <net/if.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <memory>
#include <cctype>
#include <fcntl.h>
#include <limits.h>
#include <csignal>      
#include <csetjmp>      
#include <pthread.h>     
#include <sched.h>      
#include <cerrno>         
#elif (APPLE)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <chrono>
#endif

#ifdef _MSC_VER
#pragma warning(pop)  // Restore external Windows header file warnings
#endif