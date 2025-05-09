

#pragma once



MSVC_DISABLE_WARNING(ASSIGNMENT_OPERATOR NO_INLINE_FUNC SPECTRE)

struct VM {


public:
    


public:
    // this will allow the enum to be used in the public interface as "VM::TECHNIQUE"
    enum enum_flags tmp_ignore_this = NO_MEMO;

    // constructor stuff ignore this
    VM() = delete;
    VM(const VM&) = delete;
    VM(VM&&) = delete;



private: // START OF PRIVATE VM DETECTION TECHNIQUE DEFINITIONS
   








public: // START OF PUBLIC FUNCTIONS

};


// ============= EXTERNAL DEFINITIONS =============
// These are added here due to warnings related to C++17 inline variables for C++ standards that are under 17.
// It's easier to just group them together rather than having C++17<= preprocessors with inline stuff


VM::u8 VM::detected_count_num = 0;


std::vector<VM::enum_flags> VM::technique_vector = []() -> std::vector<VM::enum_flags> {
    std::vector<VM::enum_flags> tmp{};

    // all the techniques have a macro value starting from 0 to ~90, hence why it's a classic loop
    for (u8 i = VM::technique_begin; i < VM::technique_end; i++) {
        tmp.push_back(static_cast<VM::enum_flags>(i));
    }

    return tmp;
}();


// this value is incremented each time VM::add_custom is called
VM::u16 VM::technique_count = base_technique_count;
