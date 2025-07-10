#include <malloc.h>
#include <zephyr/logging/log.h>

#include "system_info.h"
#include <zephyr/kernel.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/sys/sys_heap.h>

namespace eerie_leap::utilities::dev_tools {

LOG_MODULE_REGISTER(dev_tools_logger);

// Set CONFIG_THREAD_ANALYZER_ISR_STACK_USAGE for ISR stack usage info
void SystemInfo::print_thread_info(int cpu) {
    thread_analyzer_print(cpu);
}

static const char* stack_info_thread_name_filter = nullptr;
static void print_stack_info_callback(thread_analyzer_info* info) {
    if (stack_info_thread_name_filter != nullptr && strcmp(stack_info_thread_name_filter, info->name) != 0)
        return;

    // Print stack usage information
    size_t used_percent = (info->stack_used * 100U) / info->stack_size;

    LOG_INF("  %-14s: Unused %zu Usage %zu / %zu (%zu %%)",
        info->name,
        info->stack_size - info->stack_used,
        info->stack_used,
        info->stack_size,
        used_percent);
}

void SystemInfo::print_stack_info(int cpu, const char *thread_name) {
    stack_info_thread_name_filter = thread_name;

#ifdef CONFIG_THREAD_ANALYZER
    LOG_INF("Stack analyze for threads:");
    thread_analyzer_run((print_stack_info_callback), cpu);
#else
    LOG_ERR("Thread analyzer is not supprted.");
#endif
}

static const char* cpu_info_thread_name_filter = nullptr;
static void print_cpu_info_callback(thread_analyzer_info *info) {
    if (cpu_info_thread_name_filter != nullptr && strcmp(cpu_info_thread_name_filter, info->name) != 0)
        return;

    LOG_INF("  %-14s: CPU Load: %u %%",
		info->name,
		info->utilization);
}

void SystemInfo::print_cpu_info(int cpu, const char *thread_name) {
    cpu_info_thread_name_filter = thread_name;

#ifdef CONFIG_THREAD_ANALYZER
    LOG_INF("CPU analyze for threads:");
    thread_analyzer_run(print_cpu_info_callback, cpu);
#else
    LOG_ERR("Thread analyzer is not supprted.");
#endif
}

static void print_heap_stats(sys_heap *heap) {
    sys_memory_stats stats;
    sys_heap_runtime_stats_get(heap, &stats);

    size_t used_percent = (stats.allocated_bytes * 100U) / (stats.free_bytes + stats.allocated_bytes);

    LOG_INF("  %-14p: Unused %zu Usage %zu / %zu (%zu %%), Max Alloc %zu",
        heap,
        stats.free_bytes,
        stats.allocated_bytes,
        stats.free_bytes + stats.allocated_bytes,
        used_percent,
        stats.max_allocated_bytes);
}

void SystemInfo::print_heap_info() {
    sys_heap **heap_p;

    int heaps_count = sys_heap_array_get(&heap_p);
    LOG_INF("Heap analyze, there are %zu heaps allocated at addrs:", heaps_count);

    for (int i = 0; i < heaps_count; ++i)
        print_heap_stats(heap_p[i]);
}

} // namespace eerie_leap::utilities::dev_tools
