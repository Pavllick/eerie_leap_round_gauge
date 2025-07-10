#pragma once

#include "i_time_service.h"

namespace eerie_leap::utilities::time {

using namespace std::chrono;

class BootElapsedTimeService : public ITimeService {
public:
    void Initialize() override;
    system_clock::time_point GetCurrentTime() override;
};

} // namespace eerie_leap::utilities::time