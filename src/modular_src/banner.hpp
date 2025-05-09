/**
 * ██╗   ██╗███╗   ███╗ █████╗ ██╗    ██╗ █████╗ ██████╗ ███████╗
 * ██║   ██║████╗ ████║██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔════╝
 * ██║   ██║██╔████╔██║███████║██║ █╗ ██║███████║██████╔╝█████╗
 * ╚██╗ ██╔╝██║╚██╔╝██║██╔══██║██║███╗██║██╔══██║██╔══██╗██╔══╝
 *  ╚████╔╝ ██║ ╚═╝ ██║██║  ██║╚███╔███╔╝██║  ██║██║  ██║███████╗
 *   ╚═══╝  ╚═╝     ╚═╝╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝ 2.3.0 (May 2025)
 *
 *  C++ VM detection library
 *
 *  - Made by: kernelwernel (https://github.com/kernelwernel)
 *  - Co-developed by: Requiem (https://github.com/NotRequiem)
 *  - Contributed by:
 *      - Alex (https://github.com/greenozon)
 *      - Marek Knápek (https://github.com/MarekKnapek)
 *      - Vladyslav Miachkov (https://github.com/fameowner99)
 *      - Alan Tse (https://github.com/alandtse)
 *      - Georgii Gennadev (https://github.com/D00Movenok)
 *      - utoshu (https://github.com/utoshu)
 *      - Jyd (https://github.com/jyd519)
 *      - dmfrpro (https://github.com/dmfrpro)
 *      - Pierre-Étienne Messier (https://github.com/pemessier)
 *  - Repository: https://github.com/kernelwernel/VMAware
 *  - Docs: https://github.com/kernelwernel/VMAware/docs/documentation.md
 *  - Full credits: https://github.com/kernelwernel/VMAware#credits-and-contributors-%EF%B8%8F
 *  - License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.html)
 *
 *
 * ============================== SECTIONS ==================================
 * - enums for publicly accessible techniques  => line 557
 * - struct for internal cpu operations        => line 743
 * - struct for internal memoization           => line 1209
 * - struct for internal utility functions     => line 1337
 * - struct for internal core components       => line 10102
 * - start of VM detection technique list      => line 2450
 * - start of public VM detection functions    => line 10759
 * - start of externally defined variables     => line 11710
 *
 *
 * ============================== EXAMPLE ===================================
 * #include "vmaware.hpp"
 * #include <iostream>
 * 
 * int main() {
 *     if (VM::detect()) {
 *         std::cout << "Virtual machine detected!" << "\n";
 *     } else {
 *         std::cout << "Running on baremetal" << "\n";
 *     }
 * 
 *     std::cout << "VM name: " << VM::brand() << "\n";
 *     std::cout << "VM type: " << VM::type() << "\n";
 *     std::cout << "VM certainty: " << (int)VM::percentage() << "%" << "\n";
 * }
 * 
 *
 * ========================== CODE DOCUMENTATION =============================
 * 
 * Welcome! This is just a preliminary text to lay the context of how it works, 
 * how it's structured, and guide anybody who's trying to understand the whole code. 
 * Reading over 12k+ lines of other people's C++ code is obviously not an easy task, 
 * and that's perfectly understandable. I'd struggle as well if I was in your position
 * while not even knowing where to start. So here's a more human-friendly explanation:
 * 
 * 
 * Firstly, the lib is completely static, meaning that there's no need for struct 
 * constructors to be initiated (unless you're using the VM::vmaware struct).
 * The main focus of the lib are the tables:
 *  - the TECHNIQUE table stores all the VM detection technique information in a std::map 
 * 
 *  - the BRAND table stores every VM brand as a std::map as well, but as a scoreboard. 
 *    This means that if a VM detection has detected a VM brand, that brand will have an
 *    incremented score. After every technique is run, the brand with the highest score
 *    is chosen as the officially detected brand. 
 * 
 * The techniques are all static functions, which all return a boolean. There are a few 
 * categories of techniques that target vastly different things such as OS queries, CPU
 * values, other hardware values, firmware data, and system files just to name a few. 
 * 
 * 
 * Secondly, there are multiple modules in the lib that are combined to integrate with
 * the functionalities needed:
 *    - core module:
 *        This contains many important components such as the aforementioned tables, 
 *        the standard structure for how VM techniques are organised, functionalities 
 *        to run all the techniques in the technique table, functionalities to run
 *        custom-made techniques by the user, and an argument handler based on the 
 *        arguments inputted by the user.
 *
 *    - cpu module:
 *        As the name suggests, this contains functionalities for the CPU. There are
 *        many techniques that utilise some kind of low-level CPU interaction, so 
 *        this module was added to further standardise it.
 * 
 *    - memo module:
 *        This contains functionalities for memoizing technique results (not to be
 *        confused with "memorization"). More specifically, this allows us to cache 
 *        a technique result in a table where each entry contains a technique and its
 *        result. This allows us to avoid re-running techniques which happens a lot
 *        internally. Some techniques are costlier than others in terms of 
 *        performance, so this is a crucial module that allows us to save a lot of
 *        time. Additionally, it contains other memoization caches for various other
 *        things for convenience purposes. 
 * 
 *    - util module:
 *        This contains many utility functionalities to be used by the techniques.
 *        Examples of functionalities include file I/O, registries, permission 
 *        checks, system commands, HDD sizes, RAM sizes, debugs, process checking, 
 *        OS queries, Hyper-X, and so on. (It should be mentioned that this is 
 *        probably the least enjoyable part of the lib to read, since it's really messy)
 * 
 * 
 * Thirdly, I'll explain in this section how all of these facets of the lib interact with 
 * each other. Let's take an example with VM::detect(), where it returns a boolean true or 
 * false if a VM has been detected or not. The chain of steps it takes goes like this:
 *    1. The function tries to handle the user arguments (if there's 
 *       any), and generates a std::bitset. This bitset has a length of 
 *       every VM detection technique + settings, where each bit 
 *       corresponds to whether this technique will be run or not, 
 *       and which settings were selected. 
 * 
 *    2. After the bitset has been generated, this information is then 
 *       passed to the core module of the lib. It analyses the bitset, 
 *       and runs every VM detection technique that has been selected, 
 *       while ignoring the ones that weren't selected (by default most 
 *       of them are already selected anyway). The function that does 
 *       this mechanism is core::run_all()
 * 
 *    3. While the core::run_all() function is being ran, it checks if 
 *       each technique has already been memoized or not. If it has, 
 *       retrieve the result from the cache and move to the next technique. 
 *       If it hasn't, run the technique and cache the result to the 
 *       cache table. 
 * 
 *    4. After every technique has been executed, this generates a 
 *       uint16_t score. Every technique has a score value between 0 to 
 *       100, and if they are detected then this score is accumulated to 
 *       a total score. If the total is above 150, that means it's a VM[1]. 
 * 
 * 
 * There are other functions such as VM::brand(), which returns a std::string of the most 
 * likely brand that your system is running on. It has a bit of a different mechanism:
 *    1. Same as step 1 in VM::detect()
 * 
 *    2. Check if the majority of techniques have been run already and stored
 *       in the cache. If not, invoke core::run_all(). The reason why this is
 *       important is because a lot of techniques increment a point for its 
 *       respected brand that was detected. For example, if the VM::QEMU_USB
 *       technique has detected a VM, it'll add a score to the QEMU brand in
 *       the scoreboard. If no technique has been run, then there's no way to
 *       populate the scoreboard with any points. After every VM detection 
 *       technique has been invoked/retrieved, the brand scoreboard is now
 *       ready to be analysed.
 * 
 *    3. Create a filter for the scoreboard, where every brand that has a score
 *       of 0 are erased for abstraction purposes. Now the scoreboard is only
 *       populated with relevant brands where they all have at least a single
 *       point. These are the contenders for which brand will be outputted.
 * 
 *    4. Merge certain brand combinations together. For example, Azure's cloud 
 *       is based on Hyper-V, but Hyper-V may have a higher score due to the 
 *       prevalence of it in a practical setting, which will put Azure to the 
 *       side. In reality, there should be an indication that Azure is involved
 *       since it's a better idea to let the user know that the brand is "Azure 
 *       Hyper-V" instead of just "Hyper-V". So what this step does is "merge" 
 *       the brands together to form a more accurate idea of the brand(s) involved.
 * 
 *    5. After all of this, the scoreboard is sorted in descending order, where
 *       the brands with the highest points are now selected as the official 
 *       output of the VM::brand() function.
 * 
 *    6. The result is then cached to the memo module, so if another function
 *       invokes VM:brand() again, "the result is retrieved from the cache 
 *       without needing to run all of the previous steps again.
 *      
 * (NOTE: it's a bit more complicated than this, but that's the gist of how this function works)
 * 
 * Most of the functions provided usually depend on the 2 techniques covered. 
 * And they serve as a functionality base for other components of the lib.
 *      
 *  
 *  [1]: If the user has provided a setting argument called VM::HIGH_THRESHOLD, 
 *       the threshold becomes 300 instead of 150.
 */