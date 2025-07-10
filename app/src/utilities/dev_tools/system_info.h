#pragma once

namespace eerie_leap::utilities::dev_tools {

class SystemInfo {
public:
    /** @brief Run the thread analyzer and print stack size statistics.
     *
     *  This function runs the thread analyzer and prints the output in
     *  standard form. In the special case when Kconfig option
     *  THREAD_ANALYZER_AUTO_SEPARATE_CORES is set, the function analyzes
     *  only the threads running on the specified cpu.
     *
     *  @param cpu cpu to analyze, ignored if THREAD_ANALYZER_AUTO_SEPARATE_CORES=n
     */
    static void print_thread_info(int cpu = 0);
    static void print_stack_info(int cpu = 0, const char *thread_name = nullptr);
    static void print_cpu_info(int cpu = 0, const char *thread_name = nullptr);
    static void print_heap_info();
};

} // namespace eerie_leap::utilities::dev_tools