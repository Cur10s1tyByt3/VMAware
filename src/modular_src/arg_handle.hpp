#pragma once

#include "enums.hpp"
#include "modules/core.hpp"
#include <stdexcept>


struct arg {
    static flagset flag_collector;
    static flagset disabled_flag_collector;

    static void generate_default(flagset& flags) {
        // set all bits to 1
        flags.set();

        // disable all non-default techniques
        flags.flip(VMWARE_DMESG);
        flags.flip(PORT_CONNECTORS);
        flags.flip(ACPI_TEMPERATURE);
        flags.flip(LSHW_QEMU);

        // disable all the settings flags
        flags.flip(NO_MEMO);
        flags.flip(HIGH_THRESHOLD);
        flags.flip(NULL_ARG);
        flags.flip(DYNAMIC);
        flags.flip(MULTIPLE);
        flags.flip(ALL);
    }

    static void generate_all(flagset& flags) {
        // set all bits to 1
        flags.set();

        // disable all the settings flags
        flags.flip(NO_MEMO);
        flags.flip(HIGH_THRESHOLD);
        flags.flip(NULL_ARG);
        flags.flip(DYNAMIC);
        flags.flip(MULTIPLE);
        flags.flip(DEFAULT);
    }

    static void generate_current_disabled_flags(flagset& flags) {
        const bool setting_no_memo = flags.test(NO_MEMO);
        const bool setting_high_threshold = flags.test(HIGH_THRESHOLD);
        const bool setting_dynamic = flags.test(DYNAMIC);
        const bool setting_multiple = flags.test(MULTIPLE);
        const bool setting_all = flags.test(ALL);
        const bool setting_default = flags.test(DEFAULT);

        if (disabled_flag_collector.count() == 0) {
            return;
        } else {
            flags &= disabled_flag_collector;
        }

        flags.set(NO_MEMO, setting_no_memo);
        flags.set(HIGH_THRESHOLD, setting_high_threshold);
        flags.set(DYNAMIC, setting_dynamic);
        flags.set(MULTIPLE, setting_multiple);
        flags.set(ALL, setting_all);
        flags.set(DEFAULT, setting_default);
    }

    static void disable_flagset_manager(const flagset& flags) {
        disabled_flag_collector = flags;
    }

    static void disable_flag_manager(const enum_flags flag) {
        disabled_flag_collector.set(flag, false);
    }

    static void flag_manager(const enum_flags flag) {
        if (
            (flag == INVALID) ||
            (flag > enum_size)
        ) {
            throw std::invalid_argument("Non-flag or invalid flag provided for VM::detect(), aborting");
        }

        if (flag == DEFAULT) {
            generate_default(flag_collector);
        } else if (flag == ALL) {
            generate_all(flag_collector);
        } else {
            flag_collector.set(flag);
        }
    }

    /**
     * basically what this entire recursive variadic template inheritance 
     * fuckery does is manage the variadic arguments being given through 
     * the arg_handler function, which could either be a std::bitset<N>, 
     * a uint8_t, or a combination of both of them. This will handle 
     * both argument types and implement them depending on what their 
     * types are. If it's a std::bitset<N>, do the |= operation on 
     * flag_collector. If it's a uint8_t, simply .set() that into the 
     * flag_collector. That's the gist of it.
     *
     * Also I won't even deny, the majority of this section was 90% generated
     * by chatgpt. Can't be arsed with this C++ variadic templatisation shit.
     * Like is it really my fault that I have a hard time understanging C++'s 
     * god awful metaprogramming designs? And don't even get me started on SFINAE. 
     * 
     * You don't need an IQ of 3 digits to realise how dogshit this language
     * is, when you end up in situations where there's a few correct solutions
     * to a problem, but with a billion ways you can do the same thing but in 
     * the "wrong" way. I genuinely can't wait for Carbon to come out.
     */

    // Define a base class for different types
    struct TestHandler {
        virtual ~TestHandler() = default;

        virtual void handle(const flagset& flags) {
            disable_flagset_manager(flags);
        }

        virtual void handle(const enum_flags flag) {
            flag_manager(flag);
        }
    };

    // Define a base class for different types
    struct DisableTestHandler {
        virtual ~DisableTestHandler() = default;

        virtual void disable_handle(const enum_flags flag) {
            disable_flag_manager(flag);
        }
    };

    // Define derived classes for specific type implementations
    struct TestBitsetHandler : public TestHandler {
        using TestHandler::handle; 

        void handle(const flagset& flags) override {
            disable_flagset_manager(flags);
        }
    };

    struct TestUint8Handler : public TestHandler {
        using TestHandler::handle;  

        void handle(const enum_flags flag) override {
            flag_manager(flag);
        }
    };

    struct DisableTestUint8Handler : public DisableTestHandler {
        using DisableTestHandler::disable_handle;  

        void disable_handle(const enum_flags flag) override {
            disable_flag_manager(flag);
        }
    };

    // Define a function to dispatch handling based on type
    template <typename T>
    static void dispatch(const T& value, TestHandler& handler) {
        handler.handle(value);
    }

    // Define a function to dispatch handling based on type
    template <typename T>
    static void disable_dispatch(const T& value, DisableTestHandler& handler) {
        handler.disable_handle(value);
    }

    // Base case for the recursive handling
    static void handleArgs() {
        // Base case: Do nothing
    }

    // Base case for the recursive handling
    static void handle_disabled_args() {
        // Base case: Do nothing
    }

    // Helper function to check if a given argument is of a specific type
    template <typename T, typename U>
    static bool isType(U&&) {
        return std::is_same<T, typename std::decay<U>::type>::value;
    }

    // Recursive case to handle each argument based on its type
    template <typename First, typename... Rest>
    static void handleArgs(First&& first, Rest&&... rest) {
        TestBitsetHandler bitsetHandler;
        TestUint8Handler uint8Handler;

        if (isType<flagset>(first)) {
            dispatch(first, bitsetHandler);
        } else if (isType<enum_flags>(first)) {
            dispatch(first, uint8Handler);
        } else {
            const std::string msg =
                "Arguments must either be a std::bitset<" +
                std::to_string(static_cast<u32>(enum_size + 1)) +
                "> such as VM::DEFAULT, or a flag such as VM::RDTSC for example";

            throw std::invalid_argument(msg);
        }

        // Recursively handle the rest of the arguments
        handleArgs(std::forward<Rest>(rest)...);
    }

    // Recursive case to handle each argument based on its type
    template <typename First, typename... Rest>
    static void handle_disabled_args(First&& first, Rest&&... rest) {
        DisableTestUint8Handler Disableuint8Handler;

        if (isType<flagset>(first)) {
            throw std::invalid_argument("Arguments must not contain VM::DEFAULT or VM::ALL, only technique flags are accepted (view the documentation for a full list)");
        } else if (isType<enum_flags>(first)) {
            disable_dispatch(first, Disableuint8Handler);
        } else {
            throw std::invalid_argument("Arguments must be a technique flag, aborting");
        }

        // Recursively handle the rest of the arguments
        handle_disabled_args(std::forward<Rest>(rest)...);
    }

    template <typename... Args>
    static constexpr bool is_empty() {
        return (sizeof...(Args) == 0);
    }

public:
    // fetch the flags, could be an enum value OR a std::bitset.
    // This will then generate a different std::bitset as the 
    // return value by enabling the bits based on the argument.
    template <typename... Args>
    static flagset arg_handler(Args&&... args) {
        flag_collector.reset();
        generate_default(disabled_flag_collector);

        if VMAWARE_CONSTEXPR (is_empty<Args...>()) {
            generate_default(flag_collector);
            return flag_collector;
        }

        // set the bits in the flag, can take in 
        // either an enum value or a std::bitset
        handleArgs(std::forward<Args>(args)...);
        
        if (flag_collector.count() == 0) {
            generate_default(flag_collector);
        }


        const bool setting_no_memo = flag_collector.test(NO_MEMO);
        const bool setting_high_threshold = flag_collector.test(HIGH_THRESHOLD);
        const bool setting_dynamic = flag_collector.test(DYNAMIC);
        const bool setting_multiple = flag_collector.test(MULTIPLE);
        const bool setting_all = flag_collector.test(ALL);
        const bool setting_default = flag_collector.test(DEFAULT);

        flag_collector &= disabled_flag_collector;

        flag_collector.set(NO_MEMO, setting_no_memo);
        flag_collector.set(HIGH_THRESHOLD, setting_high_threshold);
        flag_collector.set(DYNAMIC, setting_dynamic);
        flag_collector.set(MULTIPLE, setting_multiple);
        flag_collector.set(ALL, setting_all);
        flag_collector.set(DEFAULT, setting_default);


        // handle edgecases
        core::flag_sanitizer(flag_collector);

        return flag_collector;
    }

    // same as above but for VM::disable which only accepts technique flags
    template <typename... Args>
    static void disabled_arg_handler(Args&&... args) {
        disabled_flag_collector.reset();

        generate_default(disabled_flag_collector);

        if VMAWARE_CONSTEXPR (is_empty<Args...>()) {
            throw std::invalid_argument("VM::DISABLE() must contain a flag");
        }

        handle_disabled_args(std::forward<Args>(args)...);

        // check if a settings flag is set, which is not valid
        if (core::is_setting_flag_set(disabled_flag_collector)) {
            throw std::invalid_argument("VM::DISABLE() must not contain a settings flag, they are disabled by default anyway");
        }

        return;
    }
};