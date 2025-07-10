#pragma once

#include <string>
#include <chrono>

#include <zephyr/timing/timing.h>

namespace eerie_leap::utilities::time {

using namespace std::chrono;

class TimeHelpers {
public:
    static std::string GetFormattedString(const std::chrono::system_clock::time_point& tp) {
        using namespace std::chrono;

        auto duration = tp.time_since_epoch();
        auto secs = duration_cast<seconds>(duration);
        auto millis = duration_cast<milliseconds>(duration - secs).count();

        std::time_t time_sec = system_clock::to_time_t(tp);
        std::tm* timeinfo = localtime(&time_sec);

        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        std::string formatted_time = std::string(buffer);
        return formatted_time + "." + std::to_string(millis);
    }

    /**
     * @brief Measures the execution time of a function in microseconds
     *
     * Example:
     * @code
     * float execution_time = TimeHelpers::MeasureExecutionTimeUs([]() {
     *     // Your code here
     * });
     * @endcode
     *
     * @tparam Func
     * @param func
     * @return float Execution time in microseconds
     */
    template <typename Func>
    static float MeasureExecutionTimeUs(Func&& func) {
        timing_t start_time, end_time;

        timing_init();
        timing_start();
        start_time = timing_counter_get();

        func();

        end_time = timing_counter_get();
        uint64_t total_cycles = timing_cycles_get(&start_time, &end_time);
        uint64_t total_ns = timing_cycles_to_ns(total_cycles);
        timing_stop();

        return static_cast<float>(total_ns) / 1000.0f;
    }
};

} // namespace eerie_leap::utilities::time
