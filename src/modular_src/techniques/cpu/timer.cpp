#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/util.hpp"
#include "../../includes.hpp"


/**
 * @brief Check for timing anomalies in the system
 * @category x86
 * @author Requiem (https://github.com/NotRequiem)
 * @implements VM::TIMER
 */
#if (CLANG)
    #pragma clang optimize off
#elif (MSVC)
    #pragma optimize("", off)
#elif (GCC)
    #pragma GCC push_options
    #pragma GCC optimize("O0")
#endif
[[nodiscard]]
#if (LINUX)
    __attribute__((no_sanitize("address", "leak", "thread", "undefined")))
#endif
static bool timer() {
#if (ARM || !x86)
    return false;
#else
    if (util::hyper_x() == HYPERV_ARTIFACT_VM) {
        return false;
    }

    if (util::is_running_under_translator()) {
        debug("Running inside binary translation layer.");
        return false;
    }

    // to minimize context switching/scheduling
#if (WINDOWS)
    const HANDLE hThread = GetCurrentThread();
    const int oldPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
#else
    bool hasSchedPriority = (geteuid() == 0);
    int oldPolicy = SCHED_OTHER;
    sched_param oldParam{};

    if (hasSchedPriority) {
        oldPolicy = sched_getscheduler(0);
        sched_getparam(0, &oldParam);
        sched_param newParam{};
        newParam.sched_priority = sched_get_priority_max(SCHED_FIFO);

        if (sched_setscheduler(0, SCHED_FIFO, &newParam) == -1) {
            hasSchedPriority = false;  
        }
    }
#endif

    auto restoreThreadPriority = [&]() {
#if (WINDOWS)
        SetThreadPriority(hThread, oldPriority);
#else
        sched_setscheduler(0, oldPolicy, &oldParam);
#endif
    };

#if (WINDOWS)
    DWORD_PTR procMask, sysMask;
    GetProcessAffinityMask(GetCurrentProcess(), &procMask, &sysMask);
#elif (LINUX)
    cpu_set_t oldTscSet;
    CPU_ZERO(&oldTscSet);
    pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &oldTscSet);
#endif

    // 1. TSC Synchronization Check Across Cores
    // Try reading the invariant TSC on two different cores to attempt to detect vCPU timers being shared
    constexpr u8 tscIterations = 10;
    constexpr u16 tscSyncDiffThreshold = 500;

    bool tscSyncDetected = false;
    tscSyncDetected = [&]() noexcept -> bool {
        int tscIssueCount = 0;
        u64 tscCore1 = 0, tscCore2 = 0;

        for (int i = 0; i < tscIterations; ++i) {
            unsigned int aux = 0;
#if (WINDOWS)
            DWORD_PTR prevMask = SetThreadAffinityMask(GetCurrentThread(), DWORD{ 1 });
            if (prevMask == 0) {
                // could not bind even to the first core...?
                return true;
            }

            __try {
                tscCore1 = __rdtscp(&aux);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                // __rdtscp should be supported nowadays
                return true;
            }

            DWORD_PTR gotMask = SetThreadAffinityMask(GetCurrentThread(), DWORD{ 2 });
            if (gotMask == 0) {
                // fails because there is no core #1 on a single-core box
                return false;
            }

            __try {
                tscCore2 = __rdtscp(&aux);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return true;
            }
#else
            cpu_set_t set;
            static sigjmp_buf jumpBuf;

            // Use a non-capturing lambda cast to a function pointer for signal handling
            struct sigaction oldAct, newAct {};
            newAct.sa_flags = SA_SIGINFO;
            using sa_sigaction_fn = void (*)(int, siginfo_t*, void*);
            newAct.sa_sigaction = static_cast<sa_sigaction_fn>(
                [](int, siginfo_t*, void*) {
                    siglongjmp(jumpBuf, 2);
                }
                );
            sigemptyset(&newAct.sa_mask);

            // core 0
            CPU_ZERO(&set);
            CPU_SET(0, &set);
            int ret = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
            if (ret == EINVAL) {
                // no CPU 0? super weird
                return true;
            }
            else if (ret != 0) {
                // assume single-core
                return false;
            }

            sigaction(SIGILL, &newAct, &oldAct);

            if (sigsetjmp(jumpBuf, 1) == 0) {
                tscCore1 = __rdtscp(&aux);
            }
            else {
                // no rdtscp support
                sigaction(SIGILL, &oldAct, nullptr);
                return true;
            }

            sigaction(SIGILL, &oldAct, nullptr);

            // core 1
            CPU_ZERO(&set);
            CPU_SET(1, &set);
            ret = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
            if (ret == EINVAL) {
                // single‑core 
                return false;
            }
            else if (ret != 0) {
                return true;
            }

            sigaction(SIGILL, &newAct, &oldAct);
            if (sigsetjmp(jumpBuf, 1) == 0) {
                tscCore2 = __rdtscp(&aux);
            }
            else {
                sigaction(SIGILL, &oldAct, nullptr);
                return true;
            }

            sigaction(SIGILL, &oldAct, nullptr);
#endif
            // hypervisors often have nearly identical TSCs across vCPUs
            const u64 diff = (tscCore2 > tscCore1)
                ? (tscCore2 - tscCore1)
                : (tscCore1 - tscCore2);

            if (diff < tscSyncDiffThreshold) {            
                ++tscIssueCount;
            }
        }
#ifdef __VMAWARE_DEBUG__
        debug("TIMER: TSC sync check",
            " - Core1: ", tscCore1,
            " Core2: ", tscCore2,
            " Delta: ", tscCore2 - tscCore1,
            " (Threshold: <", tscSyncDiffThreshold,
            ')'
        );
#endif  
        return (tscIssueCount >= tscIterations / 2);
    }();

#if (WINDOWS)
    SetThreadAffinityMask(GetCurrentThread(), procMask);
#elif (LINUX)
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &oldTscSet);
#endif

    if (tscSyncDetected) {
        restoreThreadPriority();
        return true;
    }

#if (WINDOWS)
    // 2. Ratio-based Timing Check on Single Core
    // measure trapping vs non-trapping instructions

    // warm-up to reduce noise
    for (int i = 0; i < 10; ++i) {
        unsigned aux;
    #if (MSVC)
        int cpuInfo[4]; __cpuid(cpuInfo, 0);
        __rdtsc(); 
        GetProcessHeap();
        __rdtscp(&aux);
        #pragma warning (disable : 6387)
        CloseHandle((HANDLE)0);
        #pragma warning (default : 6387)
        __rdtscp(&aux);
    #elif (GCC) || (CLANG)
        unsigned low, high;
        __asm__ __volatile__(
            "cpuid\n\t"
            "rdtsc\n\t"
            : "=a"(low), "=d"(high)
            : "a"(0)
            : "ebx", "ecx"
        );
        GetProcessHeap();
        __asm__ __volatile__(
            "rdtscp\n\t"
            "cpuid"
            : "=a"(low), "=d"(high)
            :: "rbx", "rcx"
        );
        CloseHandle((HANDLE)0);
        __asm__ __volatile__(
            "rdtscp\n\t"
            "cpuid"
            : "=a"(low), "=d"(high)
            :: "rbx", "rcx"
        );
    #endif
    }

    // actual measurement
    constexpr u8  SAMPLE_COUNT = 100;
    constexpr u16 SCALE_FACTOR = 1000;
    constexpr u32 THRESHOLD_SCALED = 10 * SCALE_FACTOR;  // <10× ratio => VM
    u64 samples[SAMPLE_COUNT] = { 0 };

    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        unsigned aux;
        u64 t0, t1, t2;
    #if (MSVC)
        int cpuInfo[4]; __cpuid(cpuInfo, 0);
        t0 = __rdtsc();
        GetProcessHeap();
        t1 = __rdtscp(&aux);
    #pragma warning (disable : 6387)
        CloseHandle((HANDLE)0);
    #pragma warning (default : 6387)
        t2 = __rdtscp(&aux);
    #else
        unsigned low, high;
        __asm__ __volatile__(
            "cpuid\n\t"
            "rdtsc\n\t"
            : "=a"(low), "=d"(high)
            : "a"(0)
            : "ebx", "ecx"
        );
        t0 = ((u64)high << 32) | low;

        GetProcessHeap();

        __asm__ __volatile__(
            "rdtscp\n\t"
            "cpuid"
            : "=a"(low), "=d"(high)
            :: "rbx", "rcx"
        );
        t1 = ((u64)high << 32) | low;

        CloseHandle((HANDLE)0);

        __asm__ __volatile__(
            "rdtscp\n\t"
            "cpuid"
            : "=a"(low), "=d"(high)
            :: "rbx", "rcx"
        );
        t2 = ((u64)high << 32) | low;
    #endif
        const u64 heapCost = t1 - t0;
        const u64 closeCost = t2 - t1;
        samples[i] = (heapCost > 0)
            ? ((closeCost * SCALE_FACTOR) / heapCost)
            : UINT64_MAX;
    }

    std::sort(std::begin(samples), std::end(samples));
    restoreThreadPriority();
    const u64 median = samples[SAMPLE_COUNT / 2];
    debug("TIMER: Ratio: ", median, " - Threshold: <", THRESHOLD_SCALED);

    if (median < THRESHOLD_SCALED) {
        return true;
    }

    // simple check to detect poorly coded RDTSC patches
    typedef struct _PROCESSOR_POWER_INFORMATION {
        ULONG Number;
        ULONG MaxMhz;
        ULONG CurrentMhz;
        ULONG MhzLimit;
        ULONG MaxIdleState;
        ULONG CurrentIdleState;
    } PROCESSOR_POWER_INFORMATION, * PPROCESSOR_POWER_INFORMATION;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD procCount = sysInfo.dwNumberOfProcessors;

    std::vector<PROCESSOR_POWER_INFORMATION> ppi(procCount);

    const NTSTATUS status = CallNtPowerInformation(
        ProcessorInformation,
        nullptr,
        0,
        ppi.data(),
        sizeof(PROCESSOR_POWER_INFORMATION) * procCount
    );

    if (status != 0)
        return false;       

    for (DWORD i = 0; i < procCount; ++i) {
        if (ppi[i].CurrentMhz < 1000) {
            return true;
        }
    }

#endif
    return false;
#endif
}
#if (CLANG)
    #pragma clang optimize off
#elif (MSVC)
    #pragma optimize("", off)
#elif (GCC)
    #pragma GCC pop_options
#endif