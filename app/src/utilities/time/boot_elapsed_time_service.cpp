#include <ctime>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "boot_elapsed_time_service.h"

namespace eerie_leap::utilities::time {

void BootElapsedTimeService::Initialize() { }

system_clock::time_point BootElapsedTimeService::GetCurrentTime() {
	return system_clock::time_point(milliseconds(k_uptime_get()));
}

} // namespace eerie_leap::utilities::time
