#include <stdexcept>

#include "canbus_sensors_reader.h"

namespace eerie_leap::domain::sensor_domain::readers {

using namespace eerie_leap::subsys::canbus;

CanbusSensorsReader::CanbusSensorsReader(
    std::shared_ptr<ITimeService> time_service,
    std::shared_ptr<Canbus> canbus,
    std::shared_ptr<Dbc> dbc,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame)
        : time_service_(std::move(time_service)),
        canbus_(std::move(canbus)),
        dbc_(std::move(dbc)),
        sensor_readings_frame_(std::move(sensor_readings_frame)) {

    for(const auto& [frame_id, dbc_message] : dbc_->GetAllMessages())
        frames_signals_map_.emplace(frame_id, dbc_message.GetSignalNameHashes());

    for(const auto& [frame_id, _] : frames_signals_map_) {
        int handler_id = canbus_->RegisterFrameReceivedHandler(
            frame_id,
            [this, frame_id](const CanFrame& frame) {
                UpdateReadings(frame);
            });

        if(handler_id < 0)
            throw std::runtime_error("Failed to register CAN frame handler for frame ID: " + std::to_string(frame_id));

        if(!registered_handler_ids_.contains(frame_id))
            registered_handler_ids_.insert({frame_id, {}});
        registered_handler_ids_[frame_id].push_back(handler_id);
    }
}

CanbusSensorsReader::~CanbusSensorsReader() {
    for(const auto& [frame_id, handler_ids] : registered_handler_ids_)
        for(const auto& handler_id : handler_ids)
            canbus_->RemoveFrameReceivedHandler(frame_id, handler_id);
}

void CanbusSensorsReader::UpdateReadings(const CanFrame& frame) {
    if(!frames_signals_map_.contains(frame.id))
        return;

    for(const auto& signal_name_hash : frames_signals_map_.at(frame.id)) {
        double value = dbc_->GetMessage(frame.id)->GetSignalValue(
            signal_name_hash,
            frame.data.data());

        sensor_readings_frame_->AddOrUpdateReading(std::make_shared<SensorReadingDto>(SensorReadingDto{
            .id_hash = signal_name_hash,
            .timestamp_ms = duration_cast<milliseconds>(time_service_->GetCurrentTime().time_since_epoch()),
            .value = static_cast<float>(value)
        }));
    }
}

} // namespace eerie_leap::domain::sensor_domain::readers
