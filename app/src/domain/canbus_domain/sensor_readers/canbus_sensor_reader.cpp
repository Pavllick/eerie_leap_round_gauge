#include <stdexcept>

#include "canbus_sensor_reader.h"

namespace eerie_leap::domain::canbus_domain::sensor_readers {

using namespace eerie_leap::subsys::canbus;

CanbusSensorReader::CanbusSensorReader(
    std::shared_ptr<ITimeService> time_service,
    std::shared_ptr<GuidGenerator> guid_generator,
    std::shared_ptr<Canbus> canbus,
    std::shared_ptr<Dbc> dbc)
        : time_service_(std::move(time_service)),
        guid_generator_(std::move(guid_generator)),
        canbus_(std::move(canbus)),
        dbc_(std::move(dbc)) {

    for(const auto& [frame_id, dbc_message] : dbc_->GetAllMessages())
        frames_signals_map_.emplace(frame_id, dbc_message.GetSignalNameHashes());

    for(const auto& [frame_id, _] : frames_signals_map_) {
        canbus_->RegisterFrameReceivedHandler(
            frame_id,
            [this, frame_id](const CanFrame& frame) {
                auto lock_key = k_spin_lock(&can_frame_lock_);
                Process(frame);
                k_spin_unlock(&can_frame_lock_, lock_key);
            });
    }
}

void CanbusSensorReader::Process(const CanFrame& frame) {
    if(!frames_signals_map_.contains(frame.id))
        return;

    for(const auto& signal_name_hash : frames_signals_map_.at(frame.id)) {
        double value = dbc_->GetMessage(frame.id)->GetSignalValue(
            signal_name_hash,
            frame.data.data());

        reading_processor_.Process({
            .id = guid_generator_->Generate(),
            .sensor_id_hash = static_cast<uint32_t>(signal_name_hash),
            .timestamp_ms = duration_cast<milliseconds>(time_service_->GetTimeSinceBoot().time_since_epoch()),
            .value = static_cast<float>(value),
            .status = ReadingStatus::PROCESSED
        });
    }
}

} // namespace eerie_leap::domain::canbus_domain::sensor_readers
