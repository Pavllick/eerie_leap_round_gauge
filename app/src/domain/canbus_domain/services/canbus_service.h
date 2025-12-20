#pragma once

#include <memory>
#include <unordered_map>
#include <streambuf>

#include "domain/canbus_domain/models/can_channel_configuration.h"
#include "subsys/canbus/canbus.h"
#include "subsys/dbc/dbc.h"

namespace eerie_leap::domain::canbus_domain::services {

using namespace eerie_leap::subsys::canbus;
using namespace eerie_leap::subsys::dbc;
using namespace eerie_leap::domain::canbus_domain::models;

class CanbusService {
private:
    std::unordered_map<uint8_t, std::shared_ptr<Canbus>> canbus_;
    std::shared_ptr<CanChannelConfiguration> can_channel_configuration_;

    void ConfigureUserSignals(const CanChannelConfiguration& channel_configuration);

public:
    CanbusService(std::shared_ptr<CanChannelConfiguration> can_channel_configuration);

    std::shared_ptr<Canbus> GetCanbus(uint8_t bus_channel) const;
};

} // namespace eerie_leap::domain::canbus_domain::services
