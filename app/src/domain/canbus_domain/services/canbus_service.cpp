#include <zephyr/logging/log.h>

#include "subsys/device_tree/dt_canbus.h"

#include "canbus_service.h"

namespace eerie_leap::domain::canbus_domain::services {

using namespace eerie_leap::subsys::device_tree;

LOG_MODULE_REGISTER(canbus_service_logger);

CanbusService::CanbusService(std::shared_ptr<CanChannelConfiguration> can_channel_configuration)
    : can_channel_configuration_(std::move(can_channel_configuration)) {

    auto canbus = std::make_shared<Canbus>(
        DtCanbus::Get(can_channel_configuration_->bus_channel),
        can_channel_configuration_->type,
        can_channel_configuration_->bitrate,
        can_channel_configuration_->data_bitrate,
        can_channel_configuration_->is_extended_id);
    if(!canbus->Initialize()) {
        LOG_ERR("Failed to initialize CAN channel %d.", can_channel_configuration_->bus_channel);
        return;
    }

    canbus_.emplace(can_channel_configuration_->bus_channel, canbus);
    ConfigureUserSignals(*can_channel_configuration_);
}

std::shared_ptr<Canbus> CanbusService::GetCanbus(uint8_t bus_channel) const {
    if(!canbus_.contains(bus_channel))
        return nullptr;

    return canbus_.at(bus_channel);
}

void CanbusService::ConfigureUserSignals(const CanChannelConfiguration& channel_configuration) {
    for(const auto& message_configuration : channel_configuration.message_configurations) {
        DbcMessage* message = nullptr;

        if(channel_configuration.dbc->HasMessage(message_configuration->frame_id))
            message = channel_configuration.dbc->GetMessage(message_configuration->frame_id);
        else
            message = channel_configuration.dbc->AddMessage(
                message_configuration->frame_id,
                message_configuration->name,
                message_configuration->message_size);

        for(const auto& signal_configuration : message_configuration->signal_configurations) {
            message->AddSignal(
                signal_configuration.name,
                signal_configuration.start_bit,
                signal_configuration.size_bits,
                signal_configuration.factor,
                signal_configuration.offset,
                signal_configuration.unit);
        }
    }
}

} // namespace eerie_leap::domain::canbus_domain::services
